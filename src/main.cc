#include "bvh.h"
#include "camera.h"
#include "constant_medium.h"
#include "hittable_list.h"
#include "material.h"
#include "obj_loader.h"
#include "png_writer.h"
#include "quad.h"
#include "rtweekend.h"
#include "sphere.h"
#include "texture.h"
#include "triangle.h"

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>

// -------- Book 1 scenes --------

static void scene_first_light(hittable_list& world, camera& cam) {
    auto ground = make_shared<lambertian>(color(0.8, 0.8, 0.0));
    auto center = make_shared<lambertian>(color(0.1, 0.2, 0.5));
    auto left   = make_shared<dielectric>(1.50);
    auto bubble = make_shared<dielectric>(1.00 / 1.50);
    auto right  = make_shared<metal>(color(0.8, 0.6, 0.2), 1.0);

    world.add(make_shared<sphere>(point3(0, -100.5, -1), 100.0, ground));
    world.add(make_shared<sphere>(point3(0, 0, -1.2),      0.5, center));
    world.add(make_shared<sphere>(point3(-1.0, 0, -1),     0.5, left));
    world.add(make_shared<sphere>(point3(-1.0, 0, -1),     0.4, bubble));
    world.add(make_shared<sphere>(point3(1.0, 0, -1),      0.5, right));

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    cam.vfov              = 20;
    cam.lookfrom          = point3(-2, 2, 1);
    cam.lookat            = point3(0, 0, -1);
    cam.vup               = vec3(0, 1, 0);
    cam.defocus_angle     = 10.0;
    cam.focus_dist        = 3.4;
}

static void scene_book1_final(hittable_list& world, camera& cam, int spp, int w) {
    auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));

    for (int a = -11; a < 11; ++a) {
        for (int b = -11; b < 11; ++b) {
            double choose_mat = random_double();
            point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());
            if ((center - point3(4, 0.2, 0)).length() <= 0.9) continue;

            shared_ptr<material> sphere_material;
            if (choose_mat < 0.8) {
                color albedo = vec3::random() * vec3::random();
                sphere_material = make_shared<lambertian>(albedo);
            } else if (choose_mat < 0.95) {
                color  albedo = vec3::random(0.5, 1);
                double fuzz   = random_double(0, 0.5);
                sphere_material = make_shared<metal>(albedo, fuzz);
            } else {
                sphere_material = make_shared<dielectric>(1.5);
            }
            world.add(make_shared<sphere>(center, 0.2, sphere_material));
        }
    }

    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, make_shared<dielectric>(1.5)));
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, make_shared<lambertian>(color(0.4, 0.2, 0.1))));
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, make_shared<metal>(color(0.7, 0.6, 0.5), 0.0)));

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = w;
    cam.samples_per_pixel = spp;
    cam.max_depth         = 50;
    cam.vfov              = 20;
    cam.lookfrom          = point3(13, 2, 3);
    cam.lookat            = point3(0, 0, 0);
    cam.vup               = vec3(0, 1, 0);
    cam.defocus_angle     = 0.6;
    cam.focus_dist        = 10.0;
}

// -------- Book 2 scenes --------

static void scene_bouncing_spheres(hittable_list& world, camera& cam, int spp, int w) {
    auto checker = make_shared<checker_texture>(0.32, color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));
    world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(checker)));

    for (int a = -11; a < 11; ++a) {
        for (int b = -11; b < 11; ++b) {
            double choose_mat = random_double();
            point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());
            if ((center - point3(4, 0.2, 0)).length() <= 0.9) continue;

            shared_ptr<material> sphere_material;
            if (choose_mat < 0.8) {
                color albedo = vec3::random() * vec3::random();
                sphere_material = make_shared<lambertian>(albedo);
                point3 center2 = center + vec3(0, random_double(0, 0.5), 0);
                world.add(make_shared<sphere>(center, center2, 0.2, sphere_material));
            } else if (choose_mat < 0.95) {
                color  albedo = vec3::random(0.5, 1);
                double fuzz   = random_double(0, 0.5);
                sphere_material = make_shared<metal>(albedo, fuzz);
                world.add(make_shared<sphere>(center, 0.2, sphere_material));
            } else {
                sphere_material = make_shared<dielectric>(1.5);
                world.add(make_shared<sphere>(center, 0.2, sphere_material));
            }
        }
    }

    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, make_shared<dielectric>(1.5)));
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, make_shared<lambertian>(color(0.4, 0.2, 0.1))));
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, make_shared<metal>(color(0.7, 0.6, 0.5), 0.0)));

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = w;
    cam.samples_per_pixel = spp;
    cam.max_depth         = 50;
    cam.vfov              = 20;
    cam.lookfrom          = point3(13, 2, 3);
    cam.lookat            = point3(0, 0, 0);
    cam.defocus_angle     = 0.6;
    cam.focus_dist        = 10.0;
}

