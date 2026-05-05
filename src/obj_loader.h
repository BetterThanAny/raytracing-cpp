#pragma once

#include "hittable_list.h"
#include "material.h"
#include "rtweekend.h"
#include "triangle.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>

inline int parse_obj_face_index(const std::string& path, int line_number,
                                const std::string& token, size_t vertex_count) {
    size_t slash = token.find('/');
    std::string num = (slash == std::string::npos) ? token : token.substr(0, slash);
    if (num.empty()) {
        throw std::runtime_error("obj: " + path + ":" + std::to_string(line_number) +
                                 ": face token '" + token + "' has no vertex index");
    }

    size_t pos = 0;
    int raw_index = 0;
    try {
        raw_index = std::stoi(num, &pos);
    } catch (const std::exception&) {
        throw std::runtime_error("obj: " + path + ":" + std::to_string(line_number) +
                                 ": invalid face index '" + num + "'");
    }

    if (pos != num.size()) {
        throw std::runtime_error("obj: " + path + ":" + std::to_string(line_number) +
                                 ": invalid face index '" + num + "'");
    }
    if (raw_index == 0) {
        throw std::runtime_error("obj: " + path + ":" + std::to_string(line_number) +
                                 ": OBJ face indices are 1-based; 0 is invalid");
    }

    long resolved = raw_index;
    if (raw_index < 0) resolved = static_cast<long>(vertex_count) + raw_index + 1;
    if (resolved < 1 || resolved > static_cast<long>(vertex_count)) {
        throw std::runtime_error("obj: " + path + ":" + std::to_string(line_number) +
                                 ": face index " + std::to_string(raw_index) +
                                 " is out of range for " + std::to_string(vertex_count) +
                                 " vertices");
    }

    return static_cast<int>(resolved - 1);
}

// Minimal .obj loader: handles `v`, `f` (tri/quad with 1-based indices,
// optionally `v/vt/vn` which we strip). Ignores normals/tex coords.
// Returns the triangles wrapped in a BVH-friendly hittable_list.
inline shared_ptr<hittable_list>
load_obj(const std::string& path, shared_ptr<material> mat,
         double scale = 1.0, const point3& translate = point3(0, 0, 0)) {
    auto mesh = make_shared<hittable_list>();
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("obj: cannot open " + path);
    }

    std::vector<point3> verts;
    std::string line;
    int tri_count = 0;
    int line_number = 0;

    while (std::getline(in, line)) {
        ++line_number;
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ss(line);
        std::string tag;
        ss >> tag;

        if (tag == "v") {
            double x, y, z;
            if (!(ss >> x >> y >> z)) {
                throw std::runtime_error("obj: " + path + ":" + std::to_string(line_number) +
                                         ": invalid vertex line");
            }
            verts.emplace_back(x * scale + translate.x(),
                               y * scale + translate.y(),
                               z * scale + translate.z());
        } else if (tag == "f") {
            std::vector<int> idx;
            std::string tok;
            while (ss >> tok) {
                idx.push_back(parse_obj_face_index(path, line_number, tok, verts.size()));
            }
            if (idx.size() < 3) {
                throw std::runtime_error("obj: " + path + ":" + std::to_string(line_number) +
                                         ": face has fewer than 3 vertices");
            }
            for (size_t i = 1; i + 1 < idx.size(); ++i) {
                mesh->add(make_shared<triangle>(verts[idx[0]], verts[idx[i]], verts[idx[i + 1]], mat));
                ++tri_count;
            }
        }
    }

    std::clog << "obj: " << path << " -> " << verts.size() << " verts, "
              << tri_count << " tris\n";
    if (tri_count == 0) {
        throw std::runtime_error("obj: " + path + " contains no faces");
    }
    return mesh;
}
