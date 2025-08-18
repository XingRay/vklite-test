//
// Created by leixing on 2025/6/7.
//

#include "MvpMatrix.h"

namespace math {

    MvpMatrix::MvpMatrix(glm::mat4 model,
                         glm::mat4 view,
                         glm::mat4 projection)
            : mModel(model), mView(view), mProjection(projection) {}

    MvpMatrix::MvpMatrix()
            : mModel{glm::mat4(1.0f)}, mView{glm::mat4(1.0f)}, mProjection{glm::mat4(1.0f)} {}

    MvpMatrix::~MvpMatrix() = default;

    MvpMatrix &MvpMatrix::model(glm::mat4 model) {
        mModel = model;
        return *this;
    }

    MvpMatrix &MvpMatrix::view(glm::mat4 view) {
        mView = view;
        return *this;
    }

    MvpMatrix &MvpMatrix::projection(glm::mat4 projection) {
        mProjection = projection;
        return *this;
    }

    glm::mat4 &MvpMatrix::getModel() {
        return mModel;
    }

    glm::mat4 &MvpMatrix::getView() {
        return mView;
    }

    glm::mat4 &MvpMatrix::getProjection() {
        return mProjection;
    }

    glm::mat4 MvpMatrix::calcMvp() {
        return mProjection * mView * mModel;
    }

    MvpMatrix &MvpMatrix::modelRotateX(float angle) {
        mModel = glm::rotate(mModel, angle, glm::vec3(1.0f, 0.0f, 0.0f));
        return *this;
    }

    MvpMatrix &MvpMatrix::modelRotateY(float angle) {
        mModel = glm::rotate(mModel, angle, glm::vec3(0.0f, 1.0f, 0.0f));
        return *this;
    }

    MvpMatrix &MvpMatrix::modelRotateZ(float angle) {
        mModel = glm::rotate(mModel, angle, glm::vec3(0.0f, 0.0f, 1.0f));
        return *this;
    }

    MvpMatrix &MvpMatrix::modelScale(float scaleX, float scaleY, float scaleZ) {
        mModel = glm::scale(mModel, glm::vec3(scaleX, scaleY, scaleZ));
        return *this;
    }

    MvpMatrix &MvpMatrix::modelScaleX(float scale) {
        modelScale(scale, 1.0f, 1.0f);
        return *this;
    }

    MvpMatrix &MvpMatrix::modelScaleY(float scale) {
        modelScale(1.0f, scale, 1.0f);
        return *this;
    }

    MvpMatrix &MvpMatrix::modelScaleZ(float scale) {
        modelScale(1.0f, 1.0f, scale);
        return *this;
    }

    /**
     * X轴翻转（水平镜像）
     * 修改投影矩阵的 [0][0]（第1行第1列），即X轴缩放因子。乘以 -1 后，X坐标被水平镜像，实现左右翻转效果。
     * 应用场景：创建水平对称图形（如镜像特效）。
     */
    MvpMatrix &MvpMatrix::projectionFlipX() {
        mProjection[0][0] *= -1;
        return *this;
    }

    /**
     * Y轴翻转
     * 投影矩阵中 [1][1]（第2行第2列）控制Y轴缩放因子。
     * 乘以 -1 后，观察空间的Y坐标被镜像映射到裁剪空间，解决OpenGL与屏幕坐标系（Y轴向下）的差异。
     * 应用场景：适配Android等平台的屏幕坐标系。
     */
    MvpMatrix &MvpMatrix::projectionFlipY() {
        mProjection[1][1] *= -1;
        return *this;
    }

    /**
     * Z轴翻转（深度方向反转）
     * [2][2]（第3行第3列）控制Z轴缩放，反转后观察空间Z坐标方向改变（如从指向屏幕外变为指向屏幕内）。
     * [2][3]（第3行第4列）是透视投影的偏移参数，需同步调整以保持深度计算正确性。
     * 应用场景：适配不同深度坐标系（如Vulkan的Z轴范围 [0,1] vs OpenGL的 [-1,1]）。
     */
    MvpMatrix &MvpMatrix::projectionFlipZ() {
        // 反转Z缩放因子
        mProjection[2][2] *= -1;
        // 调整透视偏移
        mProjection[2][3] *= -1;
        return *this;
    }

} // matrix