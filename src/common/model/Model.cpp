//
// Created by leixing on 2025/6/7.
//

#include "Model.h"

namespace model {

    Model::Model(std::vector<Vertex> &&vertices, std::vector<uint32_t> &&indices)
            : mVertices(std::move(vertices)),
              mIndices(std::move(indices)) {}

    Model::~Model() = default;

    const std::vector<Vertex> &Model::getVertices() const {
        return mVertices;
    }

    uint32_t Model::getVerticesBytes() const {
        return mVertices.size() * sizeof(Vertex);
    }

    const void *Model::getVerticesData() const {
        return mVertices.data();
    }

    const std::vector<uint32_t> &Model::getIndices() const {
        return mIndices;
    }

    uint32_t Model::getIndicesBytes() const {
        return mIndices.size() * sizeof(uint32_t);
    }

    uint32_t Model::getIndicesCount() const {
        return mIndices.size();
    }

    const void *Model::getIndicesData() const {
        return mIndices.data();
    }
}