#pragma once
#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "Renderer.h"

class Mesh : public Hittable {
public:
    Mesh() {}
    virtual bool hit(const Ray& r, double tmin, double tmax, hit_record& rec) const override;
    virtual bool bounding_box(double t0, double t1, BoundingBox& output_box) const override;

    void AddTriangle(Triangle& t);

private:
    Hittable_List triangles;
 };

bool Mesh::hit(const Ray& r, double tmin, double tmax, hit_record& rec) const {
    return triangles.hit(r, tmin, tmax, rec);


}

bool Mesh::bounding_box(double t0, double t1, BoundingBox& output_box) const {
    triangles.bounding_box(t0, t1, output_box);
}

void Mesh::AddTriangle(Triangle& t) {
    triangles.add(t);
}

#endif