//
// Created by leixing on 2025/6/7.
//

#include "Vertex.h"

namespace model {

    Vertex::Vertex(glm::vec3 position, glm::vec2 uv)
            : mPosition(position), mUv(uv) {}

    Vertex::Vertex()
            : mPosition{}, mUv{} {}

    Vertex::~Vertex() = default;

    bool Vertex::operator==(const Vertex &other) const {
        return mPosition == other.mPosition && mUv == other.mUv;
    }

    const glm::vec3 &Vertex::getPosition() const {
        return mPosition;
    }

    const glm::vec2 &Vertex::getUv() const {
        return mUv;
    }

    Vertex &Vertex::position(glm::vec3 position) {
        mPosition = position;
        return *this;
    }

    Vertex &Vertex::uv(glm::vec2 uv) {
        mUv = uv;
        return *this;
    }

} // model
