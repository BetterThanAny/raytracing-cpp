#pragma once

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "color.h"
#include "vec3.h"

#include <cstdio>
#include <string>
#include <vector>

inline bool write_png(const std::string& path, int width, int height,
                      const std::vector<color>& framebuffer) {
    std::vector<uint8_t> bytes(width * height * 3);
    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            encode_color(&bytes[(j * width + i) * 3], framebuffer[j * width + i]);
        }
    }
    return stbi_write_png(path.c_str(), width, height, 3, bytes.data(), width * 3) != 0;
}

inline bool write_ppm(const std::string& path, int width, int height,
                      const std::vector<color>& framebuffer) {
    FILE* f = std::fopen(path.c_str(), "w");
    if (!f) return false;
    if (std::fprintf(f, "P3\n%d %d\n255\n", width, height) < 0) {
        std::fclose(f);
        return false;
    }
    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            uint8_t rgb[3];
            encode_color(rgb, framebuffer[j * width + i]);
            if (std::fprintf(f, "%d %d %d\n", rgb[0], rgb[1], rgb[2]) < 0) {
                std::fclose(f);
                return false;
            }
        }
    }
    return std::fclose(f) == 0;
}
