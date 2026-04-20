#pragma once

#include "aabb.h"
#include "hittable.h"
#include "rtweekend.h"

#include <vector>

class hittable_list : public hittable {
public:
    std::vector<shared_ptr<hittable>> objects;

    hittable_list() = default;
    hittable_list(shared_ptr<hittable> object) { add(object); }

    void clear() { objects.clear(); bbox = aabb(); }

    void add(shared_ptr<hittable> object) {
        objects.push_back(object);
        bbox = aabb(bbox, object->bounding_box());
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        hit_record temp;
        bool   hit_anything = false;
        double closest_so_far = ray_t.max;

        for (const auto& object : objects) {
            if (object->hit(r, interval(ray_t.min, closest_so_far), temp)) {
                hit_anything   = true;
                closest_so_far = temp.t;
                rec            = temp;
            }
        }

        return hit_anything;
    }

    aabb bounding_box() const override { return bbox; }

    double pdf_value(const point3& origin, const vec3& direction) const override {
        if (objects.empty()) return 0.0;
        double weight = 1.0 / objects.size();
        double sum = 0.0;
        for (const auto& object : objects) sum += weight * object->pdf_value(origin, direction);
        return sum;
    }

    vec3 random(const point3& origin) const override {
        if (objects.empty()) return vec3(1, 0, 0);
        int idx = static_cast<int>(random_double() * objects.size());
        if (idx >= static_cast<int>(objects.size())) idx = static_cast<int>(objects.size()) - 1;
        return objects[idx]->random(origin);
    }

private:
    aabb bbox;
};
