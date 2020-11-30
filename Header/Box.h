#ifndef BOX_H
#define BOX_H

#include "Renderer.h"

#include "Rectangle.h"
#include "Hittable_List.h"

class Box : public Hittable  {
    public:
        Box() {}
        Box(const Point3& p0, const Point3& p1, shared_ptr<Material> ptr);

        virtual bool hit(const Ray& r, double t0, double t1, hit_record& rec) const override;

        virtual bool bounding_box(double t0, double t1, BoundingBox& output_Box) const override {
            output_Box = BoundingBox(Box_min, Box_max);
            return true;
        }

    public:
        Point3 Box_min;
        Point3 Box_max;
        Hittable_List sides;
};

Box::Box(const Point3& p0, const Point3& p1, shared_ptr<Material> ptr) {
    Box_min = p0;
    Box_max = p1;

    sides.add(make_shared<XY_Rectangle>(p0.x(), p1.x(), p0.y(), p1.y(), p1.z(), ptr));
    sides.add(make_shared<XY_Rectangle>(p0.x(), p1.x(), p0.y(), p1.y(), p0.z(), ptr));

    sides.add(make_shared<XZ_Rectangle>(p0.x(), p1.x(), p0.z(), p1.z(), p1.y(), ptr));
    sides.add(make_shared<XZ_Rectangle>(p0.x(), p1.x(), p0.z(), p1.z(), p0.y(), ptr));

    sides.add(make_shared<YZ_Rectangle>(p0.y(), p1.y(), p0.z(), p1.z(), p1.x(), ptr));
    sides.add(make_shared<YZ_Rectangle>(p0.y(), p1.y(), p0.z(), p1.z(), p0.x(), ptr));
}

bool Box::hit(const Ray& r, double t0, double t1, hit_record& rec) const {
    return sides.hit(r, t0, t1, rec);
}

#endif