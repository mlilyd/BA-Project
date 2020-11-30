#pragma once
#ifndef vec4_H
#define vec4_H

#include <cmath>
#include <iostream>
#include "Renderer.h"

using std::sqrt;

class vec4 {
public:
    vec4() : e{ 0,0,0 } {}
    vec4(double e0, double e1, double e2, double e3) : e{ e0, e1, e2, e3} {}

    double x() const { return e[0]; }
    double y() const { return e[1]; }
    double z() const { return e[2]; }
    double w() const { return e[3]; }

    vec4 operator-() const { return vec4(-e[0], -e[1], -e[2], -e[3]); }
    double operator[](int i) const { return e[i]; }
    double& operator[](int i) { return e[i]; }

    vec4& operator+=(const vec4& v) {
        e[0] += v.e[0];
        e[1] += v.e[1];
        e[2] += v.e[2];
        e[3] += v.e[3];
        return *this;
    }

    vec4& operator*=(const double t) {
        e[0] *= t;
        e[1] *= t;
        e[2] *= t;
        e[3] *= t;
        return *this;
    }

    vec4& operator/=(const double t) {
        return *this *= 1 / t;
    }

    double length() const {
        return sqrt(length_squared());
    }

    double length_squared() const {
        return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
    }

    inline static vec4 random() {
        return vec4(random_double(), random_double(), random_double(), random_double());
    }

    inline static vec4 random(double min, double max) {
        return vec4(random_double(min, max), random_double(min, max), random_double(min, max), random_double(min, max));
    }

public:
    double e[4];
};


// Type aliases for vec4
using Point4 = vec4;   // 4D point


// vec4 Utility Functions

inline std::ostream& operator<<(std::ostream& out, const vec4& v) {
    return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

inline vec4 operator+(const vec4& u, const vec4& v) {
    return vec4(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2], u.e[3] + v.e[3]);
}

inline vec4 operator-(const vec4& u, const vec4& v) {
    return vec4(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2], u.e[3] - v.e[3]);
}

inline vec4 operator*(const vec4& u, const vec4& v) {
    return vec4(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2], u.e[3] * v.e[3]);
}

inline vec4 operator*(double t, const vec4& v) {
    return vec4(t * v.e[0], t * v.e[1], t * v.e[2], t * v.e[3]);
}

inline vec4 operator*(const vec4& v, double t) {
    return t * v;
}

inline vec4 operator/(vec4 v, double t) {
    return (1 / t) * v;
}

inline double dot(const vec4& u, const vec4& v) {
    return u.e[0] * v.e[0]
        + u.e[1] * v.e[1]
        + u.e[2] * v.e[2]
        + u.e[3] * v.e[3];
}


//FROM https://github.com/hollasch/ray4
inline vec4 cross(const vec4& u, const vec4& v, const vec4& w) {
    double A = (v[0] * w[1]) - (v[1] * w[0]);
    double B = (v[0] * w[2]) - (v[2] * w[0]);
    double C = (v[0] * w[3]) - (v[3] * w[0]);
    double D = (v[1] * w[2]) - (v[2] * w[1]);
    double E = (v[1] * w[3]) - (v[3] * w[1]);
    double F = (v[2] * w[3]) - (v[3] * w[2]);

    return vec4(
        (u[1] * F) - (u[2] * E) + (u[3] * D),
        -(u[0] * F) + (u[2] * C) - (u[3] * B),
        (u[0] * E) - (u[1] * C) + (u[3] * A),
        -(u[0] * D) + (u[1] * B) - (u[2] * A)
    );
}

inline vec4 unit_vector(vec4 v) {
    return v / v.length();
}

vec4 random_v4_in_unit_sphere() {
    while (true) {
        auto p = vec4::random(-1, 1);
        if (p.length_squared() >= 1) continue;
        return p;
    }
}

//doesn't really work
vec4 random_v4_unit_vector() {
    auto a = random_double(0, 2 * pi);
    auto z = random_double(-1, 1);
    auto r = sqrt(1 - z * z);
    return vec4(r * cos(a), r * sin(a), z, z);
}

vec4 random_v4_in_hemisphere(const vec4& normal) {
    vec4 in_unit_sphere = random_v4_in_unit_sphere();
    if (dot(in_unit_sphere, normal) > 0.0) // In the same hemisphere as the normal
        return in_unit_sphere;
    else
        return -in_unit_sphere;
}


#endif
