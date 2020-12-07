#include "../Header/Renderer.h"
#include "../Header/Camera.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <iostream>
#include <CL/cl.h>
#include <CL/opencl.h>

#define ARRAY_SIZE 64

//OpenCL accelerations


/* Find a GPU or CPU associated with the first available platform

The `platform` structure identifies the first platform identified by the
OpenCL runtime. A platform identifies a vendor's installation, so a system
may have an NVIDIA platform and an AMD platform.

The `device` structure corresponds to the first accessible device
associated with the platform. Because the second parameter is
`CL_DEVICE_TYPE_GPU`, this device must be a GPU.
*/
cl_device_id create_device() {

    cl_platform_id platform;
    cl_device_id dev;
    int err;

    /* Identify a platform */
    err = clGetPlatformIDs(1, &platform, NULL);
    if (err < 0) {
        std::cout<<"Couldn't identify a platform";
        exit(1);
    }

    // Access a device
    // GPU
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (err == CL_DEVICE_NOT_FOUND) {
        // CPU
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
    }
    if (err < 0) {
        std::cout << "Couldn't access any devices";
        exit(1);
    }

    return dev;
}


/* Create program from a std::string and compile it */
cl_program build_program(cl_context ctx, cl_device_id device, std::string kernel_code) {

    cl_program program;
    char* program_log;
    size_t log_size;
    int err;

    const size_t program_size = kernel_code.length();
    const char* kernel_code_c = kernel_code.c_str();
    

    /* Create program from file

    Creates a program from the source code in the add_numbers.cl file.
    Specifically, the code reads the file's content into a char array
    called program_buffer, and then calls clCreateProgramWithSource.
    */
    program = clCreateProgramWithSource(ctx, 1, &kernel_code_c, &program_size, &err);
    if (err < 0) {
        perror("Couldn't create the program");
        exit(1);
    }

    /* Build program

    The fourth parameter accepts options that configure the compilation.
    These are similar to the flags used by gcc. For example, you can
    define a macro with the option -DMACRO=VALUE and turn off optimization
    with -cl-opt-disable.
    */
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (err < 0) {

        /* Find size of log and print to std output */
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
            0, NULL, &log_size);
        program_log = (char*)malloc(log_size + 1);
        program_log[log_size] = '\0';
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
            log_size + 1, program_log, NULL);
        printf("%s\n", program_log);
        free(program_log);
        exit(1);
    }

    return program;
}


vector<Color> simple_gradient(const int width, const int height){

    vector<Color> img_pixels;
    for (int j=height-1;j>=0; j--){
        for (int i=0;i<width;i++){
            Color pixel_Color(double(i)/(width-1), double(j)/(height-1), 0.20);
            img_pixels.push_back(pixel_Color);
        }
    }
    return img_pixels;

}

Color ray_color(const Ray& r, const Hittable& scene, const Color& background, int depth) {
    hit_record rec;

    if (depth <= 0){
        return Color(0.0, 0.0, 0.0);
    }
    /*
    if (!scene.hit(r, 0.001, infinity, rec)){
        return background;
    }
    */
    if (scene.hit(r, 0.001, infinity, rec)){
        Ray scattered;
        Color attenuation;
        try {
            if (rec.material_ptr != nullptr){
                if (rec.material_ptr->scatter(r, rec, attenuation, scattered)) {
                    return attenuation * ray_color(scattered, scene, background, depth - 1);
                }
            }
        }
        catch (...){
 
        }
         
    }
    
    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5*(unit_direction.y() + 1.0);
    return (1.0-t)*Color(1.0, 1.0, 1.0) + t*Color(0.5, 0.7, 1.0);
}
/*
Color ray4_color(const Ray4& r, const Hittable4& scene, const Color& background, int depth) {
    hit_record rec;

    if (depth <= 0) {
        return Color(0.0, 0.0, 0.0);
    }
    /*
    if (!scene.hit(r, 0.001, infinity, rec)){
        return background;
    }
    
    if (scene.hit(r, 0.001, infinity, rec)) {
        Ray scattered;
        Color attenuation;
        if (rec.material_ptr->scatter(r, rec, attenuation, scattered)) {
            return attenuation * ray_color(scattered, scene, background, depth - 1);
        }
    }
   
    vec4 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * Color(1.0, 1.0, 1.0) + t * Color(0.5, 0.7, 1.0);
}
*/
double ambient_occlusion(const Ray &r, const hit_record &rec, const Hittable &scene, int ray_shot){

    Point3 p = rec.p;
    int ray_hit = 0;
    vec3 normal = rec.normal;

    for(int i=0;i<ray_shot;i++){
        vec3 dest = random_in_hemisphere(normal);
        Ray r = Ray(p, dest);
        hit_record temp_rec;
        if (scene.hit(r, 0.0001, infinity, temp_rec)){
            ray_hit += 1;
        }
    }

    return 1.0 - (ray_hit/ray_shot);
}

