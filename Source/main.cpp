#include "Renderer.h"
#include "Hittable.h"
#include "Hittable_List.h"
#include "Sphere.h"
#include "Camera.h"


#include <iostream>


vector<Color> simple_gradient(const int width, const int height){

    vector<Color> img_pixels;
    for (int j=height-1;j>=0; j--){
        for (int i=0;i<width;i++){
            Color pixel_Color(double(i)/(width-1), double(j)/(height-1), 0.20);
            img_pixels.push_back(pixel_Color);
        }
    }
    return img_pixels;

}
/*
double hit_sphere(const Point3& center, double radius, const Ray& r) {

    vec3 oc = r.origin() - center;
    auto a = r.direction().length_squared();
    auto half_b = dot(oc, r.direction());
    auto c = oc.length_squared() - radius*radius;
    auto discriminant = half_b*half_b - a*c;
    if (discriminant < 0){
        return -1.0;
    } else {
        return (-half_b - sqrt(discriminant) ) / a;
    }

}
*/

Color ray_color(const Ray& r, const Hittable& scene, const Color& background, int depth) {
    hit_record rec;

    if (depth <= 0){
        return Color(0.0, 0.0, 0.0);
    }
    /*
    if (!scene.hit(r, 0.001, infinity, rec)){
        return background;
    }
    */
    if (scene.hit(r, 0.001, infinity, rec)){
        Ray scattered;
        Color attenuation;
        if (rec.material_ptr->scatter(r, rec, attenuation, scattered)){
            return attenuation * ray_color(scattered, scene, background,  depth-1);
        }
    }
    
    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5*(unit_direction.y() + 1.0);
    return (1.0-t)*Color(1.0, 1.0, 1.0) + t*Color(0.5, 0.7, 1.0);
}

double ambient_occlusion(const Ray &r, const hit_record &rec, const Hittable &scene, int ray_shot){

    Point3 p = rec.p;
    int ray_hit = 0;
    vec3 normal = rec.normal;

    for(int i=0;i<ray_shot;i++){
        vec3 dest = random_in_hemisphere(normal);
        Ray r = Ray(p, dest);
        hit_record temp_rec;
        if (scene.hit(r, 0.0001, infinity, temp_rec)){
            ray_hit += 1;
        }
    }

    return 1.0 - (ray_hit/ray_shot);
}

Color ray_AO(const Ray& r, const Hittable& scene, int depth, int ray_shot) {
    hit_record rec;

    if (depth <= 0){
        return Color(0.0, 0.0, 0.0);
    }

    if (scene.hit(r, 0.001, infinity, rec)){
        return Color(1.0, 1.0, 1.0) * ambient_occlusion(r, rec, scene, ray_shot);
    }
    else{
        return Color(1.0, 1.0, 1.0);
    }
}

vector<Color> render(Camera camera, Hittable_List scene, const Color& background, const int img_width, const int img_height, int samples_per_pixel, int max_depth){
    vector<Color> img_pixels;
    for (int j = img_height-1; j>=0; --j){
        for (int i = 0; i<img_width; ++i){
            Color pixel(0,0,0);
            double AO_coeff = 0.0;
            for (int s=0;s<samples_per_pixel; ++s){
                auto u = (i + random_double())/(img_width-1);
                auto v = (j + random_double())/(img_height-1);
                Ray r = camera.get_ray(u,v);
                pixel += ray_color(r, scene, background, max_depth); 
            }
            img_pixels.push_back(pixel);
        }
    }
    return img_pixels;
}

vector<Color> render_AO(Camera camera, Hittable_List scene, const Color& background, const int img_width, const int img_height, int samples_per_pixel, int max_depth, int ray_shot){
    vector<Color> img_pixels;
    for (int j = img_height-1; j>=0; --j){
        for (int i = 0; i<img_width; ++i){
            Color pixel(0,0,0);
            double AO_coeff = 0.0;
            for (int s=0;s<samples_per_pixel; ++s){
                auto u = (i + random_double())/(img_width-1);
                auto v = (j + random_double())/(img_height-1);
                Ray r = camera.get_ray(u,v);
                pixel += ray_AO(r, scene, max_depth, ray_shot); 
            }
            img_pixels.push_back(pixel);
        }
    }
    return img_pixels;
}

Hittable_List spheres(){
    
    //set up scene
    Hittable_List scene;

    //auto material_1 = make_shared<Lambertian>(Color(1.0, 1.0, 0));
    //auto material_2 = make_shared<Lambertian>(Color(0.25, 1.0, 0.1));

    ///scene.add(make_shared<Sphere>(Point3(0,0,-1), 0.5, material_1));
    //scene.add(make_shared<Sphere>(Point3(0,-100.5,-1), 100, material_2));

    auto material_ground = make_shared<Lambertian>(Color(0.8, 0.8, 0.0));
    auto material_center = make_shared<Lambertian>(Color(0.7, 0.3, 0.3));
  
    scene.add(make_shared<Sphere>(Point3( 0.0, -100.5, -1.0), 100.0, material_ground));
    scene.add(make_shared<Sphere>(Point3( 0.0,    0.0, -1.0),   0.5, material_center));
    
    return scene;

}

Hittable_List boxes(){

    Hittable_List scene;
    auto red   = make_shared<Lambertian>(Color(.65, .05, .05));
    auto white = make_shared<Lambertian>(Color(.73, .73, .73));
    auto green = make_shared<Lambertian>(Color(.12, .45, .15));
    auto blue = make_shared<Lambertian>(Color(.15, .12, .45));


    /*scene.add(make_shared<XZ_Rectangle>(-1000, 1000, -1000, 1000, 0, white));

    scene.add(make_shared<Box>(Point3(130, 0, 65),  Point3(295, 165, 230), green));
    scene.add(make_shared<Box>(Point3(-65, 0, 65),  Point3(100, 165, 230), green));
    scene.add(make_shared<Box>(Point3(-250, 0, 65),  Point3(-85, 165, 230), green));

    scene.add(make_shared<Box>(Point3(265, 0, 295), Point3(430, 330, 460), red));
    scene.add(make_shared<Box>(Point3(500, 0, 34), Point3(600, 200, 200), blue));

    scene.add(make_shared<Sphere>(Point3( 400, 50, 10),   100, green));
    */
    scene.add(make_shared<Sphere>(Point3(0,0,0), 10, red));
    scene.add(make_shared<Triangle>(Point3(-10, 0, 0), Point3(-5, 0, 0), Point3(-3, 5, 0), blue));

    return scene;

}

int main(){

    // openCL declarations
    cl_platform_id platform;
    cl_device_id device_id;
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel k_mult;


    const int width = 500;
    const auto aspect_ratio = 16.0/9.0;
    const int height = width/aspect_ratio;
    const int samples = 200;
    
    const int max_depth = 5;
    const int ray_shot = 2;
    //Camera camera(Point3(-100, 500,-1000), Point3(300, 300, 0), vec3(0,1,0), 45, aspect_ratio);
    Camera camera(Point3(0, 10,-100), Point3(0, 0, 0), vec3(0,1,0), 45, aspect_ratio);

    const Color background = Color(0.0, 0.0, 0.0);

    Hittable_List scene = boxes();
    vector<Color> ao;
    vector<Color> img = render(camera, scene, background, width, height, samples, max_depth);
    //vector<Color> img_ao = render_AO(camera, scene, background, width, height, samples, max_depth, ray_shot);

    write_img_jpg(img, "renders/world_render_11_tesseract.jpg", width, height, samples);
    //write_img_jpg(img_ao, "renders/world_render_11_scene_ao_only.jpg", width, height, samples);


}
