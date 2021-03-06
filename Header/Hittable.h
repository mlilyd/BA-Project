#ifndef HITTABLE_H
#define HITTABLE_H

#include "Renderer.h"
#include "ray.h"
#include "BoundingBox.h"

class Material;

struct hit_record {
    Point3 p;
    vec3 normal;
    shared_ptr<Material> material_ptr;
    double t;
    double u;
    double v;
    bool front_face;

    inline void set_face_normal(const Ray& r, const vec3& outward_normal){
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class Hittable {
    public:
        virtual bool hit(const Ray& r,
                        double t_min,
                        double t_max,
                        hit_record& rec) const = 0;
        virtual bool bounding_box(double t0, double t1, BoundingBox& output_box) const = 0;
        
};


#endif