Color ray_AO(const Ray& r, const Hittable& scene, int depth, int ray_shot) {
    hit_record rec;

    if (depth <= 0){
        return Color(0.0, 0.0, 0.0);
    }

    if (scene.hit(r, 0.001, infinity, rec)){
        return Color(1.0, 1.0, 1.0) * ambient_occlusion(r, rec, scene, ray_shot);
    }
    else{
        return Color(1.0, 1.0, 1.0);
    }
}

vector<Color> render(Camera camera, Hittable_List scene, const Color& background, const int img_width, const int img_height, int samples_per_pixel, int max_depth){
    vector<Color> img_pixels;
    for (int j = img_height-1; j>=0; --j){
        for (int i = 0; i<img_width; ++i){
            Color pixel(0,0,0);
            double AO_coeff = 0.0;
            for (int s=0;s<samples_per_pixel; ++s){
                auto u = (i + random_double())/(img_width-1);
                auto v = (j + random_double())/(img_height-1);
                Ray r = camera.get_ray(u,v);
                pixel += ray_color(r, scene, background, max_depth); 
            }
            img_pixels.push_back(pixel);
        }
    }
    return img_pixels;
}

vector<Color> render_AO(Camera camera, Hittable_List scene, const Color& background, const int img_width, const int img_height, int samples_per_pixel, int max_depth, int ray_shot){
    vector<Color> img_pixels;
    for (int j = img_height-1; j>=0; --j){
        for (int i = 0; i<img_width; ++i){
            Color pixel(0,0,0);
            double AO_coeff = 0.0;
            for (int s=0;s<samples_per_pixel; ++s){
                auto u = (i + random_double())/(img_width-1);
                auto v = (j + random_double())/(img_height-1);
                Ray r = camera.get_ray(u,v);
                pixel += ray_AO(r, scene, max_depth, ray_shot); 
            }
            img_pixels.push_back(pixel);
        }
    }
    return img_pixels;
}

/*
vector<Color> render4d(Camera4 camera, Hittable_List scene, const Color& bg, const int img_width, const int img_height, int samples_per_pixel, int max_depth, int ray_shot) {
    vector<Color> img_pixels;
    for (int j = img_height - 1; j >= 0; --j) {
        for (int i = 0; i < img_width; ++i) {
            Color pixel(0, 0, 0);
            double AO_coeff = 0.0;
            for (int s = 0; s < samples_per_pixel; ++s) {
                auto u = (i + random_double()) / ((img_width) - 1);
                auto v = (j + random_double()) / ((img_height) - 1);
                Ray4 r = camera.get_ray(u, v);
                //pixel += ray_color(r, scene, background, max_depth);
            }
            img_pixels.push_back(pixel);
        }
    }
    return img_pixels;
}
*/