static void scene_checkered_spheres(hittable_list& world, camera& cam) {
    auto checker = make_shared<checker_texture>(0.32, color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));
    world.add(make_shared<sphere>(point3(0, -10, 0), 10, make_shared<lambertian>(checker)));
    world.add(make_shared<sphere>(point3(0,  10, 0), 10, make_shared<lambertian>(checker)));

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    cam.vfov              = 20;
    cam.lookfrom          = point3(13, 2, 3);
    cam.lookat            = point3(0, 0, 0);
}

static void scene_earth(hittable_list& world, camera& cam) {
    auto earth_tex = make_shared<image_texture>("scenes/earthmap.jpg");
    auto earth_surface = make_shared<lambertian>(earth_tex);
    world.add(make_shared<sphere>(point3(0, 0, 0), 2, earth_surface));

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    cam.vfov              = 20;
    cam.lookfrom          = point3(0, 0, 12);
    cam.lookat            = point3(0, 0, 0);
}

static void scene_perlin_spheres(hittable_list& world, camera& cam) {
    auto pertext = make_shared<noise_texture>(4.0);
    world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
    world.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(pertext)));

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    cam.vfov              = 20;
    cam.lookfrom          = point3(13, 2, 3);
    cam.lookat            = point3(0, 0, 0);
}

static void scene_quads(hittable_list& world, camera& cam) {
    auto left_red    = make_shared<lambertian>(color(1.0, 0.2, 0.2));
    auto back_green  = make_shared<lambertian>(color(0.2, 1.0, 0.2));
    auto right_blue  = make_shared<lambertian>(color(0.2, 0.2, 1.0));
    auto upper_orange = make_shared<lambertian>(color(1.0, 0.5, 0.0));
    auto lower_teal  = make_shared<lambertian>(color(0.2, 0.8, 0.8));

    world.add(make_shared<quad>(point3(-3, -2,  5), vec3(0, 0, -4), vec3(0, 4, 0), left_red));
    world.add(make_shared<quad>(point3(-2, -2,  0), vec3(4, 0,  0), vec3(0, 4, 0), back_green));
    world.add(make_shared<quad>(point3( 3, -2,  1), vec3(0, 0,  4), vec3(0, 4, 0), right_blue));
    world.add(make_shared<quad>(point3(-2,  3,  1), vec3(4, 0,  0), vec3(0, 0, 4), upper_orange));
    world.add(make_shared<quad>(point3(-2, -3,  5), vec3(4, 0,  0), vec3(0, 0, -4), lower_teal));

    cam.aspect_ratio      = 1.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    cam.vfov              = 80;
    cam.lookfrom          = point3(0, 0, 9);
    cam.lookat            = point3(0, 0, 0);
}

static void scene_simple_light(hittable_list& world, camera& cam) {
    auto pertext = make_shared<noise_texture>(4);
    world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
    world.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(pertext)));

    auto difflight = make_shared<diffuse_light>(color(4, 4, 4));
    world.add(make_shared<sphere>(point3(0, 7, 0), 2, difflight));
    world.add(make_shared<quad>(point3(3, 1, -2), vec3(2, 0, 0), vec3(0, 2, 0), difflight));

    cam.background        = color(0, 0, 0);
    cam.use_sky_gradient  = false;
    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    cam.vfov              = 20;
    cam.lookfrom          = point3(26, 3, 6);
    cam.lookat            = point3(0, 2, 0);
}

