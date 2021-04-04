// Based on OpenCL ray tracing tutorial by Sam Lapere, 2016
// from http://raytracey.blogspot.com

//#define __CL_ENABLE_EXCEPTIONS
//#define CL_HPP_ENABLE_EXCEPTIONS

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <vector>
#include <CL/opencl.hpp>
#include <CL/cl.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../Header/stb_image_write.h"

// CONSTANT DEFINITIONS
const int img_width = 1280;
const int img_height = 720;

cl_float3 cpu_output[img_width * img_height]{};
uint8_t output256[img_width * img_height * 3]{};

cl::CommandQueue queue;
cl::Kernel kernel;
cl::Context context;
cl::Program program;
cl::Buffer cl_output;
cl::Buffer cl_spheres;

//dummy variables required for memory allignment
struct Sphere {
    cl_float radius;
    cl_float dummy1;
    cl_float dummy2;
    cl_float dummy3;
    cl_float3 position;
    cl_float3 color;
    cl_float3 emission;
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
    std::ifstream file("C:\\Users\\Lily\\Documents\\UNI\\BA\\project\\Project\\Source\\kernel.cl");
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
        fprintf(file, "P3\n%d %d\n%d\n", img_width, img_height, 255);
    }

    for (std::pair<int, int> i(0,0); i.first < img_width * img_height; i.first++) {

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
        stbi_write_png((filename + ".png").c_str(), img_width, img_height, 3, output256, img_width*3);
        std::cout << "Finished writing png!\n";

    }

}

void cleanUp() {
    delete cpu_output;
}

#define float3(x, y, z) {{x, y, z}} 

void initScene(Sphere* cpu_spheres) {
    
    // left wall
    cpu_spheres[0].radius = 200.0f;
    cpu_spheres[0].position = float3(-200.6f, 0.0f, 0.0f);
    cpu_spheres[0].color = float3(0.75f, 0.25f, 0.25f);
    cpu_spheres[0].emission = float3(0.0f, 0.0f, 0.0f);

    // right wall
    cpu_spheres[1].radius = 200.0f;
    cpu_spheres[1].position = float3(200.6f, 0.0f, 0.0f);
    cpu_spheres[1].color = float3(0.25f, 0.25f, 0.75f);
    cpu_spheres[1].emission = float3(0.0f, 0.0f, 0.0f);
    

    // floor
    cpu_spheres[2].radius = 200.0f;
    cpu_spheres[2].position = float3(0.0f, -200.4f, 0.0f);
    cpu_spheres[2].color = float3(0.9f, 0.8f, 0.7f);
    cpu_spheres[2].emission = float3(0.0f, 0.0f, 0.0f);

    
    // ceiling
    cpu_spheres[3].radius = 200.0f;
    cpu_spheres[3].position = float3(0.0f, 200.4f, 0.0f);
    cpu_spheres[3].color = float3(0.9f, 0.8f, 0.7f);
    cpu_spheres[3].emission = float3(0.0f, 0.0f, 0.0f);

    // back wall
    cpu_spheres[4].radius = 200.0f;
    cpu_spheres[4].position = float3(0.0f, 0.0f, -200.4f);
    cpu_spheres[4].color = float3(0.9f, 0.8f, 0.7f);
    cpu_spheres[4].emission = float3(0.0f, 0.0f, 0.0f);

 
    // front wall 
    cpu_spheres[5].radius = 200.0f;
    cpu_spheres[5].position = float3(0.0f, 0.0f, 202.0f);
    cpu_spheres[5].color = float3(0.9f, 0.8f, 0.7f);
    cpu_spheres[5].emission = float3(0.0f, 0.0f, 0.0f);
    

    // left sphere
    cpu_spheres[0].radius = 0.4f;
    cpu_spheres[0].position = float3(0.0f, 0.0f, 0.0f);
    cpu_spheres[0].color = float3(0.9f, 0.8f, 0.7f);
    cpu_spheres[0].emission = float3(0.2f, 0.2f, 0.2f);

    // right sphere
    cpu_spheres[1].radius = 0.16f;
    cpu_spheres[1].position = float3(0.25f, -0.24f, 0.1f);
    cpu_spheres[1].color = float3(0.9f, 0.8f, 0.7f);
    cpu_spheres[1].emission = float3(0.0f, 0.0f, 0.0f);
   
    // lightsource
    cpu_spheres[2].radius = 1.0f;
    cpu_spheres[2].position = float3(0.0f, 1.36f, 0.0f);
    cpu_spheres[2].color = float3(0.0f, 0.0f, 0.0f);
    cpu_spheres[2].emission = float3(9.0f, 8.0f, 6.0f);
    

}

int main() {
     //setup opencl
    initOpenCL();

    // initialize scene
    Sphere cpu_spheres[9];
    initScene(cpu_spheres);

    // create buffer on device for image output and scene
    cl_output = cl::Buffer(context, CL_MEM_WRITE_ONLY, img_width * img_height * sizeof(cl_float3));
    cl_spheres = cl::Buffer(context, CL_MEM_READ_ONLY, 9 * sizeof(Sphere));
    
    cl::Buffer cl_test_array = cl::Buffer(context, CL_MEM_READ_ONLY, 3 * sizeof(cl_float3));

    std::cout << "Rendering image...\n";
    //specify kernel arguments
    kernel.setArg(0, cl_output);
    kernel.setArg(1, img_width);
    kernel.setArg(2, img_height);
    //kernel.setArg(3, cl_test_array);
    //kernel.setArg(4, cl_spheres);
    
    //specify work items. 
    //each pixel has its own work item, so number of work items equals number of pixels
    std::size_t global_work_size = img_width * img_height;
    std::size_t local_work_size = 64;

    //launch kernel
    queue.enqueueNDRangeKernel(kernel, NULL, global_work_size, local_work_size);
    queue.finish();
    
    std::cout << "Rendering done!\n";
        
    //read opencl output to CPU 
    queue.enqueueReadBuffer(cl_output, CL_TRUE, 0, img_width * img_height * sizeof(cl_float3), cpu_output);

    //save image to PPM format
    std::string filename = "Renders/new";
    
    saveImage(filename, false);
   
    return 0;
}