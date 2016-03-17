#pragma once

#include <ipp/shared.hpp>
#include <ipp/log.hpp>
#include <ipp/noncopyable.hpp>
#include "binding.hpp"

namespace ipp {
namespace render {
namespace gl {

/**
 * @brief Abstract interface to render targets
 */
class RenderTarget : public NonCopyable {
public:
    template <typename T>
    friend class Binding;

    /**
     * @brief Clear flags
     */
    enum ClearFlags {
        Color = GL_COLOR_BUFFER_BIT,
        Depth = GL_DEPTH_BUFFER_BIT,
        Stencil = GL_STENCIL_BUFFER_BIT
    };

    /**
     * @brief Clear state
     */
    struct ClearState {
        glm::vec4 color;
        float depth;
        float stencil;

        uint32_t flags;
    };

private:
    ClearState _clearState;

    /**
     * @brief Is RenderTarget currently bound to active GL context
     */
    virtual bool isBound() const = 0;

    /**
     * @brief Bind RenderTarget to active GL context
     */
    virtual void bind() const = 0;

    /**
     * @brief Unbind RenderTarget to active GL context
     */
    virtual void unbind() const = 0;

public:
    virtual ~RenderTarget() = default;

    /**
     * @brief Return current ClearState value
     */
    ClearState getClearState()
    {
        return _clearState;
    }

    /**
     * @brief Reset render target ClearState by copying state value
     */
    void setClearState(ClearState state)
    {
        _clearState = state;
    }

    /**
     * @brief Clear render target using current render target ClearState
     */
    virtual void clear(const Binding<RenderTarget>& renderTarget);

    /**
     * @brief Render target dimensions
     */
    virtual glm::ivec2 getDimensions() const = 0;
};
}
}
}