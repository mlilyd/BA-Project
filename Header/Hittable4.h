#pragma once
#ifndef HITTABLE_H
#define HITTABLE_H

#include "Renderer.h"
#include "BoundingBox.h"
#include "Ray4.h"

class Material;

struct hit_record {
    Point4 p;
    vec4 normal;
    shared_ptr<Material> material_ptr;
    double t;
    double u;
    double v;
    bool front_face;

    inline void set_face_normal(const Ray4& r, const vec4& outward_normal) {
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class Hittable4 {
public:
    virtual bool hit(const Ray4& r,
        double t_min,
        double t_max,
        hit_record& rec) const = 0;

};


#endif