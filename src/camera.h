#pragma once

#include "color.h"
#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "pdf.h"
#include "rtweekend.h"

#include <atomic>
#include <thread>
#include <vector>

class camera {
public:
    double aspect_ratio      = 1.0;
    int    image_width       = 100;
    int    samples_per_pixel = 10;
    int    max_depth         = 10;
    color  background        = color(0.70, 0.80, 1.00);
    bool   use_sky_gradient  = true;

    double vfov       = 90;
    point3 lookfrom   = point3(0, 0, 0);
    point3 lookat     = point3(0, 0, -1);
    vec3   vup        = vec3(0, 1, 0);

    double defocus_angle = 0;
    double focus_dist    = 10;

    int num_threads = 0;

    void render(const hittable& world, std::ostream& out) {
        std::vector<color> framebuffer;
        hittable_list empty;
        render_to_buffer(world, empty, framebuffer);
        out << "P3\n" << image_width << ' ' << image_height << "\n255\n";
        for (int j = 0; j < image_height; ++j) {
            for (int i = 0; i < image_width; ++i) {
                write_color(out, framebuffer[j * image_width + i]);
            }
        }
    }

    void render_to_buffer(const hittable& world, std::vector<color>& framebuffer) {
        hittable_list empty;
        render_to_buffer(world, empty, framebuffer);
    }

    void render_to_buffer(const hittable& world, const hittable& lights,
                          std::vector<color>& framebuffer) {
        initialize();
        framebuffer.assign(image_width * image_height, color(0, 0, 0));

        int threads = num_threads > 0 ? num_threads : std::thread::hardware_concurrency();
        if (threads <= 0) threads = 1;

        std::atomic<int> next_row{0};
        std::atomic<int> done_rows{0};

        auto worker = [&]() {
            while (true) {
                int j = next_row.fetch_add(1);
                if (j >= image_height) break;
                for (int i = 0; i < image_width; ++i) {
                    color pixel(0, 0, 0);
                    for (int s = 0; s < samples_per_pixel; ++s) {
                        ray r = get_ray(i, j);
                        pixel += ray_color(r, max_depth, world, lights);
                    }
                    framebuffer[j * image_width + i] = pixel_samples_scale * pixel;
                }
                int done = done_rows.fetch_add(1) + 1;
                std::clog << "\rRendering: " << done << "/" << image_height
                          << " rows (" << (100 * done / image_height) << "%)   " << std::flush;
            }
        };

        std::vector<std::thread> pool;
        pool.reserve(threads);
        for (int t = 0; t < threads; ++t) pool.emplace_back(worker);
        for (auto& th : pool) th.join();
        std::clog << "\n";
    }

    int get_image_width() const  { return image_width; }
    int get_image_height() const { return image_height; }

private:
    int    image_height = 0;
    double pixel_samples_scale = 0;
    point3 center;
    point3 pixel00_loc;
    vec3   pixel_delta_u;
    vec3   pixel_delta_v;
    vec3   u_basis, v_basis, w_basis;
    vec3   defocus_disk_u;
    vec3   defocus_disk_v;

    void initialize() {
        image_height = static_cast<int>(image_width / aspect_ratio);
        if (image_height < 1) image_height = 1;

        pixel_samples_scale = 1.0 / samples_per_pixel;

        center = lookfrom;

        double theta = degrees_to_radians(vfov);
        double h     = std::tan(theta / 2);
        double viewport_height = 2 * h * focus_dist;
        double viewport_width  = viewport_height * (static_cast<double>(image_width) / image_height);

        w_basis = unit_vector(lookfrom - lookat);
        u_basis = unit_vector(cross(vup, w_basis));
        v_basis = cross(w_basis, u_basis);

        vec3 viewport_u = viewport_width * u_basis;
        vec3 viewport_v = viewport_height * -v_basis;

        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        point3 viewport_upper_left = center - focus_dist * w_basis
                                   - viewport_u / 2 - viewport_v / 2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        double defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = u_basis * defocus_radius;
        defocus_disk_v = v_basis * defocus_radius;
    }

    ray get_ray(int i, int j) const {
        vec3 offset     = sample_square();
        point3 pixel_sample = pixel00_loc
                            + (i + offset.x()) * pixel_delta_u
                            + (j + offset.y()) * pixel_delta_v;

        point3 ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
        vec3   ray_dir    = pixel_sample - ray_origin;
        double ray_time   = random_double();
        return ray(ray_origin, ray_dir, ray_time);
    }

    vec3 sample_square() const {
        return vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }

    point3 defocus_disk_sample() const {
        vec3 p = random_in_unit_disk();
        return center + p.x() * defocus_disk_u + p.y() * defocus_disk_v;
    }

    color sky_color(const ray& r) const {
        vec3   unit_dir = unit_vector(r.direction());
        double a = 0.5 * (unit_dir.y() + 1.0);
        return (1.0 - a) * color(1.0, 1.0, 1.0) + a * background;
    }

    bool has_lights(const hittable& lights) const {
        const hittable_list* list = dynamic_cast<const hittable_list*>(&lights);
        return list && !list->objects.empty();
    }

    color ray_color(const ray& r, int depth, const hittable& world,
                    const hittable& lights) const {
        if (depth <= 0) return color(0, 0, 0);

        hit_record rec;
        if (!world.hit(r, interval(0.001, infinity), rec)) {
            return use_sky_gradient ? sky_color(r) : background;
        }

        scatter_record srec;
        color emission = rec.mat->emitted(r, rec, rec.u, rec.v, rec.p);

        if (!rec.mat->scatter(r, rec, srec)) return emission;

        if (srec.skip_pdf) {
            return srec.attenuation * ray_color(srec.skip_pdf_ray, depth - 1, world, lights);
        }

        // Pick PDF: mixture with lights if we have them, otherwise surface PDF alone.
        shared_ptr<pdf> combined_pdf;
        if (has_lights(lights)) {
            auto light_pdf = make_shared<hittable_pdf>(lights, rec.p);
            combined_pdf = make_shared<mixture_pdf>(light_pdf, srec.pdf_ptr);
        } else {
            combined_pdf = srec.pdf_ptr;
        }

        ray    scattered(rec.p, combined_pdf->generate(), r.time());
        double pdf_val = combined_pdf->value(scattered.direction());
        if (pdf_val <= 0) return emission;

        double scattering_pdf_val = rec.mat->scattering_pdf(r, rec, scattered);
        color  sample_color       = ray_color(scattered, depth - 1, world, lights);

        return emission + (srec.attenuation * scattering_pdf_val * sample_color) / pdf_val;
    }
};
