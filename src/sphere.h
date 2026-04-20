#pragma once

#include "hittable.h"
#include "onb.h"
#include "rtweekend.h"

class sphere : public hittable {
public:
    sphere(const point3& static_center, double radius, shared_ptr<material> mat)
        : center1(static_center), radius(std::fmax(0.0, radius)),
          mat(mat), is_moving(false) {
        vec3 rvec(radius, radius, radius);
        bbox = aabb(static_center - rvec, static_center + rvec);
    }

    sphere(const point3& c1, const point3& c2, double radius, shared_ptr<material> mat)
        : center1(c1), radius(std::fmax(0.0, radius)), mat(mat),
          is_moving(true), center_vec(c2 - c1) {
        vec3 rvec(radius, radius, radius);
        aabb box1(c1 - rvec, c1 + rvec);
        aabb box2(c2 - rvec, c2 + rvec);
        bbox = aabb(box1, box2);
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        point3 center = is_moving ? sphere_center(r.time()) : center1;
        vec3   oc     = center - r.origin();
        double a      = r.direction().length_squared();
        double h      = dot(r.direction(), oc);
        double c      = oc.length_squared() - radius * radius;
        double discr  = h * h - a * c;
        if (discr < 0) return false;

        double sqrtd = std::sqrt(discr);
        double root  = (h - sqrtd) / a;
        if (!ray_t.surrounds(root)) {
            root = (h + sqrtd) / a;
            if (!ray_t.surrounds(root)) return false;
        }

        rec.t = root;
        rec.p = r.at(rec.t);
        vec3 outward_normal = (rec.p - center) / radius;
        rec.set_face_normal(r, outward_normal);
        get_sphere_uv(outward_normal, rec.u, rec.v);
        rec.mat = mat;
        return true;
    }

    aabb bounding_box() const override { return bbox; }

    double pdf_value(const point3& origin, const vec3& direction) const override {
        if (is_moving) return 0.0;
        double dist_squared = (center1 - origin).length_squared();
        if (dist_squared <= radius * radius) return 0.0; // origin inside sphere
        hit_record rec;
        if (!this->hit(ray(origin, direction), interval(0.001, infinity), rec)) return 0.0;

        double cos_theta_max = std::sqrt(1 - radius * radius / dist_squared);
        double solid_angle = 2 * pi * (1 - cos_theta_max);
        return 1 / solid_angle;
    }

    vec3 random(const point3& origin) const override {
        vec3   direction     = center1 - origin;
        double dist_squared  = direction.length_squared();
        if (dist_squared <= radius * radius) return random_unit_vector();
        onb    uvw(direction);
        return uvw.transform(random_to_sphere(radius, dist_squared));
    }

private:
    point3 center1;
    double radius;
    shared_ptr<material> mat;
    bool   is_moving;
    vec3   center_vec;
    aabb   bbox;

    point3 sphere_center(double time) const { return center1 + time * center_vec; }

    static void get_sphere_uv(const point3& p, double& u, double& v) {
        double theta = std::acos(-p.y());
        double phi   = std::atan2(-p.z(), p.x()) + pi;
        u = phi / (2 * pi);
        v = theta / pi;
    }

    static vec3 random_to_sphere(double radius, double distance_squared) {
        double r1 = random_double();
        double r2 = random_double();
        double z  = 1 + r2 * (std::sqrt(1 - radius * radius / distance_squared) - 1);

        double phi = 2 * pi * r1;
        double x = std::cos(phi) * std::sqrt(1 - z * z);
        double y = std::sin(phi) * std::sqrt(1 - z * z);
        return vec3(x, y, z);
    }
};
