//
// Created by leixing on 2025/4/8.
//

#pragma once

#include <cstdint>

#include <vulkan/vulkan.hpp>

namespace image {

    class ImageInterface {
    private:
        // fields

    public:
        ImageInterface();

        virtual ~ImageInterface() = 0;

        [[nodiscard]]
        virtual uint32_t getWidth() const = 0;

        [[nodiscard]]
        virtual uint32_t getHeight() const = 0;

        [[nodiscard]]
        virtual vk::Format getVkFormat() const = 0;

        [[nodiscard]]
        virtual const void *getPixels() const = 0;

        [[nodiscard]]
        virtual uint32_t getPixelBytes() const = 0;
    };

} // vklite
