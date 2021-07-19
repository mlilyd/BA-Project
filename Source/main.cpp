// OpenCL setup based on OpenCL ray tracing tutorial by Sam Lapere, 2016
// from http://raytracey.blogspot.com

//#define __CL_ENABLE_EXCEPTIONS
//#define CL_HPP_ENABLE_EXCEPTIONS

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <vector>
#include <cmath>
#include <random>

#include <CL/opencl.hpp>
#include <CL/cl.h>

#include <glm/glm.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../Header/stb_image_write.h"

#define float3(x, y, z) {{x, y, z}} 
#define float4(x, y, z, w) {{x, y, z, w}}



// CONSTANT DEFINITIONS
const int width = 50;
const int height = 50;
const int depth = 50;

const float epsilon = 0.00003f;
const float pi = 3.1415926535897932385;

cl_float3 cpu_output[width * height]{};
//cl_float3 cpu_output_4d[width * height * depth]{};
uint8_t output256[width * height * 3]{};

cl::CommandQueue queue;
cl::Kernel kernel;
cl::Context context;
cl::Program program;
cl::Buffer cl_output;
cl::Buffer cl_spheres;


struct TriangleMesh {
    std::vector<cl_float3> vertices;
    std::vector<float> ao_values;
    int faces;
    std::vector<int> vertIndex;
};

