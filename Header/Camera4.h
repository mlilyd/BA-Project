#ifndef CAMERA4_H
#define CAMERA4_H

#include "Ray4.h"
#include "Renderer.h"

class Camera4 {
public:
    Camera4(Point4 lookfrom, Point4 lookat, vec4 vup, vec4 vov, double vfov = 2.0, double aspect_ratio = 16.0 / 9.0) {
        auto theta = degrees_to_radians(vfov);
        auto h = tan(theta / 2);
        auto viewport_height = 2.0 * h;
        auto viewport_width = aspect_ratio * viewport_height;

        auto w = unit_vector(lookfrom - lookat);
        auto u = unit_vector(cross(vup, w, vov));
        auto v = cross(w, u, vov);


        origin = lookfrom;
        over = vov;
        horizontal = viewport_width * u;
        vertical = viewport_height * v;
        lower_left_corner = origin - horizontal / 2 - vertical / 2 - w;
    }

    Ray4 get_ray(double u, double v) const {
        return Ray4(origin, lower_left_corner + u * horizontal + v * vertical - origin);
    }

private:
    Point4 origin;
    Point4 lower_left_corner;
    vec4 horizontal;
    vec4 vertical;
    vec4 over;
};
#endif