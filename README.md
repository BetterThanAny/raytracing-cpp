# raytracing-cpp

A from-scratch offline path tracer implementing the full *Ray Tracing in One Weekend* series in modern C++17.

Follows the three books by Peter Shirley et al:
- Book 1 — *Ray Tracing in One Weekend*: Lambertian / Metal / Dielectric, defocus blur
- Book 2 — *The Next Week*: motion blur, BVH, checker / image / Perlin textures, quads, lights, volumes, Cornell box
- Book 3 — *The Rest of Your Life*: Monte Carlo integration, PDFs (cosine / sphere / hittable / mixture), ONB, importance sampling
- Bonus — Möller–Trumbore triangle + minimal OBJ loader (Stanford Bunny, 69k tris, BVH-accelerated)

Multithreaded row-parallel renderer (`std::thread` + atomic work queue). Outputs PPM and PNG (via `stb_image_write`).

## Build

```bash
cmake -S . -B build
cmake --build build -j
```

Requires C++17, CMake ≥ 3.16, and a POSIX threads implementation.

## Run

```bash
./build/raytracer <scene>
```

Available scenes:

| Scene | Output |
|-------|--------|
| `first_light` | `first_light.png` — Book 1 material showcase |
| `book1_final_preview` / `book1_final` | Random sphere scene |
| `checkered_spheres`, `earth`, `perlin_spheres`, `quads`, `simple_light` | Book 2 feature demos |
| `cornell_box(_preview)` | Brute-force path-traced Cornell box |
| `cornell_box_mcmc(_preview)` | Same scene with importance sampling |
| `cornell_smoke(_preview)` | Cornell with volumetric smoke boxes |
| `book2_final(_preview)` | Book 2 all-in-one final scene |
| `icosahedron` | Triangle-mesh material showcase |
| `bunny(_preview)` | Stanford Bunny via OBJ loader |

Rendered images land in `output/<scene>.{png,ppm}`.

## Sample outputs

Selected images (`output/`):

- `cornell_box_mcmc.png` — 600×600 @ 1000 spp, 3 min 48 s with light importance sampling
- `book2_final_preview.png` — motion blur + volumes + emission + textures + BVH
- `bunny_preview.png` — Stanford Bunny (35,947 verts / 69,451 tris)

## Structure

```
src/
  rtweekend.h    — shared utilities, constants, RNG
  vec3.h, ray.h, interval.h, color.h
  aabb.h, bvh.h                  — BVH acceleration
  hittable.h, hittable_list.h    — scene graph + transforms
  sphere.h, quad.h, triangle.h   — primitives
  material.h, texture.h          — shading
  perlin.h                       — noise
  onb.h, pdf.h                   — Monte Carlo sampling
  camera.h                       — threaded render loop
  rt_image.h, png_writer.h       — I/O
  obj_loader.h                   — minimal OBJ mesh loader
  constant_medium.h              — participating media
  main.cc                        — scenes + entry
external/
  stb_image.h, stb_image_write.h (from nothings/stb)
scenes/
  earthmap.jpg, bunny.obj        — assets
```

## References

- [*Ray Tracing in One Weekend* series](https://raytracing.github.io/) — Peter Shirley et al.
- [stb single-header libraries](https://github.com/nothings/stb)
- Stanford Bunny OBJ via [alecjacobson/common-3d-test-models](https://github.com/alecjacobson/common-3d-test-models)