static void scene_cornell_box(hittable_list& world, camera& cam, int spp, int w,
                              hittable_list* lights = nullptr) {
    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(15, 15, 15));

    world.add(make_shared<quad>(point3(555, 0, 0), vec3(0, 555, 0), vec3(0, 0, 555), green));
    world.add(make_shared<quad>(point3(0, 0, 0), vec3(0, 555, 0), vec3(0, 0, 555), red));
    auto light_quad = make_shared<quad>(point3(343, 554, 332),
                                        vec3(-130, 0, 0), vec3(0, 0, -105), light);
    world.add(light_quad);
    world.add(make_shared<quad>(point3(0, 0, 0), vec3(555, 0, 0), vec3(0, 0, 555), white));
    world.add(make_shared<quad>(point3(555, 555, 555), vec3(-555, 0, 0), vec3(0, 0, -555), white));
    world.add(make_shared<quad>(point3(0, 0, 555), vec3(555, 0, 0), vec3(0, 555, 0), white));

    shared_ptr<hittable> box1 = box(point3(0, 0, 0), point3(165, 330, 165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265, 0, 295));
    world.add(box1);

    shared_ptr<hittable> box2 = box(point3(0, 0, 0), point3(165, 165, 165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(130, 0, 65));
    world.add(box2);

    if (lights) {
        // Flat shape sampler — use an unrotated light quad for sampling.
        lights->add(make_shared<quad>(point3(343, 554, 332),
                                      vec3(-130, 0, 0), vec3(0, 0, -105),
                                      shared_ptr<material>()));
    }

    cam.background        = color(0, 0, 0);
    cam.use_sky_gradient  = false;
    cam.aspect_ratio      = 1.0;
    cam.image_width       = w;
    cam.samples_per_pixel = spp;
    cam.max_depth         = 50;
    cam.vfov              = 40;
    cam.lookfrom          = point3(278, 278, -800);
    cam.lookat            = point3(278, 278, 0);
}

static void scene_cornell_smoke(hittable_list& world, camera& cam, int spp, int w) {
    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(7, 7, 7));

    world.add(make_shared<quad>(point3(555, 0, 0), vec3(0, 555, 0), vec3(0, 0, 555), green));
    world.add(make_shared<quad>(point3(0, 0, 0), vec3(0, 555, 0), vec3(0, 0, 555), red));
    world.add(make_shared<quad>(point3(113, 554, 127), vec3(330, 0, 0), vec3(0, 0, 305), light));
    world.add(make_shared<quad>(point3(0, 555, 0), vec3(555, 0, 0), vec3(0, 0, 555), white));
    world.add(make_shared<quad>(point3(0, 0, 0), vec3(555, 0, 0), vec3(0, 0, 555), white));
    world.add(make_shared<quad>(point3(0, 0, 555), vec3(555, 0, 0), vec3(0, 555, 0), white));

    shared_ptr<hittable> box1 = box(point3(0, 0, 0), point3(165, 330, 165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265, 0, 295));

    shared_ptr<hittable> box2 = box(point3(0, 0, 0), point3(165, 165, 165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(130, 0, 65));

    world.add(make_shared<constant_medium>(box1, 0.01, color(0, 0, 0)));
    world.add(make_shared<constant_medium>(box2, 0.01, color(1, 1, 1)));

    cam.background        = color(0, 0, 0);
    cam.use_sky_gradient  = false;
    cam.aspect_ratio      = 1.0;
    cam.image_width       = w;
    cam.samples_per_pixel = spp;
    cam.max_depth         = 50;
    cam.vfov              = 40;
    cam.lookfrom          = point3(278, 278, -800);
    cam.lookat            = point3(278, 278, 0);
}

