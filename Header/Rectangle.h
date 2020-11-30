#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "Renderer.h"

class XY_Rectangle : public Hittable {
    public:
        XY_Rectangle() {}

        XY_Rectangle(double _x0, double _x1, double _y0, double _y1, double _k,
            shared_ptr<Material> mat)
            : x0(_x0), x1(_x1), y0(_y0), y1(_y1), k(_k), mp(mat) {};

        virtual bool hit(const Ray& r, double t0, double t1, hit_record& rec) const override;

        
        virtual bool bounding_box(double t0, double t1, BoundingBox& output_box) const override {
            // The bounding box must have non-zero width in each dimension, so pad the Y
            // dimension a small amount.
            output_box = BoundingBox(Point3(x0, y0, k-0.0001), Point3(x1, y1, k+0.0001));
            return true;
        }
        

        vec3 surface_normal(){
            return vec3(0, 0, 1);
        }

    public:
        shared_ptr<Material> mp;
        double x0, x1, y0, y1, k;
};

class XZ_Rectangle : public Hittable {
    public:
        XZ_Rectangle() {}

        XZ_Rectangle(double _x0, double _x1, double _z0, double _z1, double _k,
            shared_ptr<Material> mat)
            : x0(_x0), x1(_x1), z0(_z0), z1(_z1), k(_k), mp(mat) {};

        virtual bool hit(const Ray& r, double t0, double t1, hit_record& rec) const override;

        virtual bool bounding_box(double t0, double t1, BoundingBox& output_box) const override {
            // The bounding box must have non-zero width in each dimension, so pad the Y
            // dimension a small amount.
            output_box = BoundingBox(Point3(x0,k-0.0001,z0), Point3(x1, k+0.0001, z1));
            return true;
        }

       vec3 surface_normal(){
            return vec3(0, 1, 0);
        }

    public:
        shared_ptr<Material> mp;
        double x0, x1, z0, z1, k;
};

class YZ_Rectangle : public Hittable {
    public:
        YZ_Rectangle() {}

        YZ_Rectangle(double _y0, double _y1, double _z0, double _z1, double _k,
            shared_ptr<Material> mat)
            : y0(_y0), y1(_y1), z0(_z0), z1(_z1), k(_k), mp(mat) {};

        virtual bool hit(const Ray& r, double t0, double t1, hit_record& rec) const override;

        virtual bool bounding_box(double t0, double t1, BoundingBox& output_box) const override {
            // The bounding box must have non-zero width in each dimension, so pad the X
            // dimension a small amount.
            output_box = BoundingBox(Point3(k-0.0001, y0, z0), Point3(k+0.0001, y1, z1));
            return true;
        }

        vec3 surface_normal(){
            return vec3(1, 0, 0);
        }

    public:
        shared_ptr<Material> mp;
        double y0, y1, z0, z1, k;
};

bool XY_Rectangle::hit(const Ray& r, double t0, double t1, hit_record& rec) const {
    auto t = (k-r.origin().z()) / r.direction().z();
    if (t < t0 || t > t1)
        return false;
    auto x = r.origin().x() + t*r.direction().x();
    auto y = r.origin().y() + t*r.direction().y();
    if ((x < x0) || (x > x1) || (y < y0) || (y > y1))
        return false;
    rec.u = (x-x0)/(x1-x0);
    rec.v = (y-y0)/(y1-y0);
    rec.t = t;
    auto outward_normal = vec3(0, 0, 1);
    rec.set_face_normal(r, outward_normal);
    rec.material_ptr = mp;
    rec.p = r.at(t);
    return true;
}

bool XZ_Rectangle::hit(const Ray& r, double t0, double t1, hit_record& rec) const {
    auto t = (k-r.origin().y()) / r.direction().y();
    if (t < t0 || t > t1)
        return false;
    auto x = r.origin().x() + t*r.direction().x();
    auto z = r.origin().z() + t*r.direction().z();
    if (x < x0 || x > x1 || z < z0 || z > z1)
        return false;
    rec.u = (x-x0)/(x1-x0);
    rec.v = (z-z0)/(z1-z0);
    rec.t = t;
    auto outward_normal = vec3(0, 1, 0);
    rec.set_face_normal(r, outward_normal);
    rec.material_ptr = mp;
    rec.p = r.at(t);
    return true;
}

bool YZ_Rectangle::hit(const Ray& r, double t0, double t1, hit_record& rec) const {
    auto t = (k-r.origin().x()) / r.direction().x();
    if (t < t0 || t > t1)
        return false;
    auto y = r.origin().y() + t*r.direction().y();
    auto z = r.origin().z() + t*r.direction().z();
    if (y < y0 || y > y1 || z < z0 || z > z1)
        return false;
    rec.u = (y-y0)/(y1-y0);
    rec.v = (z-z0)/(z1-z0);
    rec.t = t;
    auto outward_normal = vec3(1, 0, 0);
    rec.set_face_normal(r, outward_normal);
    rec.material_ptr = mp;
    rec.p = r.at(t);
    return true;
}

#endif