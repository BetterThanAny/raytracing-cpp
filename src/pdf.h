#pragma once

#include "hittable.h"
#include "onb.h"
#include "rtweekend.h"
#include "vec3.h"

#include <memory>

inline vec3 random_cosine_direction() {
    double r1 = random_double();
    double r2 = random_double();

    double phi = 2 * pi * r1;
    double x = std::cos(phi) * std::sqrt(r2);
    double y = std::sin(phi) * std::sqrt(r2);
    double z = std::sqrt(1 - r2);
    return vec3(x, y, z);
}

class pdf {
public:
    virtual ~pdf() = default;
    virtual double value(const vec3& direction) const = 0;
    virtual vec3   generate() const = 0;
};

class sphere_pdf : public pdf {
public:
    double value(const vec3& /*direction*/) const override { return 1.0 / (4.0 * pi); }
    vec3   generate() const override { return random_unit_vector(); }
};

class cosine_pdf : public pdf {
public:
    cosine_pdf(const vec3& w) : uvw(w) {}

    double value(const vec3& direction) const override {
        double cosine_theta = dot(unit_vector(direction), uvw.w());
        return std::fmax(0.0, cosine_theta / pi);
    }

    vec3 generate() const override { return uvw.transform(random_cosine_direction()); }

private:
    onb uvw;
};

class hittable_pdf : public pdf {
public:
    hittable_pdf(const hittable& objects, const point3& origin)
        : objects(objects), origin(origin) {}

    double value(const vec3& direction) const override {
        return objects.pdf_value(origin, direction);
    }

    vec3 generate() const override { return objects.random(origin); }

private:
    const hittable& objects;
    point3          origin;
};

class mixture_pdf : public pdf {
public:
    mixture_pdf(shared_ptr<pdf> p0, shared_ptr<pdf> p1) : p{p0, p1} {}

    double value(const vec3& direction) const override {
        return 0.5 * p[0]->value(direction) + 0.5 * p[1]->value(direction);
    }

    vec3 generate() const override {
        return random_double() < 0.5 ? p[0]->generate() : p[1]->generate();
    }

private:
    shared_ptr<pdf> p[2];
};
