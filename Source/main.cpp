// OpenCL setup based on OpenCL ray tracing tutorial by Sam Lapere, 2016
// from http://raytracey.blogspot.com

//#define __CL_ENABLE_EXCEPTIONS
//#define CL_HPP_ENABLE_EXCEPTIONS

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <vector>
#include <cmath>
#include <CL/opencl.hpp>
#include <CL/cl.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../Header/stb_image_write.h"

#define float3(x, y, z) {{x, y, z}} 
#define float4(x, y, z, w) {{x, y, z, w}}

// CONSTANT DEFINITIONS
const int width = 300;
const int height = 300;
const int depth = 2;
const float epsilon = 0.00003f;

cl_float3 cpu_output[width * height]{};
//cl_float3 cpu_output_4d[width * height * depth]{};
uint8_t output256[width * height * 3]{};

cl::CommandQueue queue;
cl::Kernel kernel;
cl::Context context;
cl::Program program;
cl::Buffer cl_output;
cl::Buffer cl_spheres;

struct Triangle {
    cl_float3 v0;
    cl_float3 v1;
    cl_float3 v2;
    cl_float3 color;
};

struct Ray4 {
    cl_float4 origin;
    cl_float4 dir;
};

void initOpenCL() {
    // get all platforms (drivers), e.g. NVIDIA
    std::vector<cl::Platform> all_platforms;
    cl::Platform::get(&all_platforms);

    if (all_platforms.size() == 0) {
        std::cout << " No platforms found. Check OpenCL installation!\n";
        exit(1);
    }
    cl::Platform default_platform = all_platforms[0];
    std::cout << "Using platform: " << default_platform.getInfo<CL_PLATFORM_NAME>() << "\n";

    // get default device (CPUs, GPUs) of the default platform
    std::vector<cl::Device> all_devices;
    default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
    if (all_devices.size() == 0) {
        std::cout << " No devices found. Check OpenCL installation!\n";
        exit(1);
    }

    // on this computer, 
    // [0] = Intel HD Graphics 4400
    // [1] = Intel i5 CPU
    cl::Device device = all_devices[1];
    std::cout << "Using device: " << device.getInfo<CL_DEVICE_NAME>() << "\n";

    //create context and queue on device
    context = cl::Context(device);
    queue = cl::CommandQueue(context, device);

    //converet opencl kernel code to string
    std::string source;
    std::ifstream file("C:\\Users\\Lily\\Documents\\UNI\\BA\\programming\\Project\\Source\\kernel.cl");
    if (!file) {
        std::cout << "\nFile not found";
        exit(1);
    }
    while (!file.eof()) {
        char line[256];
        file.getline(line, 255);
        source += line;
    }

    //create opencl program using source
    program = cl::Program(context, source.c_str());

    // some error handling otherwise openCL will throw unhandled exception?
    cl_int result = program.build({ device });
    if (result) std::cout << "Error during compilation OpenCL code!!!\n (" << result << ")" << std::endl;
    if (result == CL_BUILD_PROGRAM_FAILURE) std::cout << "CL Build Program Failure?" << std::endl;

    kernel = cl::Kernel(program, "render_triangle");
}

float clamp(float x) { return x < 0.0f ? 0.0f : x > 1.0f ? 1.0f : x; }

//convert RGB float in range [0, 1] to int in range [0, 255]
int toInt(float x) { return int(clamp(x) * 255 + .5); }

//Using fprint for a more efficient write operation
void saveImage(std::string filename, bool ppm = true, bool png = true) {
    std::cout << "Saving image ...\n";
    FILE* file;

    if (ppm) {
        file = fopen((filename + ".ppm").c_str(), "w");
        fprintf(file, "P3\n%d %d\n%d\n", width, height, 255);
    }

    for (std::pair<int, int> i(0,0); i.first < width * height; i.first++) {

        int r = toInt(cpu_output[i.first].s[0]);
        int g = toInt(cpu_output[i.first].s[1]);
        int b = toInt(cpu_output[i.first].s[2]);

        if (ppm) {
            fprintf(file, "%d %d %d ", r, g, b);
        }

        if (png) {
            output256[i.second++] = r;
            output256[i.second++] = g;
            output256[i.second++] = b;
        }
    }
    if (ppm) {
        fclose(file);
        std::cout << "Finished writing PPM!\n";
    }
    if (png) {
        stbi_write_png((filename + ".png").c_str(), width, height, 3, output256, width*3);
        std::cout << "Finished writing png!\n";

    }

}

void saveToBinary(std::string filename, std::vector<cl_float3> data) {
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    if (!file) {
        std::cout << "Cannot open file!\n";
        return;
    }
    file.write((char*)&data[0], data.size()*sizeof(cl_float3));
    file.close();
}

