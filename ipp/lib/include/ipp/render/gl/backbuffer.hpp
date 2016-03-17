#pragma once

#include <ipp/shared.hpp>
#include <ipp/log.hpp>
#include <ipp/noncopyable.hpp>
#include "rendertarget.hpp"

namespace ipp {
namespace render {
namespace gl {

/**
 * @brief RenderTarget that renders to output window BackBuffer
 */
class BackBuffer final : public RenderTarget {
private:
    glm::ivec2 _dimensions;

    /**
     * @brief Is BackBuffer the active render target.
     */
    bool isBound() const override;

    /**
     * @brief Bind back buffer by clearing any frame buffer and updating viewport
     */
    void bind() const override;

    /**
     * @brief No-op simply implements RenderTarget
     */
    void unbind() const override;

public:
    BackBuffer();

    /**
     * @brief BackBuffer dimensions
     */
    glm::ivec2 getDimensions() const override;

    /**
     * @brief Update dimensions variable
     * @note Does not resize backbuffer, just resets private dimension values
     */
    void setDimensions(glm::ivec2 dimensions);
};
}
}
}