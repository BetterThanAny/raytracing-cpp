#pragma once

#include "hittable.h"
#include "rtweekend.h"

class triangle : public hittable {
public:
    triangle(const point3& v0, const point3& v1, const point3& v2,
             shared_ptr<material> mat)
        : v0(v0), v1(v1), v2(v2), mat(mat) {
        edge1 = v1 - v0;
        edge2 = v2 - v0;
        vec3 n = cross(edge1, edge2);
        area_n = n.length();
        normal = unit_vector(n);
        set_bounding_box();
    }

    void set_bounding_box() {
        point3 min(std::fmin(std::fmin(v0.x(), v1.x()), v2.x()),
                   std::fmin(std::fmin(v0.y(), v1.y()), v2.y()),
                   std::fmin(std::fmin(v0.z(), v1.z()), v2.z()));
        point3 max(std::fmax(std::fmax(v0.x(), v1.x()), v2.x()),
                   std::fmax(std::fmax(v0.y(), v1.y()), v2.y()),
                   std::fmax(std::fmax(v0.z(), v1.z()), v2.z()));
        bbox = aabb(min, max);
    }

    aabb bounding_box() const override { return bbox; }

    // Möller–Trumbore intersection
    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        vec3 h = cross(r.direction(), edge2);
        double a = dot(edge1, h);
        if (std::fabs(a) < 1e-8) return false;

        double f = 1.0 / a;
        vec3   s = r.origin() - v0;
        double u = f * dot(s, h);
        if (u < 0.0 || u > 1.0) return false;

        vec3   q = cross(s, edge1);
        double v = f * dot(r.direction(), q);
        if (v < 0.0 || u + v > 1.0) return false;

        double t = f * dot(edge2, q);
        if (!ray_t.contains(t)) return false;

        rec.t = t;
        rec.p = r.at(t);
        rec.u = u;
        rec.v = v;
        rec.set_face_normal(r, normal);
        rec.mat = mat;
        return true;
    }

private:
    point3 v0, v1, v2;
    vec3   edge1, edge2;
    vec3   normal;
    double area_n;
    shared_ptr<material> mat;
    aabb   bbox;
};
