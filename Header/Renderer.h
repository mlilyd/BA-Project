#ifndef RENDERER_H
#define RENDERER_H

#include <cmath>
#include <cstdlib>
#include <limits>
#include <memory>
#include <vector>
#include <random>

#include <CL/cl.h>
#include <CL/opencl.h>

// define classes and functions from the standard library

using std::shared_ptr;
using std::vector;
using std::make_shared;
using std::sqrt;

// define constants

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// utility functions

inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}

//returns a random real number in [0,1)
inline double random_double() {
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    static std::mt19937 generator;
    return distribution(generator);
}

//return a random real number in [min,max)
inline double random_double(double min, double max){
    return min + (max-min)*random_double();
}


inline double clamp(double x, double min, double max){
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

//returns a random integer in [min, max]
inline int random_int(int min, int max){
    return static_cast<int>(random_double(min, max+1));
}
// include other headers
#include "Write.h"
#include "Ray.h"
#include "vec3.h"
#include "Hittable.h"
#include "Hittable_List.h"
#include "Material.h"
#include "Sphere.h"
#include "Rectangle.h"
#include "Box.h"
#include "Triangle.h"



#include "vec4.h"
#include "Tetrahedron.h"
#include "Ray4.h"
#include "Camera4.h"




#endif