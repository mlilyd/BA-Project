
__kernel void add(__global const int* v1, __global const int* v2, __global int* v3) {
    int ID;
    ID = get_global_id(0);
    v3[ID] = v1[ID] + v2[ID];
}

__kernel void render_gradient(__global float3* cl_output, int img_width, int img_height){
    const int id = get_global_id(0);
    int x = id % img_width;
    int y = id / img_width; 
    float fx = (float)x / (float)img_width;
    float fy = (float)y / (float)img_height;
    cl_output[id] = (float3) (fx, fy, 0.0);
}