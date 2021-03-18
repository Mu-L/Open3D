// ----------------------------------------------------------------------------
// -                        Open3D: www.open3d.org                            -
// ----------------------------------------------------------------------------
// The MIT License (MIT)
//
// Copyright (c) 2018 www.open3d.org
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------------------------------------------------------

#include "open3d/t/io/TriangleMeshIO.h"

#include <unordered_map>

#include "open3d/utility/Console.h"
#include "open3d/utility/FileSystem.h"

namespace open3d {
namespace t {
namespace io {

static const std::unordered_map<
        std::string,
        std::function<bool(
                const std::string &, geometry::TriangleMesh &, bool, bool)>>
        file_extension_to_trianglemesh_read_function{
        };

static const std::unordered_map<
        std::string,
        std::function<bool(const std::string &,
                           const geometry::TriangleMesh &,
                           const bool,
                           const bool,
                           const bool,
                           const bool,
                           const bool,
                           const bool)>>
        file_extension_to_trianglemesh_write_function{
        };
}  // unnamed namespace

namespace io {

std::shared_ptr<geometry::TriangleMesh> CreateMeshFromFile(
        const std::string &filename, bool print_progress) {
    auto mesh = std::make_shared<geometry::TriangleMesh>();
    ReadTriangleMesh(filename, *mesh, print_progress);
    return mesh;
}

bool ReadTriangleMesh(const std::string &filename,
                      geometry::TriangleMesh &mesh,
                      bool enable_post_processing /* = false */,
                      bool print_progress /* = false */) {
    std::string filename_ext =
            utility::filesystem::GetFileExtensionInLowerCase(filename);
    if (filename_ext.empty()) {
        utility::LogWarning(
                "Read geometry::TriangleMesh failed: unknown file "
                "extension.");
        return false;
    }

    auto map_itr =
            file_extension_to_trianglemesh_read_function.find(filename_ext);
    bool success = false;
    if (map_itr == file_extension_to_trianglemesh_read_function.end()) {
        open3d::geometry::TriangleMesh legacy_mesh;
        success =
                open3d::io::ReadTriangleMesh(filename, legacy_mesh, enable_post_processing, print_progress);
        if (!success) {
            return false;
        }
        mesh = geometry::TriangleMesh::FromLegacyTriangleMesh(legacy_mesh);
    } else {
        success = map_itr->second(filename, mesh, enable_post_processing, print_progress);
        utility::LogDebug(
            "Read geometry::TriangleMesh: {:d} triangles and {:d} vertices.",
            mesh.GetTriangles().GetLength(), mesh.GetVertices().GetLength());
        if (mesh.HasVertices() && !mesh.HasTriangles()) {
            utility::LogWarning(
                "geometry::TriangleMesh appears to be a geometry::PointCloud "
                "(only contains vertices, but no triangles).");
        }
    }
    return success;
}

bool WriteTriangleMesh(const std::string &filename,
                       const geometry::TriangleMesh &mesh,
                       bool write_ascii /* = false*/,
                       bool compressed /* = false*/,
                       bool write_vertex_normals /* = true*/,
                       bool write_vertex_colors /* = true*/,
                       bool write_triangle_uvs /* = true*/,
                       bool print_progress /* = false*/) {
    std::string filename_ext =
            utility::filesystem::GetFileExtensionInLowerCase(filename);
    if (filename_ext.empty()) {
        utility::LogWarning(
                "Write geometry::TriangleMesh failed: unknown file "
                "extension.");
        return false;
    }
    auto map_itr =
            file_extension_to_trianglemesh_write_function.find(filename_ext);
    if (map_itr == file_extension_to_trianglemesh_write_function.end()) {
        return open3d::io::WriteTriangleMesh(
                filename, mesh.ToLegacyTriangleMesh(), write_ascii, compressed, write_vertex_normals, write_vertex_colors, write_triangle_uvs, print_progress);
    }
    bool success = map_itr->second(filename, mesh, write_ascii, compressed,
                                   write_vertex_normals, write_vertex_colors,
                                   write_triangle_uvs, print_progress);
    utility::LogDebug(
            "Write geometry::TriangleMesh: {:d} triangles and {:d} vertices.",
            mesh.GetTriangles().GetLength(), mesh.GetVertices().GetLength());
    return success;
}

}  // namespace io
}  // namespace t
}  // namespace open3d
