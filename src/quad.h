#pragma once

#include "hittable.h"
#include "hittable_list.h"
#include "rtweekend.h"

class quad : public hittable {
public:
    quad(const point3& Q, const vec3& u, const vec3& v, shared_ptr<material> mat)
        : Q(Q), u(u), v(v), mat(mat) {
        vec3 n = cross(u, v);
        normal = unit_vector(n);
        D = dot(normal, Q);
        w = n / dot(n, n);
        area = n.length();
        set_bounding_box();
    }

    void set_bounding_box() {
        aabb box1(Q, Q + u + v);
        aabb box2(Q + u, Q + v);
        bbox = aabb(box1, box2);
    }

    aabb bounding_box() const override { return bbox; }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        double denom = dot(normal, r.direction());
        if (std::fabs(denom) < 1e-8) return false;

        double t = (D - dot(normal, r.origin())) / denom;
        if (!ray_t.contains(t)) return false;

        point3 intersection = r.at(t);
        vec3   planar_hitpt = intersection - Q;
        double alpha = dot(w, cross(planar_hitpt, v));
        double beta  = dot(w, cross(u, planar_hitpt));

        if (!is_interior(alpha, beta, rec)) return false;

        rec.t = t;
        rec.p = intersection;
        rec.mat = mat;
        rec.set_face_normal(r, normal);
        return true;
    }

    virtual bool is_interior(double a, double b, hit_record& rec) const {
        interval unit(0, 1);
        if (!unit.contains(a) || !unit.contains(b)) return false;
        rec.u = a;
        rec.v = b;
        return true;
    }

    double pdf_value(const point3& origin, const vec3& direction) const override {
        hit_record rec;
        if (!this->hit(ray(origin, direction), interval(0.001, infinity), rec)) return 0.0;

        double dist_squared = rec.t * rec.t * direction.length_squared();
        double cosine = std::fabs(dot(direction, rec.normal) / direction.length());
        return dist_squared / (cosine * area);
    }

    vec3 random(const point3& origin) const override {
        point3 p = Q + random_double() * u + random_double() * v;
        return p - origin;
    }

private:
    point3 Q;
    vec3   u, v;
    vec3   w;
    shared_ptr<material> mat;
    aabb   bbox;
    vec3   normal;
    double D;
    double area;
};

inline shared_ptr<hittable_list> box(const point3& a, const point3& b, shared_ptr<material> mat) {
    auto sides = make_shared<hittable_list>();

    point3 min(std::fmin(a.x(), b.x()), std::fmin(a.y(), b.y()), std::fmin(a.z(), b.z()));
    point3 max(std::fmax(a.x(), b.x()), std::fmax(a.y(), b.y()), std::fmax(a.z(), b.z()));

    vec3 dx(max.x() - min.x(), 0, 0);
    vec3 dy(0, max.y() - min.y(), 0);
    vec3 dz(0, 0, max.z() - min.z());

    sides->add(make_shared<quad>(point3(min.x(), min.y(), max.z()),  dx,  dy, mat));
    sides->add(make_shared<quad>(point3(max.x(), min.y(), max.z()), -dz,  dy, mat));
    sides->add(make_shared<quad>(point3(max.x(), min.y(), min.z()), -dx,  dy, mat));
    sides->add(make_shared<quad>(point3(min.x(), min.y(), min.z()),  dz,  dy, mat));
    sides->add(make_shared<quad>(point3(min.x(), max.y(), max.z()),  dx, -dz, mat));
    sides->add(make_shared<quad>(point3(min.x(), min.y(), min.z()),  dx,  dz, mat));

    return sides;
}