static void scene_book2_final(hittable_list& world, camera& cam, int spp, int w) {
    hittable_list boxes1;
    auto ground = make_shared<lambertian>(color(0.48, 0.83, 0.53));
    int boxes_per_side = 20;
    for (int i = 0; i < boxes_per_side; ++i) {
        for (int j = 0; j < boxes_per_side; ++j) {
            double w_b = 100.0;
            double x0 = -1000.0 + i * w_b;
            double z0 = -1000.0 + j * w_b;
            double y0 = 0.0;
            double x1 = x0 + w_b;
            double y1 = random_double(1, 101);
            double z1 = z0 + w_b;
            boxes1.add(box(point3(x0, y0, z0), point3(x1, y1, z1), ground));
        }
    }
    world.add(make_shared<bvh_node>(boxes1));

    auto light = make_shared<diffuse_light>(color(7, 7, 7));
    world.add(make_shared<quad>(point3(123, 554, 147), vec3(300, 0, 0), vec3(0, 0, 265), light));

    point3 center1(400, 400, 200);
    point3 center2 = center1 + vec3(30, 0, 0);
    auto moving_mat = make_shared<lambertian>(color(0.7, 0.3, 0.1));
    world.add(make_shared<sphere>(center1, center2, 50, moving_mat));

    world.add(make_shared<sphere>(point3(260, 150, 45), 50, make_shared<dielectric>(1.5)));
    world.add(make_shared<sphere>(point3(0, 150, 145), 50, make_shared<metal>(color(0.8, 0.8, 0.9), 1.0)));

    auto boundary = make_shared<sphere>(point3(360, 150, 145), 70, make_shared<dielectric>(1.5));
    world.add(boundary);
    world.add(make_shared<constant_medium>(boundary, 0.2, color(0.2, 0.4, 0.9)));

    auto global_fog = make_shared<sphere>(point3(0, 0, 0), 5000, make_shared<dielectric>(1.5));
    world.add(make_shared<constant_medium>(global_fog, 0.0001, color(1, 1, 1)));

    auto emat = make_shared<lambertian>(make_shared<image_texture>("scenes/earthmap.jpg"));
    world.add(make_shared<sphere>(point3(400, 200, 400), 100, emat));

    auto pertext = make_shared<noise_texture>(0.2);
    world.add(make_shared<sphere>(point3(220, 280, 300), 80, make_shared<lambertian>(pertext)));

    hittable_list boxes2;
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    int ns = 1000;
    for (int j = 0; j < ns; ++j)
        boxes2.add(make_shared<sphere>(point3::random(0, 165), 10, white));
    world.add(make_shared<translate>(
        make_shared<rotate_y>(make_shared<bvh_node>(boxes2), 15),
        vec3(-100, 270, 395)));

    cam.background        = color(0, 0, 0);
    cam.use_sky_gradient  = false;
    cam.aspect_ratio      = 1.0;
    cam.image_width       = w;
    cam.samples_per_pixel = spp;
    cam.max_depth         = 40;
    cam.vfov              = 40;
    cam.lookfrom          = point3(478, 278, -600);
    cam.lookat            = point3(278, 278, 0);
}

// -------- Triangle / OBJ scenes --------

// Build a unit icosahedron at origin using golden ratio.
static void add_icosahedron(hittable_list& world, shared_ptr<material> mat,
                            double scale = 1.0, const point3& origin = point3(0, 0, 0)) {
    const double phi = (1.0 + std::sqrt(5.0)) / 2.0;
    const double a = scale;
    const double b = scale * phi;

    std::vector<point3> v = {
        {-a,  b,  0}, { a,  b,  0}, {-a, -b,  0}, { a, -b,  0},
        { 0, -a,  b}, { 0,  a,  b}, { 0, -a, -b}, { 0,  a, -b},
        { b,  0, -a}, { b,  0,  a}, {-b,  0, -a}, {-b,  0,  a}
    };
    for (auto& p : v) p = p + origin;

    const int faces[20][3] = {
        {0, 11, 5}, {0, 5, 1}, {0, 1, 7}, {0, 7, 10}, {0, 10, 11},
        {1, 5, 9}, {5, 11, 4}, {11, 10, 2}, {10, 7, 6}, {7, 1, 8},
        {3, 9, 4}, {3, 4, 2}, {3, 2, 6}, {3, 6, 8}, {3, 8, 9},
        {4, 9, 5}, {2, 4, 11}, {6, 2, 10}, {8, 6, 7}, {9, 8, 1}
    };

    for (auto& f : faces)
        world.add(make_shared<triangle>(v[f[0]], v[f[1]], v[f[2]], mat));
}

