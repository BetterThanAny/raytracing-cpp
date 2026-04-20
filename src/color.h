#pragma once

#include "interval.h"
#include "vec3.h"

inline double linear_to_gamma(double linear) {
    return linear > 0 ? std::sqrt(linear) : 0;
}

inline void write_color(std::ostream& out, const color& pixel) {
    double r = pixel.x();
    double g = pixel.y();
    double b = pixel.z();

    r = linear_to_gamma(r);
    g = linear_to_gamma(g);
    b = linear_to_gamma(b);

    static const interval intensity(0.000, 0.999);
    int rbyte = static_cast<int>(256 * intensity.clamp(r));
    int gbyte = static_cast<int>(256 * intensity.clamp(g));
    int bbyte = static_cast<int>(256 * intensity.clamp(b));

    out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
}

inline void encode_color(uint8_t* dst, const color& pixel) {
    double r = linear_to_gamma(pixel.x());
    double g = linear_to_gamma(pixel.y());
    double b = linear_to_gamma(pixel.z());

    static const interval intensity(0.000, 0.999);
    dst[0] = static_cast<uint8_t>(256 * intensity.clamp(r));
    dst[1] = static_cast<uint8_t>(256 * intensity.clamp(g));
    dst[2] = static_cast<uint8_t>(256 * intensity.clamp(b));
}