struct TetraMesh {
    std::vector<cl_float4> vertices;
    std::vector<float> ao_values;
    int vols;
    std::vector<int> vertIndex;
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

void saveToBinary(std::string filename, std::vector<glm::vec3> data) {
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    if (!file) {
        std::cout << "Cannot open file!\n";
        return;
    }
    file.write((char*)data.data(), data.size() * sizeof(glm::vec3));
    file.close();
}

void saveAOToBinary(std::string filename, std::vector<float> data) {
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    if (!file) {
        std::cout << "Cannot open file!\n";
        return;
    }
    file.write((char*)data.data(), data.size() * sizeof(float));
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

float dot(cl_float4 A, cl_float4 B) {
    return A.s0 * B.s0 + A.s1 * B.s1 + A.s2 * B.s2 + A.s3 * B.s3;
}


float det4(cl_float4 A, cl_float4 B, cl_float4 C, cl_float4 D) {
    float res = 0.0;
    res += A.s0 * ((B.s1 * C.s2 * D.s3) + (C.s1 * D.s2 * B.s3) + (D.s1 * B.s2 * C.s3) - (B.s3 * C.s2 * D.s1) - (C.s3 * D.s2 * B.s1) - (D.s3 * B.s2 * C.s1));
    res -= A.s1 * ((B.s0 * C.s2 * D.s3) + (C.s0 * D.s2 * B.s3) + (D.s0 * B.s2 * C.s3) - (B.s3 * C.s2 * D.s0) - (C.s3 * D.s2 * B.s0) - (D.s3 * B.s2 * C.s0));
    res += A.s2 * ((B.s0 * C.s1 * D.s3) + (C.s0 * D.s1 * B.s3) + (D.s0 * B.s1 * C.s3) - (B.s3 * C.s1 * D.s0) - (C.s3 * D.s1 * B.s0) - (D.s3 * B.s1 * C.s0));
    res -= A.s3 * ((B.s0 * C.s1 * D.s2) + (C.s0 * D.s1 * B.s2) + (D.s0 * B.s1 * C.s2) - (B.s2 * C.s1 * D.s0) - (C.s2 * D.s1 * B.s0) - (D.s2 * B.s1 * C.s0));
    return res;
}

cl_float4 cross4(cl_float4 A, cl_float4 B, cl_float4 C) {
 
    float x =   ((A.s1 * B.s2 * C.s3) + (A.s2 * B.s3 * C.s1) + (A.s3 * B.s1 * C.s2) - (C.s1 * B.s2 * A.s3) - (C.s2 * B.s3 * A.s1) - (C.s3 * B.s1 * A.s2));
    float y = - ((A.s0 * B.s2 * C.s3) + (A.s2 * B.s3 * C.s0) + (A.s3 * B.s0 * C.s2) - (C.s0 * B.s2 * A.s3) - (C.s2 * B.s3 * A.s0) - (C.s3 * B.s0 * A.s2));
    float z =   ((A.s0 * B.s1 * C.s3) + (A.s1 * B.s3 * C.s0) + (A.s3 * B.s0 * C.s1) - (C.s0 * B.s1 * A.s3) - (C.s1 * B.s3 * A.s0) - (C.s3 * B.s0 * A.s1));
    float w = - ((A.s0 * B.s1 * C.s2) + (A.s1 * B.s2 * C.s0) + (A.s2 * B.s0 * C.s1) - (C.s0 * B.s1 * A.s2) - (C.s1 * B.s2 * A.s0) - (C.s2 * B.s0 * A.s1));
    return float4(x, y, z, w);
}


bool intersect_tetrahedron(cl_float4 v0, cl_float4 v1, cl_float4 v2, cl_float4 v3, Ray4 ray, float &t) {

    cl_float4 v0v1 = v1 - v0;
    cl_float4 v0v2 = v2 - v0;
    cl_float4 v0v3 = v3 - v0;
    cl_float4 Tvec = ray.origin - v0;
    
    //std::cout << ray.dir.s0 << " " << ray.dir.s1 << " " << ray.dir.s2 << " "  << ray.dir.s3 << std::endl;
    //std::cout << v0v1.s0 << " " << v0v1.s1 << " " << v0v1.s2 << " " << v0v1.s3 << std::endl;
    //std::cout << v0v2.s0 << " " << v0v2.s1 << " " << v0v2.s2 << " " << v0v2.s3 << std::endl;
    //std::cout << v0v3.s0 << " " << v0v3.s1 << " " << v0v3.s2 << " " << v0v3.s3 << std::endl;

    float detM = det4(ray.dir, v0v1, v0v2, v0v3);
    //std::cout << "detM: " << detM << std::endl;

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
    //normalize coordinates
    float fx = (float)x / (float)width;
    float fy = (float)y / (float)height;
    float fz = (float)z / (float)depth;

    float aspect_ratio = (float)width / (float)height;

    float fx2 = (fx - 0.5f) * aspect_ratio;
    float fy2 = fy - 0.5f;
    float fz2 = fz - 0.5f;

    cl_float4 pixel_pos = float4(fx2, fy2, -fz2, 0.0f);

    struct Ray4 ray;
    ray.origin = float4(0.0f, 0.0f, 0.0f, 1.0f);
    ray.dir = normalize(pixel_pos - ray.origin);

    return ray;
}

bool intersect_mesh(Ray4 ray, std::vector<cl_float4>vertices, int vol, int* vertIndex, int &tetraIndex) {
    float t_old = 1e20;
    float t_new = 1e20;
    for (int i = 0; i < vol; i++) {
        cl_float4 v0 = vertices[vertIndex[0 + i*4]];
        cl_float4 v1 = vertices[vertIndex[1 + i * 4]];
        cl_float4 v2 = vertices[vertIndex[2 + i * 4]];
        cl_float4 v3 = vertices[vertIndex[3 + i * 4]];

        
        bool intersect = intersect_tetrahedron(v0, v1, v2, v3, ray, t_new);
        //std::cout << "t: " << t << std::endl;
        if (intersect) {
            //std::cout << "intersects a tetrahedra!\n";
            if (t_old > t_new){
                t_old = t_new;
                tetraIndex = i;
            }
            return true;
        }
    }
    return false;
}

bool intersect_mesh(Ray4 ray, TetraMesh mesh, int& tetraIndex) {
    float t_old = 1e20;
    float t_new = 1e20;
    for (int i = 0; i < mesh.vols; i++) {
        cl_float4 v0 = mesh.vertices[mesh.vertIndex[0 + i * 4]];
        cl_float4 v1 = mesh.vertices[mesh.vertIndex[1 + i * 4]];
        cl_float4 v2 = mesh.vertices[mesh.vertIndex[2 + i * 4]];
        cl_float4 v3 = mesh.vertices[mesh.vertIndex[3 + i * 4]];


        bool intersect = intersect_tetrahedron(v0, v1, v2, v3, ray, t_new);
        //std::cout << "t: " << t << std::endl;
        if (intersect) {
            //std::cout << "intersects a tetrahedra!\n";
            if (t_old > t_new) {
                t_old = t_new;
                tetraIndex = i;
            }
            return true;
        }
    }
    return false;
}

float random_angle(float min, float max) {
    float random = ((float)rand()) / (float)RAND_MAX;
    float diff = max - min;
    float res = random * diff;
    return min + res;
}

cl_float4 point_in_sphere_4d(cl_float4 center, float radius, float theta, float phi, float psi) {

    float x = center.s0 + radius * cos(psi);
    float y = center.s1 + radius * sin(psi) * cos(theta);
    float z = center.s2 + radius * sin(psi) * sin(theta) * cos(phi);
    float w = center.s3 + radius * sin(psi) * sin(theta) * sin(phi);

    return float4(x, y, z, w);
}

cl_float4 sample_hemisphere(cl_float4 center, float radius, cl_float4 normal) {
    
    while (true) {
        float theta = random_angle(0, pi);
        float phi = random_angle(0, 2 * pi);
        float psi = random_angle(0, pi);

        cl_float4 sample = point_in_sphere_4d(center, radius, theta, phi, psi);
        if (dot(sample, normal) > 0) {
            std::cout << "sample found!" << std::endl;
            return sample;
        }

    }
}


std::vector<float> get_ao4d(TetraMesh mesh, float radius, int samples) {
    std::vector<float> ao_values(mesh.vertices.size());

    for (int i = 0; i < mesh.vols; i++) {
        cl_float4 v0 = mesh.vertices[mesh.vertIndex[0 + i * 4]];
        cl_float4 v1 = mesh.vertices[mesh.vertIndex[1 + i * 4]];
        cl_float4 v2 = mesh.vertices[mesh.vertIndex[2 + i * 4]];
        cl_float4 v3 = mesh.vertices[mesh.vertIndex[3 + i * 4]];

        cl_float4 tetraVertex[4] = { v0, v1, v2, v3 };
        int index[4] = { mesh.vertIndex[0 + i * 4], mesh.vertIndex[1 + i * 4], mesh.vertIndex[2 + i * 4], mesh.vertIndex[3 + i * 4] };

        cl_float4 normal = cross4(v1 - v0, v2 - v0, v3 - v0);

        for (int j = 0; j < 4; j++) {
            cl_float4 vertex = tetraVertex[j];
            int vertexIndex = index[j];

            float ao = 0.0f;
            Ray4 ray;
            ray.origin = vertex;
            int k = 0;
            int counter = 0;

            for (int l = 0; l < samples; l++) {
                std::cout << "sample no. " << l << std::endl;
                ray.dir = sample_hemisphere(vertex, radius, normal);
               
                if (intersect_mesh(ray, mesh, k)) {
                    ao += 1.0;
                }
            }
            ao_values[vertexIndex] = ao / samples;
        }

        return ao_values;
    }
}

std::vector<glm::vec3> render4d_to_3d_glm(TetraMesh mesh) {
    std::vector<glm::vec3> data(width * height * depth);

    for (int i = 0; i < width * height * depth; i++) {
        int x = i % width;
        int z = i / (width * height);
        int y = (i - (z * width * height)) / width;
        //std::cout <<"Coordinates: " << x << " " << y << " " << z << std::endl;

        Ray4 camray = createCamRay4D(x, y, z);
        int tetraIndex = -1;
        if (intersect_mesh(camray, mesh, tetraIndex)) {

            if (tetraIndex != -1) {
                //std::cout << tetraIndex << std::endl;
            }
            data[i] = glm::vec3(1.0f, 1.0f, 1.0f);
            //std::cout << data[i].x << data[i].y << data[i].z << std::endl;
        }
        else {
            data[i] = glm::vec3(0.0f, 0.0f, 0.0f);
        }
    }

    return data;
}

std::vector<glm::vec3> render4d_ao_to_3d_glm(TetraMesh mesh, bool min1 = true) {
    std::vector<glm::vec3> data(width * height * depth);

    for (int i = 0; i < width * height * depth; i++) {
        int x = i % width;
        int z = i / (width * height);
        int y = (i - (z * width * height)) / width;
        //std::cout <<"Coordinates: " << x << " " << y << " " << z << std::endl;

        Ray4 camray = createCamRay4D(x, y, z);
        int tetraIndex = -1;
        if (intersect_mesh(camray, mesh, tetraIndex)) {

            if (tetraIndex != -1) {
                //std::cout << tetraIndex << std::endl;
            }
            ///*
            float ao0 = mesh.ao_values[mesh.vertIndex[0 + tetraIndex * 4]];
            float ao1 = mesh.ao_values[mesh.vertIndex[1 + int(tetraIndex * 4)]];
            float ao2 = mesh.ao_values[mesh.vertIndex[2 + int(tetraIndex * 4)]];
            float ao3 = mesh.ao_values[mesh.vertIndex[3 + int(tetraIndex * 4)]];

            float avg_ao;
            if (min1) {
                avg_ao = 1 - ((ao0 + ao1 + ao2 + ao3) / 4);
            }
            else {
                avg_ao = ((ao0 + ao1 + ao2 + ao3) / 4);
            }
              
            std::cout << "avg_ao: " << avg_ao << std::endl;
            data[i] = glm::vec3(avg_ao, avg_ao, avg_ao);
            //*/
            //data[i] = glm::vec3(1, 1, 1);
            //std::cout << data[i].x << data[i].y << data[i].z << std::endl;
        }
        else {
            data[i] = glm::vec3(0.0f, 0.0f, 0.0f);
        }
    }

    return data;
}



int main() {

    //init scene
    TetraMesh mesh;

    mesh.vertices = {
                        float4(-0.25, 0.0, 0.0, 0.0), //0
                        float4(-0.25, 0.25,-0.25, 0.0),//1 
                        float4(-0.3, 0.0, 0.5, 0.0), //2
                        float4(-0.5, 0.0, 0.0, 0.0), //3

                        float4(-0.5, 0.25, 0.0, 0.5), //4
                        float4(-0.35, 0.25, -0.25, 0.5), //5
                        float4(0.25, 0.0, 0.0, 0.0), //6
                        float4(0.5, 0.0, 0.0, 0.0) //7
    };

    mesh.vols = 2;
    mesh.vertIndex = { 0, 1, 2, 3, 4, 5, 6, 7 };

    mesh.ao_values = get_ao4d(mesh, 1.0, 25);



    std::string filename = "Renders/testa/z50";

    std::vector<glm::vec3> data = render4d_to_3d_glm(mesh);
    saveToBinary(filename+"_noao.raw", data);

    std::vector<glm::vec3> data_ao = render4d_ao_to_3d_glm(mesh);
    saveToBinary(filename + "_ao.raw", data_ao);
    
    //std::vector<glm::vec3> data_ao = render4d_ao_to_3d_glm(mesh);
    //saveToBinary(filename + "_min1_ao.raw", data_ao);


    return 0;
}