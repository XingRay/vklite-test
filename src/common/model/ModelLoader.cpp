//
// Created by leixing on 2025/6/7.
//

#include "ModelLoader.h"

#include <stdexcept>
#include <tiny_obj_loader.h>

namespace std {
    template<>
    struct hash<model::Vertex> {
        size_t operator()(model::Vertex const &vertex) const {
            size_t seed = 0;

            // 哈希 position
            hash<glm::vec3> vec3Hash;
            seed ^= vec3Hash(vertex.getPosition()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

            // 哈希 uv
            hash<glm::vec2> vec2Hash;
            seed ^= vec2Hash(vertex.getUv()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

            return seed;
        }
    };
}

namespace model {

    Model ModelLoader::load(const char *path) {
        // load vertex
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warning;
        std::string error;

        bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &error, path);
        if (!success) {
            throw std::runtime_error(warning + error);
        }

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        for (const auto &shape: shapes) {
            for (const auto &index: shape.mesh.indices) {
                Vertex vertex{};

                vertex.position({
                                        attrib.vertices[3 * index.vertex_index + 0],
                                        attrib.vertices[3 * index.vertex_index + 1],
                                        attrib.vertices[3 * index.vertex_index + 2]
                                });

                vertex.uv({
                                  attrib.texcoords[2 * index.texcoord_index + 0],
                                  1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
                          });

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = vertices.size();
                    vertices.push_back(vertex);
                }

                indices.push_back(uniqueVertices[vertex]);
            }
        }

//        return Model(std::move(vertices), std::move(indices));
        return {std::move(vertices), std::move(indices)};
    }

} // util