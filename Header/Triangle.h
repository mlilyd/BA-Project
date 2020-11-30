#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "Hittable.h"
#include "vec3.h"

class Triangle : public Hittable {
    public: 
        Triangle() {}
        Triangle(Point3 a, Point3 b, Point3 c, shared_ptr<Material> m): A(a), B(b), C(c), mat_ptr(m){
            vec3 temp = cross((B - A), (C - A));
            normal = temp/temp.length();
        };

        virtual bool hit(
            const Ray& r, double tmin, double tmax, hit_record& rec) const override;
        virtual bool bounding_box(double t0, double t1, BoundingBox& output_box) const override; 

    private:
        Point3 A;
        Point3 B;
        Point3 C;
        shared_ptr<Material> mat_ptr;
        vec3 normal;
};

bool Triangle::hit(const Ray& r, double tmin, double tmax, hit_record& rec) const{
    //calculate using barycentric coordinates and Cramer's rule for solving linear system

    double temp = dot(normal, A);
    double t = -(dot(normal, r.origin()) + temp) / (dot(normal, r.direction()));

    vec3 P = r.at(t);
    vec3 v0 = C-A;
    vec3 v1 = B-A;
    vec3 v2 = P-A;

    double d00 = dot(v0, v0);
    double d01 = dot(v0, v1);
    double d02 = dot(v0, v2);
    double d11 = dot(v1, v1);
    double d12 = dot(v1, v2);

    double denom = 1/((d00*d11)-(d01*d01));
    double u = ((d11*d02) - (d01*d12))*denom;
    double v = ((d00*d12) - (d01*d02))*denom;

    return (u>= 0) && (v>= 0) && (u+v<1);


}

bool Triangle::bounding_box(double t0, double t1, BoundingBox& output_box) const {
    output_box = BoundingBox(A, vec3(B.x(),C.y(), B.z()));
    return true;
}



#endif