#ifndef RAY4_H
#define RAY4_H

#include "vec4.h"

class Ray4 {

private:
    Point4 orig;
    vec4 dir;

public:
    Ray4() {}
    Ray4(const Point4& origin, const vec4& direction)
        : orig(origin), dir(direction) {}

    Point4 origin() const { return orig; }
    vec4 direction() const { return dir; }

    //need a 2nd parameter...?
    Point4 at(double t) const {
        return orig + t * dir;
    }



};

#endif