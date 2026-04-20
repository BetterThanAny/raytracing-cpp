#pragma once

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb_image.h"

#include <cstdlib>
#include <iostream>
#include <string>

class rt_image {
public:
    rt_image() = default;
    rt_image(const std::string& filename) { load(filename); }

    ~rt_image() {
        STBI_FREE(fdata);
        delete[] bdata;
    }

    bool load(const std::string& filename) {
        STBI_FREE(fdata);
        fdata = nullptr;
        delete[] bdata;
        bdata = nullptr;

        int n = bytes_per_pixel;
        fdata = stbi_loadf(filename.c_str(), &image_width, &image_height, &n, bytes_per_pixel);
        if (!fdata) return false;
        bytes_per_scanline = image_width * bytes_per_pixel;
        convert_to_bytes();
        return true;
    }

    int width()  const { return fdata ? image_width : 0; }
    int height() const { return fdata ? image_height : 0; }

    const unsigned char* pixel_data(int x, int y) const {
        static unsigned char magenta[] = {255, 0, 255};
        if (!bdata) return magenta;
        x = clamp(x, 0, image_width);
        y = clamp(y, 0, image_height);
        return bdata + y * bytes_per_scanline + x * bytes_per_pixel;
    }

private:
    static constexpr int bytes_per_pixel = 3;

    float*         fdata = nullptr;
    unsigned char* bdata = nullptr;
    int            image_width  = 0;
    int            image_height = 0;
    int            bytes_per_scanline = 0;

    static int clamp(int x, int low, int high) {
        if (x < low) return low;
        if (x < high) return x;
        return high - 1;
    }

    static unsigned char float_to_byte(float v) {
        if (v <= 0.0f) return 0;
        if (v >= 1.0f) return 255;
        return static_cast<unsigned char>(256.0f * v);
    }

    void convert_to_bytes() {
        int total_bytes = image_width * image_height * bytes_per_pixel;
        bdata = new unsigned char[total_bytes];
        for (int i = 0; i < total_bytes; ++i)
            bdata[i] = float_to_byte(fdata[i]);
    }
};
