#ifndef TETRAHEDRON_H
#define TETRAHEDRON_H

#include "Ray4.h"
#include "Hittable4.h"
#include "vec4.h"

class Tetrahedron : public Hittable4 {
public:
    Tetrahedron() {}
    Tetrahedron(Point4 a, Point4 b, Point4 c, Point4 d, shared_ptr<Material> m) : A(a), B(b), C(c), D(d), mat_ptr(m) {};

    virtual bool hit(const Ray4& r, double tmin, double tmax, hit_record4& rec) const;

private:
    Point4 A;
    Point4 B;
    Point4 C;
    Point4 D;
    shared_ptr<Material> mat_ptr;
    vec4 normal;
};

double determinant4(vec4 v0, vec4 v1, vec4 v2, vec4 v3) {
      double det =  v0.w()* v1.z()* v2.y()* v3.x() - v0.z() * v1.w() * v2.y() * v3.x() -
                    v0.w() * v1.y() * v2.z() * v3.x() + v0.y() * v1.w() * v2.z() * v3.x() +

                    v0.z() * v1.y() * v2.w() * v3.x() - v0.y() * v1.z() * v2.w() * v3.x() -
                    v0.w() * v1.z() * v2.x() * v3.y() + v0.z() * v1.w() * v2.x() * v3.y() +

                    v0.w() * v1.x() * v2.z() * v3.y() - v0.x() * v1.w() * v2.z() * v3.y() -
                    v0.z() * v1.x() * v2.w() * v3.y() + v0.x() * v1.z() * v2.w() * v3.y() +

                    v0.w() * v1.y() * v2.x() * v3.z() - v0.y() * v1.w() * v2.x() * v3.z() -
                    v0.w() * v1.x() * v2.y() * v3.z() + v0.x() * v1.w() * v2.y() * v3.z() +

                    v0.y() * v1.x() * v2.w() * v3.z() - v0.x() * v1.y() * v2.w() * v3.z() -
                    v0.z() * v1.y() * v2.x() * v3.w() + v0.y() * v1.z() * v2.x() * v3.w() +

                    v0.z() * v1.x() * v2.y() * v3.w() - v0.x() * v1.z() * v2.y() * v3.w() -
                    v0.y() * v1.x() * v2.z() * v3.w() + v0.x() * v1.y() * v2.z() * v3.w();
      return det;
}

bool Tetrahedron::hit(const Ray4& r, double tmin, double tmax, hit_record4& rec) const {
    //calculate using barycentric coordinates and Cramer's rule for solving linear system
    double t = dot(normal, r.direction());
    vec4 P = r.at(t);

    double det0 = determinant4(A, B, C, D);
    double det1 = determinant4(P, B, C, D);
    double det2 = determinant4(A, P, C, D);
    double det3 = determinant4(A, B, P, D);
    double det4 = determinant4(A, B, C, P);


    double bary_coord0 = det1 / det0;
    double bary_coord1 = det2 / det0;
    double bary_coord2 = det3 / det0;
    double bary_coord3 = det4 / det0;

    return (bary_coord0 >= 0) && (bary_coord1 >= 0) &&
            (bary_coord2 >= 0) && (bary_coord3 >= 0);

}
#endif