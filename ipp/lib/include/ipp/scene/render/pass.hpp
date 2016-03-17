#pragma once

#include <ipp/shared.hpp>
#include <ipp/render/gl/rendertarget.hpp>
#include "material.hpp"

namespace ipp {
namespace scene {
namespace render {

/**
 * @brief Abstract base class for a render pass.
 *
 * A render Pass encapsulates rendering a batch of renderable objects to a specific render target.
 * It sets implementation defined state (eg. blending modes), render targets (eg. backbuffer,
 * shadowmap)
 * and selects the desired @see Effect::Pass from each Renderable to render it.
 *
 * A pass can contain multiple subpasses and together all active Passes form a render pipeline.
 *
 * For example VSM shadow pass will contain two sub passes, one to render the shadowmap and the
 * other to
 * smooth/subsample the target before feeding the output to render pass.
 */
class Pass : public NonCopyable {
public:
    virtual ~Pass() = default;

    /**
     * @brief RenderPass render target
     */
    virtual ::ipp::render::gl::RenderTarget& getRenderTarget() = 0;
};
}
}
}
