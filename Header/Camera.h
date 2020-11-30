#ifndef CAMERA_H
#define CAMERA_H

#include "Renderer.h"

class Camera {
    public:
        Camera(Point3 lookfrom, Point3 lookat, vec3 vup, double vfov = 2.0, double aspect_ratio = 16.0/9.0){
            auto theta = degrees_to_radians(vfov);
            auto h = tan(theta/2);
            auto viewport_height = 2.0 * h;
            auto viewport_width = aspect_ratio * viewport_height;
            
            auto w = unit_vector(lookfrom-lookat);
            auto u = unit_vector(cross(vup, w));
            auto v = cross(w,u);

            
            origin = lookfrom;
            horizontal = viewport_width * u;
            vertical = viewport_height * v;
            lower_left_corner = origin - horizontal/2 - vertical/2 - w;

            /*
            auto focal_length = 1.0;
            origin = Point3(0,0,0);
            horizontal = vec3(viewport_width, 0.0, 0.0);
            vertical = vec3(0.0, viewport_height, 0.0);
            lower_left_corner = origin - horizontal/2 - vertical/2 - vec3(0, 0, focal_length);
            */
        }

        Ray get_ray(double u, double v) const {
            return Ray(origin, lower_left_corner + u*horizontal + v*vertical - origin);
        }

    private:
        Point3 origin;
        Point3 lower_left_corner;
        vec3 horizontal;
        vec3 vertical;
};
#endif