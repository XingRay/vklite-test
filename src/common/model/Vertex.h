//
// Created by leixing on 2025/6/7.
//

#pragma once

#include "math/glm.h"

namespace model {

    class Vertex {
    private:
        glm::vec3 mPosition;
        glm::vec2 mUv;

    public:
        Vertex(glm::vec3 position, glm::vec2 uv);

        Vertex();

        ~Vertex();

        bool operator==(const Vertex &other) const;

        [[nodiscard]]
        const glm::vec3 &getPosition() const;

        [[nodiscard]]
        const glm::vec2 &getUv() const;

        Vertex &position(glm::vec3 position);

        Vertex &uv(glm::vec2 uv);

    };

} // model