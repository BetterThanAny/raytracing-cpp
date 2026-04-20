#pragma once

#include "hittable_list.h"
#include "material.h"
#include "rtweekend.h"
#include "triangle.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Minimal .obj loader: handles `v`, `f` (tri/quad with 1-based indices,
// optionally `v/vt/vn` which we strip). Ignores normals/tex coords.
// Returns the triangles wrapped in a BVH-friendly hittable_list.
inline shared_ptr<hittable_list>
load_obj(const std::string& path, shared_ptr<material> mat,
         double scale = 1.0, const point3& translate = point3(0, 0, 0)) {
    auto mesh = make_shared<hittable_list>();
    std::ifstream in(path);
    if (!in) {
        std::cerr << "obj: cannot open " << path << "\n";
        return mesh;
    }

    std::vector<point3> verts;
    std::string line;
    int tri_count = 0;

    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ss(line);
        std::string tag;
        ss >> tag;

        if (tag == "v") {
            double x, y, z;
            ss >> x >> y >> z;
            verts.emplace_back(x * scale + translate.x(),
                               y * scale + translate.y(),
                               z * scale + translate.z());
        } else if (tag == "f") {
            std::vector<int> idx;
            std::string tok;
            while (ss >> tok) {
                size_t slash = tok.find('/');
                std::string num = (slash == std::string::npos) ? tok : tok.substr(0, slash);
                int i = std::stoi(num);
                if (i < 0) i = static_cast<int>(verts.size()) + i + 1;
                idx.push_back(i - 1);
            }
            for (size_t i = 1; i + 1 < idx.size(); ++i) {
                mesh->add(make_shared<triangle>(verts[idx[0]], verts[idx[i]], verts[idx[i + 1]], mat));
                ++tri_count;
            }
        }
    }

    std::clog << "obj: " << path << " -> " << verts.size() << " verts, "
              << tri_count << " tris\n";
    return mesh;
}
