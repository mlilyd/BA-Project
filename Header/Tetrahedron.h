#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "Ray4.h"
#include "Hittable4.h"
#include "vec4.h"

class Tetrahedron : public Hittable4 {
public:
    Tetrahedron() {}
    Tetrahedron(Point4 a, Point4 b, Point4 c, Point4 d, shared_ptr<Material> m) : A(a), B(b), C(c), D(d), mat_ptr(m) {};

    virtual bool hit(
        const Ray4& r, double tmin, double tmax, hit_record& rec) const override;

private:
    Point4 A;
    Point4 B;
    Point4 C;
    Point4 D;
    shared_ptr<Material> mat_ptr;
};

bool Tetrahedron::hit(const Ray4& r, double tmin, double tmax, hit_record& rec) const {
    //calculate using barycentric coordinates and Cramer's rule for solving linear system
    vec3 P = r.direction();

    vec3 v0 = C - A;
    vec3 v1 = B - A;
    vec3 v2 = P - A;

    double d00 = dot(v0, v0);
    double d01 = dot(v0, v1);
    double d02 = dot(v0, v2);
    double d11 = dot(v1, v1);
    double d12 = dot(v1, v2);

    double denom = 1 / ((d00 * d11) - (d01 * d01));
    double u = ((d11 * d02) - (d01 * d12)) * denom;
    double v = ((d00 * d12) - (d01 * d02)) * denom;

    return (u >= 0) && (v >= 0) && (u + v < 1);


}




#endif