//
// Created by leixing on 2025/6/7.
//

#pragma once

#include "glm.h"

namespace math {

    class MvpMatrix {
    private:
        // 模型矩阵
        alignas(16)glm::mat4 mModel;

        // 视图矩阵
        alignas(16)glm::mat4 mView;

        // 投影矩阵
        alignas(16)glm::mat4 mProjection;

    public:
        MvpMatrix(glm::mat4 model,
                  glm::mat4 view,
                  glm::mat4 projection);

        MvpMatrix();

        ~MvpMatrix();

        MvpMatrix &model(glm::mat4 model);

        MvpMatrix &view(glm::mat4 view);

        MvpMatrix &projection(glm::mat4 projection);

        glm::mat4 &getModel();

        glm::mat4 &getView();

        glm::mat4 &getProjection();

        glm::mat4 calcMvp();

        MvpMatrix &modelRotateX(float angle);

        MvpMatrix &modelRotateY(float angle);

        MvpMatrix &modelRotateZ(float angle);

        MvpMatrix &modelScale(float scaleX, float scaleY, float scaleZ);

        MvpMatrix &modelScaleX(float scale);

        MvpMatrix &modelScaleY(float scale);

        MvpMatrix &modelScaleZ(float scale);

        MvpMatrix &projectionFlipX();

        MvpMatrix &projectionFlipY();

        MvpMatrix &projectionFlipZ();
    };

} // matrix
