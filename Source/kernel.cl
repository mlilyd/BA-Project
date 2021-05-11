__constant float epsilon = 0.00003f;
__constant float pi = 3.14159265359f;
__constant int samples = 64;


struct Ray{
    float3 origin;
    float3 dir;
};

struct Ray4{
    float4 origin;
    float4 dir;
};

struct Triangle{
    float3 v0;
    float3 v1;
    float3 v2;
    float3 color;
};

struct TriangleMesh{
    float3* vertices;
    float3 face;
    float3 faceIndex;
    float3 vertIndex;
    float3 normal;
};

struct Tetrahedron{
    float4 v0;
    float4 v1;
    float4 v2;
    float4 v3;
};

struct TetraMesh{
    float4* vertices;
    float4* ao_values;
    int vol;
    int* volIndex;
    int* vertIndex;
    float4* normal;

};


static float get_random(unsigned int *seed0, unsigned int *seed1){
    *seed0 = 36969 * ((*seed0) & 65535) + ((*seed0) >> 16);  
	*seed1 = 18000 * ((*seed1) & 65535) + ((*seed1) >> 16);

    unsigned int ires = ((*seed0) << 16) + (*seed1);

    union {
		float f;
		unsigned int ui;
	} res;

    res.ui = (ires & 0x007fffff) | 0x40000000;
    return (res.f - 2.0f) / 2.0f;
}

float det4(float4 A, float4 B, float4 C, float4 D){
    float res = 0.0;
    res += A.s0 * ((B.s1*C.s2*D.s3) + (C.s1*D.s2*B.s3) + (D.s1*B.s2*C.s3) - (B.s3*C.s2*D.s1) - (C.s3*D.s2*B.s1) - (D.s3*B.s2*C.s1));
    res -= A.s1 * ((B.s0*C.s2*D.s3) + (C.s0*D.s2*B.s3) + (D.s0*B.s2*C.s3) - (B.s3*C.s2*D.s0) - (C.s3*D.s2*B.s0) - (D.s3*B.s2*C.s0));
    res += A.s2 * ((B.s0*C.s1*D.s3) + (C.s0*D.s1*B.s3) + (D.s0*B.s1*C.s3) - (B.s3*C.s1*D.s0) - (C.s3*D.s1*B.s0) - (D.s3*B.s1*C.s0));
    res -= A.s3 * ((B.s0*C.s1*D.s2) + (C.s0*D.s1*B.s2) + (D.s0*B.s1*C.s2) - (B.s2*C.s1*D.s0) - (C.s2*D.s1*B.s0) - (D.s2*B.s1*C.s0));
    return res;
}

bool intersect_tetrahedron(const struct Tetrahedron* tetra, const struct Ray4* ray, float* t){
   
    float4 p = ray->origin + (ray->dir * (*t));

    float detABCD = det4(tetra->v0, tetra->v1, tetra->v2, tetra->v3);

    float detPBCD = det4(p, tetra->v1, tetra->v2, tetra->v3);
    float detAPCD = det4(tetra->v0, p, tetra->v2, tetra->v3);
    float detABPD = det4(tetra->v0, tetra->v1, p, tetra->v3);
    float detABCP = det4(tetra->v0, tetra->v1, tetra->v2, p);

    float x1 = detPBCD/detABCD;
    float x2 = detAPCD/detABCD;
    float x3 = detABPD/detABCD;
    float x4 = detABCP/detABCD;
  
    if ((x1 < 0) || (x2 < 0) || (x3 < 0) || (x4 < 0)) {return false;}
    else {return true;}
}

struct Ray createCamRay(const int x_coord, const int y_coord, const int width, const int height) {
    float fx = (float)x_coord / (float)width;
    float fy = (float)y_coord / (float)height;

    float aspect_ratio = (float)width / (float)height;
    float fx2 = (fx - 0.5f) * aspect_ratio;
    float fy2 = fy - 0.5f;

    float3 pixel_pos = (float3)(fx2, -fy2, 0.0f);

    struct Ray ray;
    ray.origin = (float3)(0.0f, 0.0f, 1.0f);
    ray.dir = normalize(pixel_pos - ray.origin);

    return ray;
}

struct Ray4 createCamRay4D(const int x_coord, const int y_coord, const int z_coord, const int width, const int height, const int depth) {
    float fx = (float)x_coord / (float)width;
    float fy = (float)y_coord / (float)height;
    float fz = (float)z_coord;

    float aspect_ratio = (float)width / (float)height;
    float fx2 = (fx - 0.5f) * aspect_ratio;
    float fy2 = fy - 0.5f;

    float4 pixel_pos = (float4)(fx2, -fy2, fz, 0.0f);

    struct Ray4 ray;
    ray.origin = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
    ray.dir = normalize(pixel_pos - ray.origin);

    return ray;
}



bool intersect_triangle(const struct Triangle* triangle, const struct Ray* ray, float *t){
        float3 v0v1 = triangle->v1 - triangle->v0;
        float3 v0v2 = triangle->v2 - triangle->v0;
        
        float3 pvec = cross(ray->dir, v0v2);
        float det = dot(v0v1, pvec);

        if (det < epsilon){return false;}
        if (fabs(det) < epsilon) {return false;}

        float invDet = 1/det;

        float3 tvec = triangle->v0 - ray->origin;
        float u = dot(tvec, pvec) * invDet;
        if (u < 0 || u > 1) {return false;}

        float3 qvec = cross(tvec, v0v1);
        float v = dot(ray->dir, qvec) * invDet;
        if (v < 0 || u + v > 1){return false;}

        *t = dot(v0v2, qvec) * invDet;

        return true;
}

bool intersect4d(const struct Ray4* ray, float *tnear, float3 *tetraMesh ){
    for (int i=0; i<sizeof(tetraMesh)/sizeof(float3); i++){
       struct Tetrahedron tetra;

    }
}

__kernel void render_triangle(__global float3* cl_output,  int img_width, int img_height, struct Triangle tri){
    const int id = get_global_id(0);
    int x = id % img_width;
    int y = id / img_width;

    struct Ray camray = createCamRay(x, y, img_width, img_height);

    float t = 1e20;
    intersect_triangle(&tri, &camray, &t);

    if (t > 1e19){
        cl_output[id] = (float3)(0.0f, 0.0f, 0.0f);
        return;
    }

    cl_output[id] = tri.color;
}

__kernel void render_4d_to_3d(__global float3* cl_output, int width, int height, int depth){
    const int id = get_global_id(0);
    int x = id % width;
    int z = id / (width*height);
    int y = (id - z*width*height )/ width;
    

    struct Ray4 camray = createCamRay4D(x, y, z, width, height, depth);


    cl_output[id] = (float3)(0.0f, 0.0f, 0.0f);
}