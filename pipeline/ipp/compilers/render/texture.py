from PIL import Image
from ipp.schema.resource.render.texture.PixelFormat import *
from ipp.schema.resource.render.texture.Texture import *


def compile_texture(builder, manifest, data):
    """
    Decode source image to a bitmap exported as texture resource.

    Bitmap is flipped vertically because GL uses (bottom, left) as origin but the rest of the world
    uses (top, left) and this is the simplest way to fix the issue :)
    """
    from flatbuffers.number_types import UOffsetTFlags, Uint8Flags

    image = Image.open(data['source'])
    image = image.transpose(Image.FLIP_TOP_BOTTOM)

    # TODO: Implement other formats (eg. GPU texture compression, grayscale, alpha)
    assert image.mode == "RGB"

    bitmap = image.tobytes()

    builder.Prep(UOffsetTFlags.bytewidth, (len(bitmap)) * Uint8Flags.bytewidth)
    TextureStartBitmapVector(builder, len(bitmap))
    l = UOffsetTFlags.py_type(len(bitmap))
    builder.head = UOffsetTFlags.py_type(builder.Head() - l)
    builder.Bytes[builder.Head(): builder.Head() + l] = bitmap
    bitmap_offset = builder.EndVector(len(bitmap))

    TextureStart(builder)
    TextureAddBitmap(builder, bitmap_offset)
    TextureAddWidth(builder, image.width)
    TextureAddHeight(builder, image.height)
    TextureAddPixelFormat(builder, PixelFormat.RGB8)
    result = TextureEnd(builder)

    return result
