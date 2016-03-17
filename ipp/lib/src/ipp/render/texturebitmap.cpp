#include <ipp/resource/resourcemanager.hpp>
#include <ipp/render/texturebitmap.hpp>
#include <ipp/schema/resource/render/texture_generated.h>

using namespace std;
using namespace ipp;
using namespace ipp::render;
using namespace ipp::resource;

template <>
const string SharedResourceT<BitmapTexture2D>::ResourceTypeName = "RenderBitmapTexture2D";

BitmapTexture2D::BitmapTexture2D(std::unique_ptr<resource::ResourceBuffer> data)
    : resource::SharedResourceT<BitmapTexture2D>(data->getResourceManager(),
                                                 data->getResourcePath())
{
    auto textureData = schema::resource::render::texture::GetTexture(data->getData());
    IVL_LOG(Info, "Loading Bitmap Texture resource : {}, dimensions {}x{} bitmap size : {}",
            getResourcePath(), textureData->width(), textureData->height(),
            textureData->bitmap()->size());
    {
        auto binding = bind({0});
        binding.texImage2D(
            0, gl::Texture::PixelFormat::Rgb, static_cast<GLsizei>(textureData->width()),
            static_cast<GLsizei>(textureData->height()), gl::Texture::PixelFormat::Rgb,
            gl::Texture::PixelType::UByte, textureData->bitmap()->data());
        binding.generateMipmap();
        binding.setMagFilter(gl::Texture::MagFilter::Linear);
        binding.setMinFilter(gl::Texture::MinFilter::LinearMipmapLinear);
    }
}
