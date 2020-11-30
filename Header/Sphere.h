#ifndef SPHERE_H
#define SPHERE_H

#include "Hittable.h"
#include "vec3.h"

class Sphere : public Hittable {
    public:
        Sphere() {}
        Sphere(Point3 cen, double r, shared_ptr<Material> m) : center(cen), radius(r), mat_ptr(m){};

        virtual bool hit(
            const Ray& r, double tmin, double tmax, hit_record& rec) const override;
        virtual bool bounding_box(double t0, double t1, BoundingBox& output_box) const override; 
       
    private:
        Point3 center;
        double radius; 
        shared_ptr<Material> mat_ptr;
};

bool Sphere::hit(const Ray& r, double t_min, double t_max, hit_record& rec) const {

    vec3 oc = r.origin() - center;
    auto a = r.direction().length_squared();
    auto half_b = dot(oc, r.direction());
    auto c = oc.length_squared() - radius*radius;
    auto discriminant = half_b * half_b - a*c;

    if (discriminant > 0){
        auto root = sqrt(discriminant);

        auto temp = (-half_b - root) / a;
        if (temp < t_max && temp > t_min){
            rec.t = temp;
            rec.p = r.at(rec.t);
            vec3 outward_normal = (rec.p - center) / radius;
            rec.set_face_normal(r, outward_normal);
            rec.material_ptr = mat_ptr;
            return true;
        }

        temp = (-half_b + root) / a;
        if (temp < t_max && temp > t_min){
            rec.t = temp;
            rec.p = r.at(rec.t);
            vec3 outward_normal = (rec.p - center) / radius;
            rec.set_face_normal(r, outward_normal);
            rec.material_ptr = mat_ptr;
            return true;
        }
    }
    return false;
}


bool Sphere::bounding_box(double t0, double t1, BoundingBox& output_box) const {
    output_box = BoundingBox(center - vec3(radius, radius, radius),
                             center + vec3(radius, radius, radius));
    return true;
}

#endif