Hittable_List spheres(){
    
    //set up scene
    Hittable_List scene;

    auto material_ground = make_shared<Lambertian>(Color(0.8, 0.8, 0.0));
    auto material_center = make_shared<Lambertian>(Color(0.7, 0.3, 0.3));
  
    scene.add(make_shared<Sphere>(Point3( 0.0, -100.5, -1.0), 100.0, material_ground));
    scene.add(make_shared<Sphere>(Point3( 0.0,    0.0, -1.0),   0.5, material_center));
    
    return scene;

}

Hittable_List boxes(){

    Hittable_List scene;
    auto red   = make_shared<Lambertian>(Color(.65, .05, .05));
    auto white = make_shared<Lambertian>(Color(.73, .73, .73));
    auto green = make_shared<Lambertian>(Color(.12, .45, .15));
    auto blue = make_shared<Lambertian>(Color(.15, .12, .45));


    /*scene.add(make_shared<XZ_Rectangle>(-1000, 1000, -1000, 1000, 0, white));

    scene.add(make_shared<Box>(Point3(130, 0, 65),  Point3(295, 165, 230), green));
    scene.add(make_shared<Box>(Point3(-65, 0, 65),  Point3(100, 165, 230), green));
    scene.add(make_shared<Box>(Point3(-250, 0, 65),  Point3(-85, 165, 230), green));

    scene.add(make_shared<Box>(Point3(265, 0, 295), Point3(430, 330, 460), red));
    scene.add(make_shared<Box>(Point3(500, 0, 34), Point3(600, 200, 200), blue));

    scene.add(make_shared<Sphere>(Point3( 400, 50, 10),   100, green));
    */
    scene.add(make_shared<Sphere>(Point3(0,0,0), 10, red));
    scene.add(make_shared<Triangle>(Point3(-10, 0, 0), Point3(-5, 0, 0), Point3(-3, 5, 0), blue));

    return scene;

}

