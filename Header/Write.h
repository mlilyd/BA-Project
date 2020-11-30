#ifndef Color_H
#define Color_H

#include "vec3.h"

#include <vector>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include "Renderer.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void write_Color_ppm(std::vector<Color> img_pixels, char const*filename, int width, int height, int samples_per_pixel) {
    std::cout << "writing image to " << filename << "\n";
    
    std::ofstream file;
    file.open(filename);

    file << "P3\n" << width << " " << height << "\n255\n";
    for (int i=0;i<img_pixels.size();i++){
        Color pixel = img_pixels[i];
        auto r = pixel.x();
        auto g = pixel.y();
        auto b = pixel.z();

        //divide color by number of samples
        auto scale = 1.0 / samples_per_pixel;
            
        r = sqrt(scale*r);
        g = sqrt(scale*g);
        b = sqrt(scale*b);
        /*
        r *= scale;
        g *= scale;
        b *= scale;
        */
        file << static_cast<int>(256 * clamp(r, 0.0, 0.999)) << ' '
             << static_cast<int>(256 * clamp(g, 0.0, 0.999)) << ' '
             << static_cast<int>(256 * clamp(b, 0.0, 0.999)) << '\n';
    }

    file.close();
    
    std::cout << "Done!";
}

void write_img_jpg(std::vector<Color> img_pixels, char const *filename, int width, int height, int samples_per_pixel, int quality=100){
    std::cout << "writing image to " << filename << "\n";
    std::vector<uint8_t> img_data;

    for(int index,i=0;i<img_pixels.size();i++){
        Color pixel = img_pixels[i];
        auto r = pixel.x();
        auto g = pixel.y();
        auto b = pixel.z();

        //divide color by number of samples
        //color is the average of colors before 
        auto scale = 1.0 / samples_per_pixel;
        ///*
        r = sqrt(scale*r);
        g = sqrt(scale*g);
        b = sqrt(scale*b);
        ///*
        //r *= scale;
        //g *= scale;
        //b *= scale;
        
        img_data.push_back((uint8_t)(256 * clamp(r, 0.0, 0.999)));
        img_data.push_back((uint8_t)(256 * clamp(g, 0.0, 0.999)));
        img_data.push_back((uint8_t)(256 * clamp(b, 0.0, 0.999)));
    }

    stbi_write_jpg(filename,width,height,3, img_data.data(), quality);
    std::cout<< "Done!";
}

#endif