void cleanUp() {
    delete cpu_output;
}

cl_float4 normalize(cl_float4 v) {
    float length = std::sqrt(v.s0*v.s0 + v.s1 * v.s1 + v.s2 * v.s2 + v.s3 * v.s3);
    return float4(v.s0 / length, v.s1 / length, v.s2 / length, v.s3 / length);
}

cl_float4 operator-(cl_float4 v, cl_float4 w) {
    return float4(v.s0 - w.s0, v.s1 - w.s1, v.s2 - w.s2, v.s3 - w.s3);
}

cl_float4 operator+(cl_float4 v, cl_float4 w) {
    return float4(v.s0 + w.s0, v.s1 + w.s1, v.s2 + w.s2, v.s3 + w.s3);
}

cl_float4 operator*(cl_float4 v, float t) {
    return float4(t * v.s0, t * v.s1, t * v.s2, t * v.s3);
}

float det4(cl_float4 A, cl_float4 B, cl_float4 C, cl_float4 D) {
    float res = 0.0;
    res += A.s0 * ((B.s1 * C.s2 * D.s3) + (C.s1 * D.s2 * B.s3) + (D.s1 * B.s2 * C.s3) - (B.s3 * C.s2 * D.s1) - (C.s3 * D.s2 * B.s1) - (D.s3 * B.s2 * C.s1));
    res -= A.s1 * ((B.s0 * C.s2 * D.s3) + (C.s0 * D.s2 * B.s3) + (D.s0 * B.s2 * C.s3) - (B.s3 * C.s2 * D.s0) - (C.s3 * D.s2 * B.s0) - (D.s3 * B.s2 * C.s0));
    res += A.s2 * ((B.s0 * C.s1 * D.s3) + (C.s0 * D.s1 * B.s3) + (D.s0 * B.s1 * C.s3) - (B.s3 * C.s1 * D.s0) - (C.s3 * D.s1 * B.s0) - (D.s3 * B.s1 * C.s0));
    res -= A.s3 * ((B.s0 * C.s1 * D.s2) + (C.s0 * D.s1 * B.s2) + (D.s0 * B.s1 * C.s2) - (B.s2 * C.s1 * D.s0) - (C.s2 * D.s1 * B.s0) - (D.s2 * B.s1 * C.s0));
    return res;
}

bool intersect_tetrahedra(cl_float4 v0, cl_float4 v1, cl_float4 v2, cl_float4 v3, Ray4 ray, float &t) {

    cl_float4 v0v1 = v1 - v0;
    cl_float4 v0v2 = v2 - v0;
    cl_float4 v0v3 = v3 - v0;
    cl_float4 Tvec = ray.origin - v0;
    
    std::cout << ray.dir.s0 << " " << ray.dir.s1 << " " << ray.dir.s2 << " "  << ray.dir.s3 << std::endl;
    //std::cout << v0v1.s0 << " " << v0v1.s1 << " " << v0v1.s2 << " " << v0v1.s3 << std::endl;
    //std::cout << v0v2.s0 << " " << v0v2.s1 << " " << v0v2.s2 << " " << v0v2.s3 << std::endl;
    //std::cout << v0v3.s0 << " " << v0v3.s1 << " " << v0v3.s2 << " " << v0v3.s3 << std::endl;

    float detM = det4(ray.dir, v0v1, v0v2, v0v3);
    std::cout << "detM: " << detM << std::endl;

    //if (detM < epsilon) { return false; }
    if (std::fabs(detM) < epsilon) { return false; }

    float invDet = 1 / detM;

    float Mt = det4(Tvec,    v0v1, v0v2, v0v3);
    float My = det4(ray.dir, Tvec, v0v2, v0v3);
    float Mz = det4(ray.dir, v0v1, Tvec, v0v3);
    float Mw = det4(ray.dir, v0v1, v0v2, Tvec);



    t = Mt * invDet;

    float y = My * invDet;
    if (y < 0) { return false; }
    
    float z = Mz * invDet;
    if (z < 0) { return false; }

    float w = Mw * invDet;
    if (w < 0 || y+z+w > 1) { return false; }

    

    return true;
}

Ray4 createCamRay4D(int x, int y, int z) {
    float fx = (float)x / (float)width;
    float fy = (float)y / (float)height;
    float fz = (float)z / (float)depth;

    float aspect_ratio = (float)width / (float)height;

    float fx2 = (fx - 0.5f) * aspect_ratio;
    float fy2 = fy - 0.5f;
    float fz2 = fz - 0.5f;

    cl_float4 pixel_pos = float4(fx2, -fy2, -fz2, 0.0f);

    struct Ray4 ray;
    ray.origin = float4(0.0f, 0.0f, 0.0f, 1.0f);
    ray.dir = normalize(pixel_pos - ray.origin);

    return ray;
}