static void scene_icosahedron(hittable_list& world, camera& cam) {
    auto floor_mat = make_shared<lambertian>(
        make_shared<checker_texture>(0.5, color(0.8, 0.8, 0.8), color(0.2, 0.3, 0.1)));
    world.add(make_shared<quad>(point3(-20, 0, -20), vec3(40, 0, 0), vec3(0, 0, 40), floor_mat));

    auto gold = make_shared<metal>(color(0.9, 0.75, 0.35), 0.05);
    add_icosahedron(world, gold, 1.5, point3(0, 2.5, 0));

    auto glass = make_shared<dielectric>(1.5);
    add_icosahedron(world, glass, 1.2, point3(-4, 2, 3));

    auto diffuse = make_shared<lambertian>(color(0.2, 0.3, 0.9));
    add_icosahedron(world, diffuse, 1.2, point3(4, 2, 3));

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 800;
    cam.samples_per_pixel = 200;
    cam.max_depth         = 50;
    cam.vfov              = 30;
    cam.lookfrom          = point3(0, 6, 14);
    cam.lookat            = point3(0, 2, 0);
}

static void scene_bunny(hittable_list& world, camera& cam, int spp, int w) {
    auto floor_mat = make_shared<lambertian>(
        make_shared<checker_texture>(0.5, color(0.8, 0.8, 0.8), color(0.2, 0.3, 0.1)));
    world.add(make_shared<quad>(point3(-10, 0, -10), vec3(20, 0, 0), vec3(0, 0, 20), floor_mat));

    auto bunny_mat = make_shared<lambertian>(color(0.8, 0.4, 0.4));
    auto bunny_mesh = load_obj("scenes/bunny.obj", bunny_mat, /*scale=*/30.0, /*translate=*/point3(0, -1.0, 0));
    world.add(make_shared<bvh_node>(*bunny_mesh));

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = w;
    cam.samples_per_pixel = spp;
    cam.max_depth         = 30;
    cam.vfov              = 25;
    cam.lookfrom          = point3(0, 3, 10);
    cam.lookat            = point3(0, 3, 0);
}

static void scene_bunny_cornell(hittable_list& world, camera& cam, int spp, int w,
                                hittable_list* lights = nullptr) {
    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(15, 15, 15));

    world.add(make_shared<quad>(point3(555, 0, 0), vec3(0, 555, 0), vec3(0, 0, 555), green));
    world.add(make_shared<quad>(point3(0, 0, 0), vec3(0, 555, 0), vec3(0, 0, 555), red));
    world.add(make_shared<quad>(point3(343, 554, 332), vec3(-130, 0, 0), vec3(0, 0, -105), light));
    world.add(make_shared<quad>(point3(0, 0, 0), vec3(555, 0, 0), vec3(0, 0, 555), white));
    world.add(make_shared<quad>(point3(555, 555, 555), vec3(-555, 0, 0), vec3(0, 0, -555), white));
    world.add(make_shared<quad>(point3(0, 0, 555), vec3(555, 0, 0), vec3(0, 555, 0), white));

    auto bunny_mat = make_shared<metal>(color(0.9, 0.8, 0.7), 0.02);
    auto bunny_mesh = load_obj("scenes/bunny.obj", bunny_mat,
                               /*scale=*/2000.0,
                               /*translate=*/point3(278, -60, 278));
    world.add(make_shared<bvh_node>(*bunny_mesh));

    if (lights) {
        lights->add(make_shared<quad>(point3(343, 554, 332),
                                      vec3(-130, 0, 0), vec3(0, 0, -105),
                                      shared_ptr<material>()));
    }

    cam.background        = color(0, 0, 0);
    cam.use_sky_gradient  = false;
    cam.aspect_ratio      = 1.0;
    cam.image_width       = w;
    cam.samples_per_pixel = spp;
    cam.max_depth         = 50;
    cam.vfov              = 40;
    cam.lookfrom          = point3(278, 278, -800);
    cam.lookat            = point3(278, 278, 0);
}

// -------- main --------

