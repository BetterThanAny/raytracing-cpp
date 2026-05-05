# Review and Fix Report

## Changes
- Made BVH construction reject empty ranges instead of recursing.
- Hardened OBJ loading with file-open, face token, index range, and empty mesh validation.
- Made scene build failures print clear errors and exit nonzero instead of crashing.
- Checked PPM write failures, including header/body writes and `fclose`.

## Verification
- `cmake -S . -B /tmp/raytracing-cpp-build && cmake --build /tmp/raytracing-cpp-build -j` passed.
- `raytracer first_light ...` rendered successfully.
- `raytracer bunny_preview ...` now fails clearly with `obj: cannot open scenes/bunny.obj`, which is expected because the asset is missing.
- `git diff --check` passed.

## Remaining
- `scenes/bunny.obj` is still absent. I did not fabricate or import an asset.
