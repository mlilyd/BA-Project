#include <CL/opencl.hpp>
#include <CL/cl.h>
#include <vector>
#include <random>
#include <cmath>

#define float3(x, y, z) {{x, y, z}} 
#define float4(x, y, z, w) {{x, y, z, w}}

// CONSTANT DEFINITIONS
const int width = 300;
const int height = 300;
const int depth = 5;
const float epsilon = 0.00003f;

struct Ray{
    cl_float3 origin;
    cl_float3 dir;
};

struct Ray4{
    cl_float4 origin;
    cl_float4 dir;
};

cl_float4 normalize(cl_float4 v) {
	float length = std::sqrt(v.s0 * v.s0 + v.s1 * v.s1 + v.s2 * v.s2 + v.s3 * v.s3);
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

bool intersect_tetrahedron(cl_float4 v0, cl_float4 v1, cl_float4 v2, cl_float4 v3, Ray4 ray, float& t) {

	cl_float4 v0v1 = v1 - v0;
	cl_float4 v0v2 = v2 - v0;
	cl_float4 v0v3 = v3 - v0;
	cl_float4 Tvec = ray.origin - v0;

	float detM = det4(ray.dir, v0v1, v0v2, v0v3);
	if (detM < epsilon) { return false; }
	if (std::fabs(detM) < epsilon) { return false; }

	float invDet = 1 / detM;

	float Mt = det4(Tvec, v0v1, v0v2, v0v3);
	float My = det4(ray.dir, Tvec, v0v2, v0v3);
	float Mz = det4(ray.dir, v0v1, Tvec, v0v3);
	float Mw = det4(ray.dir, v0v1, v0v2, Tvec);

	float y = My * invDet;
	if (y < 0) { return false; }

	float z = Mz * invDet;
	if (z < 0) { return false; }

	float w = Mw * invDet;
	if (w < 0 || y + z + w > 1) { return false; }

	t = Mt * invDet;

	return true;
}

bool intersect_mesh(Ray4 ray, std::vector<cl_float4>vertices, int vol, int* vertIndex) {
	for (int i = 0; i < vol; i++) {
		cl_float4 v0 = vertices[vertIndex[0 + i * 4]];
		cl_float4 v1 = vertices[vertIndex[1 + i * 4]];
		cl_float4 v2 = vertices[vertIndex[2 + i * 4]];
		cl_float4 v3 = vertices[vertIndex[3 + i * 4]];

		float t = 1e20;
		bool intersect = intersect_tetrahedron(v0, v1, v2, v3, ray, t);
		if (intersect) {
			return true;
		}
	}
	return false;
}

cl_float3 sample_hemisphere(cl_float3 center, float radius, float theta, float phi){
	
	float x = center.s0 + radius * sin(theta) * cos(phi);
	float y = center.s1 + radius * sin(theta) * sin(phi;
	float z = center.s2 + radius * cos(theta);

	return float3(x, y, z);

}

cl_float4 sample_hemisphere4d(cl_float4 center, float radius, float theta, float phi, float psi){
	
	float x = center.s0 + radius * cos(psi);
	float y = center.s1 + radius * sin(psi) * cos(theta);
	float z = center.s2 + radius * sin(psi) * sin(theta) * cos(phi);
	float w = center.s3 + radius * sin(psi) * sin(theta) * sin(phi);

	return float4(x, y, z, w);
}

float random_theta(){
	return 0.0f;
}

float random_phi(){
	return 0.0f;
}

float random_angle(){
	return 0.0f;
}

std::vector<float> get_ao4d(std::vector<cl_float4> vertices, int vol, int* vertIndex, float radius, int samples){

	std::vector<float> ao_values(vertices.size());
	
	float ao;
		
	for (int i=0; i<vertices.size(); i++){
		cl_float4 vertex = vertices[i];

		Ray4 ray;
		ray.origin = vertex;

		for (int j=0; j<samples; j++){
			ray.dir = sample_hemisphere4d(vertex, radius, random_angle(), random_angle(), random_angle());
			if (intersect_mesh(ray, vertices, vol, vertIndex)){
				ao += 1;
			}
		}
		ao = ao/samples;
		ao_values[i] = ao;
	}

	return ao_values;

}