int main(){

    /* OpenCL structures */
    cl_device_id device;
    cl_context context;
    cl_program program;
    cl_kernel kernel;
    cl_command_queue queue;
    cl_int i, j, err;
    size_t local_size, global_size;

    /* Data and buffers    */
    float data[ARRAY_SIZE];
    float sum[2], total, actual_sum;
    cl_mem input_buffer, sum_buffer;
    cl_int num_groups;

    /* Initialize data */
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = 1.0f * i;
    }
   
    /* Create device and context
    

   Creates a context containing only one device — the device structure
   created earlier.
   */
    device = create_device();
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    if (err < 0) {
        perror("Couldn't create a context");
        exit(1);
    }

    //define kernel function as string
    std::string kernel_code = 
        "   void kernel simple_add(global const int* A, global const int* B, global int* C, "
        "                          global const int* N) {"
        "       int ID, Nthreads, n, ratio, start, stop;"
        ""
        "       ID = get_global_id(0);"
        "       Nthreads = get_global_size(0);"
        "       n = N[0];"
        ""
        "       ratio = (n / Nthreads);"  // number of elements for each thread
        "       start = ratio * ID;"
        "       stop  = ratio * (ID + 1);"
        ""
        "       for (int i=start; i<stop; i++)"
        "           C[i] = A[i] + B[i];"
        "   }";

    /* Build program */
    program = build_program(context, device, kernel_code);
    
    /* Create data buffer

    • `global_size`: total number of work items that will be
       executed on the GPU (e.g. total size of your array)
    • `local_size`: size of local workgroup. Each workgroup contains
       several work items and goes to a compute unit

    In this example, the kernel is executed by eight work-items divided into
    two work-groups of four work-items each. Returning to my analogy,
    this corresponds to a school containing eight students divided into
    two classrooms of four students each.

      Notes:
    • Intel recommends workgroup size of 64-128. Often 128 is minimum to
    get good performance on GPU
    • On NVIDIA Fermi, workgroup size must be at least 192 for full
    utilization of cores
    • Optimal workgroup size differs across applications
    */
    global_size = 8; // WHY ONLY 8?
    local_size = 4;
    num_groups = global_size / local_size;
    input_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY |
        CL_MEM_COPY_HOST_PTR, ARRAY_SIZE * sizeof(float), data, &err); // <=====INPUT
    sum_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE |
        CL_MEM_COPY_HOST_PTR, num_groups * sizeof(float), sum, &err); // <=====OUTPUT
    if (err < 0) {
        perror("Couldn't create a buffer");
        exit(1);
    };

    /* Create a command queue

    Does not support profiling or out-of-order-execution
    */
    queue = clCreateCommandQueue(context, device, 0, &err);
    if (err < 0) {
        perror("Couldn't create a command queue");
        exit(1);
    };

    /* Create a kernel */
    kernel = clCreateKernel(program, "simple_add", &err);
    if (err < 0) {
        perror("Couldn't create a kernel");
        exit(1);
    };

    /* Create kernel arguments */
    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_buffer); // <=====INPUT
    err = clSetKernelArg(kernel, 1, local_size * sizeof(float), NULL);
    err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &sum_buffer); // <=====OUTPUT
    if (err < 0) {
        perror("Couldn't create a kernel argument");
        exit(1);
    }

    /* Enqueue kernel

    At this point, the application has created all the data structures
    (device, kernel, program, command queue, and context) needed by an
    OpenCL host application. Now, it deploys the kernel to a device.

    Of the OpenCL functions that run on the host, clEnqueueNDRangeKernel
    is probably the most important to understand. Not only does it deploy
    kernels to devices, it also identifies how many work-items should
    be generated to execute the kernel (global_size) and the number of
    work-items in each work-group (local_size).
    */
    err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size,
        &local_size, 0, NULL, NULL);
    if (err < 0) {
        perror("Couldn't enqueue the kernel");
        exit(1);
    }

    /* Read the kernel's output    */
    err = clEnqueueReadBuffer(queue, sum_buffer, CL_TRUE, 0,
        sizeof(sum), sum, 0, NULL, NULL); // <=====GET OUTPUT
    if (err < 0) {
        perror("Couldn't read the buffer");
        exit(1);
    }

    /* Check result */
    total = 0.0f;
    for (j = 0; j < num_groups; j++) {
        total += sum[j];
    }
    actual_sum = 1.0f * ARRAY_SIZE / 2 * (ARRAY_SIZE - 1);
    printf("Computed sum = %.1f.\n", total);
    if (fabs(total - actual_sum) > 0.01 * fabs(actual_sum))
        printf("Check failed.\n");
    else
        printf("Check passed.\n");

    /* Deallocate resources */
    clReleaseKernel(kernel);
    clReleaseMemObject(sum_buffer);
    clReleaseMemObject(input_buffer);
    clReleaseCommandQueue(queue);
    clReleaseProgram(program);
    clReleaseContext(context);
    return 0;


    /*
    const int width = 500;
    const auto aspect_ratio = 16.0 / 9.0;
    const int height = width / aspect_ratio;
    const int samples = 200;

    const int max_depth = 5;
    const int ray_shot = 2;
    //Camera camera(Point3(-100, 500,-1000), Point3(300, 300, 0), vec3(0,1,0), 45, aspect_ratio);
    //amera camera(Point3(0, 10,-100), Point3(0, 0, 0), vec3(0,1,0), 45, aspect_ratio);

    const Color background = Color(0.0, 0.0, 0.0);

    //Hittable_List scene = boxes();
    //vector<Color> ao;
    //vector<Color> img = render(camera, scene, background, width, height, samples, max_depth);
    //vector<Color> img_ao = render_AO(camera, scene, background, width, height, samples, max_depth, ray_shot);

    //write_img_jpg(img, "renders/test_11.jpg", width, height, samples);*/
}
