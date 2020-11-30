#ifndef MATERIAL_H
#define MATERIAL_H

#include "Renderer.h"

struct hit_record;

class Material {
    public:
        virtual bool scatter(
            const Ray& r_in, const hit_record& rec, Color& attenuation, Ray& scattered
        ) const = 0;

        virtual Color emitted(double u, double v, const Point3& p) const {
            return Color(0,0,0);
        }
};

class Lambertian : public Material {
    public:
        Lambertian(const Color& a) : albedo(a) {}

        virtual bool scatter(
            const Ray& r_in, const hit_record& rec, Color& attenuation, Ray& scattered
        ) const override {
            vec3 scatter_direction = rec.normal + random_unit_vector();
            scattered = Ray(rec.p, scatter_direction);
            attenuation = albedo;
            return true;
        }

    public:
        Color albedo;
};


#endif