#pragma once

#include "interval.h"
#include "ray.h"

class aabb {
public:
    interval x, y, z;

    aabb() = default;

    aabb(const interval& x, const interval& y, const interval& z) : x(x), y(y), z(z) {
        pad_to_minimums();
    }

    aabb(const point3& a, const point3& b) {
        x = a.x() <= b.x() ? interval(a.x(), b.x()) : interval(b.x(), a.x());
        y = a.y() <= b.y() ? interval(a.y(), b.y()) : interval(b.y(), a.y());
        z = a.z() <= b.z() ? interval(a.z(), b.z()) : interval(b.z(), a.z());
        pad_to_minimums();
    }

    aabb(const aabb& box0, const aabb& box1)
        : x(box0.x, box1.x), y(box0.y, box1.y), z(box0.z, box1.z) {}

    const interval& axis_interval(int n) const {
        if (n == 1) return y;
        if (n == 2) return z;
        return x;
    }

    bool hit(const ray& r, interval ray_t) const {
        const point3& orig = r.origin();
        const vec3&   dir  = r.direction();

        for (int axis = 0; axis < 3; ++axis) {
            const interval& ax = axis_interval(axis);
            double adinv = 1.0 / dir[axis];

            double t0 = (ax.min - orig[axis]) * adinv;
            double t1 = (ax.max - orig[axis]) * adinv;

            if (t0 < t1) {
                if (t0 > ray_t.min) ray_t.min = t0;
                if (t1 < ray_t.max) ray_t.max = t1;
            } else {
                if (t1 > ray_t.min) ray_t.min = t1;
                if (t0 < ray_t.max) ray_t.max = t0;
            }

            if (ray_t.max <= ray_t.min) return false;
        }
        return true;
    }

    int longest_axis() const {
        if (x.size() > y.size()) return x.size() > z.size() ? 0 : 2;
        return y.size() > z.size() ? 1 : 2;
    }

    static const aabb empty;
    static const aabb universe;

private:
    void pad_to_minimums() {
        double delta = 0.0001;
        if (x.size() < delta) x = x.expand(delta);
        if (y.size() < delta) y = y.expand(delta);
        if (z.size() < delta) z = z.expand(delta);
    }
};

inline const aabb aabb::empty    = aabb(interval::empty, interval::empty, interval::empty);
inline const aabb aabb::universe = aabb(interval::universe, interval::universe, interval::universe);

inline aabb operator+(const aabb& bbox, const vec3& offset) {
    return aabb(bbox.x + offset.x(), bbox.y + offset.y(), bbox.z + offset.z());
}

inline aabb operator+(const vec3& offset, const aabb& bbox) { return bbox + offset; }
