// Based on OpenCL ray tracing tutorial by Sam Lapere, 2016
// from http://raytracey.blogspot.com

//#define __CL_ENABLE_EXCEPTIONS
//#define CL_HPP_ENABLE_EXCEPTIONS

#include <iostream>
#include <fstream>
#include <vector>
#include <CL/opencl.hpp>
#include <CL/cl.h>

// CONSTANT DEFINITIONS
const int img_width = 1280;
const int img_height = 720;

cl_float4* cpu_output; //cl_float4 is not part of cl namespace
cl::CommandQueue queue;
cl::Kernel kernel;
cl::Context context;
cl::Program program;
cl::Buffer cl_output;


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

    kernel = cl::Kernel(program, "add");


}


float clamp(float x) { return x < 0.0f ? 0.0f : x > 1.0f ? 1.0f : x; }
//convert RGB float in range [0, 1] to int in range [0, 255]
int toInt(float x) { return int(clamp(x) * 255 + .5); }

void writeImage() {
    // write to PPM file, based on openCL example
    FILE* f = fopen("../Renders/test.ppm", "w");
    fprintf(f, "P3\n%d %d\n%d\n", img_width, img_height, 255);

    //loop over pixels, and write RGB values
    for (int i = 0; i < img_width * img_height; i++) {
        fprintf(f, "%d %d %d ",
            toInt(cpu_output[i].s[0]),
            toInt(cpu_output[i].s[1]),
            toInt(cpu_output[i].s[2])
        );
    }

}

void cleanUp() {
    delete cpu_output;
}

int main() {
    initOpenCL();

    // create buffers on the device
    cl::Buffer buffer_A(context, CL_MEM_READ_WRITE, sizeof(int) * 10);
    cl::Buffer buffer_B(context, CL_MEM_READ_WRITE, sizeof(int) * 10);
    cl::Buffer buffer_C(context, CL_MEM_READ_WRITE, sizeof(int) * 10);

    // init arrays
    int A[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    int B[] = { 0, 1, 2, 0, 1, 2, 0, 1, 2, 0 };
    //RES   = { 0, 2, 4, 3, 5, 7, 6, 8, 10, 9 };

    // write arrays to buffer
    queue.enqueueWriteBuffer(buffer_A, CL_TRUE, 0, sizeof(int) * 10, A);
    queue.enqueueWriteBuffer(buffer_B, CL_TRUE, 0, sizeof(int) * 10, B);

    // Run Kernel
    kernel.setArg(0, buffer_A);
    kernel.setArg(1, buffer_B);
    kernel.setArg(2, buffer_C);

    queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(10), cl::NullRange);
    queue.finish();

    // init resulting array c
    int C[10]{};
    //read result C from the device to array C
    queue.enqueueReadBuffer(buffer_C, CL_TRUE, 0, sizeof(int) * 10, C);

    return 0;
}