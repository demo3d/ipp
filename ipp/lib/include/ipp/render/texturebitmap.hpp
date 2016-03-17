#pragma once

#include <ipp/shared.hpp>
#include <ipp/resource/resource.hpp>
#include "gl/texture2d.hpp"

namespace ipp {
namespace render {

/**
 * @brief Resource implementation for Texture2D that loads texture from PNG resource.
 */
class BitmapTexture2D final : public gl::Texture2D,
                              public resource::SharedResourceT<BitmapTexture2D> {
public:
    BitmapTexture2D(std::unique_ptr<resource::ResourceBuffer> data);
};
}
}
