//
// Created by leixing on 2025/6/7.
//

#pragma once

#include <vector>
#include <cstdint>

#include "Vertex.h"

namespace model {

    class Model {
    private:
        std::vector<Vertex> mVertices;
        std::vector<uint32_t> mIndices;

    public:
        Model(std::vector<Vertex> &&vertices, std::vector<uint32_t> &&indices);

        ~Model();


        [[nodiscard]]
        const std::vector<Vertex> &getVertices() const;

        [[nodiscard]]
        uint32_t getVerticesBytes() const;

        [[nodiscard]]
        const void *getVerticesData() const;


        [[nodiscard]]
        const std::vector<uint32_t> &getIndices() const;

        [[nodiscard]]
        uint32_t getIndicesBytes() const;

        [[nodiscard]]
        uint32_t getIndicesCount() const;

        [[nodiscard]]
        const void *getIndicesData() const;
    };
}


