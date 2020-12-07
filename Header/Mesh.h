#pragma once
#ifndef MESH_H
#define MESH_H

#include "Renderer.h"

class Mesh : public Hittable {
public:
    Mesh() {}
    virtual bool hit(const Ray& r, double tmin, double tmax, hit_record& rec) const override;
    virtual bool bounding_box(double t0, double t1, BoundingBox& output_box) const override;

    void AddTriangle(Triangle& t);

private:
    std::vector<Triangle> triangles;
 };

bool Mesh::hit(const Ray& r, double tmin, double tmax, hit_record& rec) const {
    
    hit_record temp;
    bool hit_anything = false;
    auto closest_so_far = tmax;

    for (Triangle t : triangles) {
        if (t.hit(r, tmin, tmax, temp)) {
            hit_anything = true;
            closest_so_far = temp.t;
            rec = temp;
            break;
        }
    }
    return hit_anything;
}

bool Mesh::bounding_box(double t0, double t1, BoundingBox& output_box) const {
    if (triangles.empty()) {
        return false;
    }
    BoundingBox temp_box;
    bool first_box = true;
    for (Triangle t: triangles) {
        if (t.bounding_box(t0, t1, temp_box)) return false;
        output_box = first_box ? temp_box : surrounding_box(output_box, temp_box);
        first_box = false;
    }
}

void Mesh::AddTriangle(Triangle& t) {
    triangles.push_back(t);
}

#endif