bool intersect_mesh(Ray4 ray, std::vector<cl_float4>vertices, int vol, int* vertIndex) {
    float t = 1e20;
    for (int i = 0; i < vol; i++) {
        cl_float4 v0 = vertices[vertIndex[0 + i*4]];
        cl_float4 v1 = vertices[vertIndex[1 + i * 4]];
        cl_float4 v2 = vertices[vertIndex[2 + i * 4]];
        cl_float4 v3 = vertices[vertIndex[3 + i * 4]];

        
        bool intersect = intersect_tetrahedra(v0, v1, v2, v3, ray, t);
        std::cout << "t: " << t << std::endl;
        if (intersect) {
            std::cout << "intersects a tetrahedra!\n";
            return true;
        }
    }
    return false;
}


std::vector<cl_float3> render4d_to_3d() {
    std::vector<cl_float3> data (width * height * depth);

    for (int i = 0; i < width * height * depth; i++) {
        int x = i % width;
        int z = i / (width * height);
        int y = (i - z * width * height) / width;

        cl_float4 v[] = { 
                       float4(0.25, 0.0, 0.0, 0.0),
                       float4(0.25, 0.25, 0.25, 0.0),
                       float4(0.3, 0.0, 0.5, 0.0),
                       float4(0.5, 0.0, 0.0, 0.0)
                        };

        std::vector<cl_float4> vertices(v, v + sizeof(v) / sizeof(cl_float4));

        int vol = 1;
        int vertIndex[] = { 0, 1, 2, 3};
        Ray4 camray = createCamRay4D(x, y, z);
        

        if (intersect_mesh(camray, vertices, vol, vertIndex)) {
            data[i] = float3(1.0f, 1.0f, 1.0f);
        }
        else {
            data[i] = float3(0.0f, 0.0f, 0.0f);
        }
    }

    return data;
}

int main() {
     //setup opencl
    initOpenCL();

    // create buffer on device for image output and scene
    cl_output = cl::Buffer(context, CL_MEM_WRITE_ONLY, width * height * sizeof(cl_float3));
    //cl_output = cl::Buffer(context, CL_MEM_WRITE_ONLY, width * height * depth * sizeof(cl_float3) );
    
    // create buffer on device for storing vertices
    //cl::Buffer cl_test_array = cl::Buffer(context, CL_MEM_READ_ONLY, 3 * sizeof(cl_float3));
    //cl_float3 test[3]{};
    //test[0] = float3(1.0f, 0.0f, 0.0f);
    //test[1] = float3(0.0f, 1.0f, 0.0f);
    //test[2] = float3(1.0f, 1.0f, 0.0f);
    //queue.enqueueWriteBuffer(cl_test_array, CL_TRUE, 0, 3 * sizeof(cl_float3), test);

    //init triangle
    Triangle t;
    t.v0 = float3(0.25f, 0.0f, 0.0f);
    t.v1 = float3(0.0f, 0.25f, 0.0f);
    t.v2 = float3(0.0f, 0.0f, 0.0f);
    t.color = float3(1.0f, 1.0f, 1.0f);

    //std::cout << "Rendering image...\n";

    //specify kernel arguments
    kernel.setArg(0, cl_output);
    kernel.setArg(1, width);
    kernel.setArg(2, height);
    //kernel.setArg(3, depth);
    kernel.setArg(3, t);
    
    //specify work items. 
    //each pixel has its own work item, so number of work items equals number of pixels
    std::size_t global_work_size = width * height;
    std::size_t local_work_size = 64;

    //launch kernel
    queue.enqueueNDRangeKernel(kernel, NULL, global_work_size, local_work_size);
    queue.finish();
       
    //read opencl output to CPU 
    queue.enqueueReadBuffer(cl_output, CL_TRUE, 0, width * height * sizeof(cl_float3), cpu_output);

    //std::string filename = "Renders/triangle_test3";
    //saveImage(filename, false);

    std::vector<cl_float3> data = render4d_to_3d();
    //saveToBinary("Renders/img9.bin", data);
    

    // check if det4 is correct, result should be -104
    /*
    cl_float4 A = float4(1.0f, 3.0f, 0.0f, 1.0f);
    cl_float4 B = float4(3.0f, 1.0f, 1.0f, 6.0f);
    cl_float4 C = float4(2.0f, 0.0f, 4.0f, 1.0f);
    cl_float4 D = float4(1.0f, 3.0f, 0.0f, 5.0f);
    std::cout << det4(A, B, C, D);
    */

    return 0;
}