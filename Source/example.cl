struct Sphere{
    float radius;
    float3 position;
    float3 color;
    float3 emit;
};

bool intersect_sphere(const struct Sphere* sphere, const struct Ray* ray, float* t){
    float3 rayToCenter = sphere->position - ray->origin;
    
    float b = dot(rayToCenter, ray->dir);
    float c = dot(rayToCenter, rayToCenter) - sphere->radius*sphere->radius;
    float disc = b*b-c;

    if (disc < 0.0f) {return false;}
    else {*t = b - sqrt(disc);}

    if (*t < 0.0f){
        *t = b + sqrt(disc);
        if (*t < 0.0f) {return false;}
    }
    else {return true;}
}

__kernel void render_gradient(__global float3* cl_output, int img_width, int img_height){
    const int id = get_global_id(0);
    int x = id % img_width;
    int y = id / img_width; 
    float fx = (float)x / (float)(img_width-1);
    float fy = (float)y / (float)(img_height-1);
    cl_output[id] = (float3) (fy, fx, 0.5);
}

__kernel void render_sphere(__global float3* cl_output, int img_width, int img_height){
    const int id = get_global_id(0);
    int x = id % img_width;
    int y = id / img_width;

    float fx = (float)x / (float)(img_width-1);
    float fy = (float)y / (float)(img_height-1);

    struct Ray camray = createCamRay(x, y, img_width, img_height);

    struct Sphere sphere;
    sphere.radius = 0.4f;
    sphere.position = (float3)(0.0f, 0.0f, 3.0f);
    sphere.color = (float3)(0.9f, 0.3f, 0.0f);

    float t = 1e20;
    intersect_sphere(&sphere, &camray, &t);

    if (t > 1e19){
        cl_output[id] = (float3)((fy-0.8)*0.5f, (fy+0.6)*0.9f, 0.95f);
        return;
    }
	
	cl_output[id] = sphere.color;
}