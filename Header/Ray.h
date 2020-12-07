#ifndef RAY_H
#define RAY_H

#include "vec3.h"

class Ray{

    private: 
        Point3 orig;
        vec3 dir;

    public:
        Ray(){}
        Ray(const Point3& origin, const vec3& direction)
            : orig(origin), dir(direction) {}
        
        Point3 origin() const {return orig;}
        vec3 direction() const {return dir;}

        Point3 at (double t) const {
            return orig + t*dir;
        }
};

#endif