#pragma once

#include "perlin.h"
#include "rt_image.h"
#include "rtweekend.h"

class texture {
public:
    virtual ~texture() = default;
    virtual color value(double u, double v, const point3& p) const = 0;
};

class solid_color : public texture {
public:
    solid_color(const color& albedo) : albedo(albedo) {}
    solid_color(double r, double g, double b) : albedo(r, g, b) {}

    color value(double /*u*/, double /*v*/, const point3& /*p*/) const override {
        return albedo;
    }

private:
    color albedo;
};

class checker_texture : public texture {
public:
    checker_texture(double scale, shared_ptr<texture> even, shared_ptr<texture> odd)
        : inv_scale(1.0 / scale), even(even), odd(odd) {}

    checker_texture(double scale, const color& c1, const color& c2)
        : checker_texture(scale, make_shared<solid_color>(c1), make_shared<solid_color>(c2)) {}

    color value(double u, double v, const point3& p) const override {
        int xi = static_cast<int>(std::floor(inv_scale * p.x()));
        int yi = static_cast<int>(std::floor(inv_scale * p.y()));
        int zi = static_cast<int>(std::floor(inv_scale * p.z()));
        bool is_even = (xi + yi + zi) % 2 == 0;
        return is_even ? even->value(u, v, p) : odd->value(u, v, p);
    }

private:
    double inv_scale;
    shared_ptr<texture> even;
    shared_ptr<texture> odd;
};

class image_texture : public texture {
public:
    image_texture(const std::string& filename) : image(filename) {}

    color value(double u, double v, const point3& /*p*/) const override {
        if (image.height() <= 0) return color(0, 1, 1);
        u = interval(0, 1).clamp(u);
        v = 1.0 - interval(0, 1).clamp(v);

        int i = static_cast<int>(u * image.width());
        int j = static_cast<int>(v * image.height());
        const unsigned char* px = image.pixel_data(i, j);
        double scale = 1.0 / 255.0;
        return color(scale * px[0], scale * px[1], scale * px[2]);
    }

private:
    rt_image image;
};

class noise_texture : public texture {
public:
    noise_texture(double scale) : scale(scale) {}

    color value(double /*u*/, double /*v*/, const point3& p) const override {
        return color(0.5, 0.5, 0.5)
             * (1 + std::sin(scale * p.z() + 10 * noise.turb(p)));
    }

private:
    perlin noise;
    double scale;
};