int main(int argc, char** argv) {
    std::string scene = argc > 1 ? argv[1] : "first_light";
    std::string out_base = argc > 2 ? argv[2] : std::string("output/") + scene;

    hittable_list world;
    hittable_list lights;
    camera cam;

    try {
        if      (scene == "first_light")          scene_first_light(world, cam);
        else if (scene == "book1_final_preview")  scene_book1_final(world, cam, 50, 400);
        else if (scene == "book1_final")          scene_book1_final(world, cam, 500, 1200);
        else if (scene == "bouncing_spheres_preview") scene_bouncing_spheres(world, cam, 50, 400);
        else if (scene == "bouncing_spheres")     scene_bouncing_spheres(world, cam, 500, 1200);
        else if (scene == "checkered_spheres")    scene_checkered_spheres(world, cam);
        else if (scene == "earth")                scene_earth(world, cam);
        else if (scene == "perlin_spheres")       scene_perlin_spheres(world, cam);
        else if (scene == "quads")                scene_quads(world, cam);
        else if (scene == "simple_light")         scene_simple_light(world, cam);
        else if (scene == "cornell_box_preview")  scene_cornell_box(world, cam, 100, 300);
        else if (scene == "cornell_box")          scene_cornell_box(world, cam, 1000, 600);
        else if (scene == "cornell_box_mcmc_preview") scene_cornell_box(world, cam, 100, 300, &lights);
        else if (scene == "cornell_box_mcmc")     scene_cornell_box(world, cam, 1000, 600, &lights);
        else if (scene == "cornell_smoke_preview") scene_cornell_smoke(world, cam, 100, 300);
        else if (scene == "cornell_smoke")        scene_cornell_smoke(world, cam, 1000, 600);
        else if (scene == "book2_final_preview")  scene_book2_final(world, cam, 100, 400);
        else if (scene == "book2_final")          scene_book2_final(world, cam, 10000, 800);
        else if (scene == "icosahedron")          scene_icosahedron(world, cam);
        else if (scene == "bunny_preview")        scene_bunny(world, cam, 50, 400);
        else if (scene == "bunny")                scene_bunny(world, cam, 300, 1000);
        else if (scene == "bunny_cornell_preview") scene_bunny_cornell(world, cam, 100, 300, &lights);
        else if (scene == "bunny_cornell")        scene_bunny_cornell(world, cam, 500, 600, &lights);
        else {
            std::cerr << "Unknown scene: " << scene << "\n";
            std::cerr << "Available scenes:\n"
                      << "  Book 1: first_light, book1_final_preview, book1_final\n"
                      << "  Book 2: bouncing_spheres(_preview), checkered_spheres, earth,\n"
                      << "          perlin_spheres, quads, simple_light,\n"
                      << "          cornell_box(_preview), cornell_smoke(_preview),\n"
                      << "          book2_final(_preview)\n"
                      << "  Book 3: cornell_box_mcmc(_preview)\n"
                      << "  Meshes: icosahedron, bunny(_preview), bunny_cornell(_preview)\n";
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to build scene '" << scene << "': " << e.what() << "\n";
        return 1;
    }

    // BVH wrap for acceleration
    hittable_list accelerated;
    try {
        accelerated.add(make_shared<bvh_node>(world));
    } catch (const std::exception& e) {
        std::cerr << "Failed to build BVH for scene '" << scene << "': " << e.what() << "\n";
        return 1;
    }

    auto t0 = std::chrono::steady_clock::now();
    std::vector<color> framebuffer;
    cam.render_to_buffer(accelerated, lights, framebuffer);
    auto t1 = std::chrono::steady_clock::now();
    double secs = std::chrono::duration<double>(t1 - t0).count();
    std::clog << "Render time: " << secs << " s\n";

    std::string png_path = out_base + ".png";
    std::string ppm_path = out_base + ".ppm";
    if (!write_png(png_path, cam.get_image_width(), cam.get_image_height(), framebuffer)) {
        std::cerr << "Failed to write " << png_path << "\n";
        return 1;
    }
    if (!write_ppm(ppm_path, cam.get_image_width(), cam.get_image_height(), framebuffer)) {
        std::cerr << "Failed to write " << ppm_path << "\n";
        return 1;
    }
    std::clog << "Wrote " << png_path << " and " << ppm_path << "\n";
    return 0;
}
