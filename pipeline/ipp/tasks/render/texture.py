import flatbuffers
from ..generator import build_generator


def build_texture(manifest, resource_path, texture_definition):
    """
    Convert source image to vertically flipped PNG texture.

    Vertical flip is necessary because GL uses (bottom, left) as origin but the rest of the world
    uses (top, left) and this is the simplest way to fix the issue :)
    """
    from ipp.compilers.render.texture import compile_texture

    package = manifest.package
    with package.create(resource_path, mode='wb') as resource:
        builder = flatbuffers.Builder(1024 * 1024 * 8)
        builder.Finish(compile_texture(builder, manifest, texture_definition))
        resource.write(builder.Output())


@build_generator('texture')
def build_generator_texture(manifest, resource_name, texture_definition):
    package = manifest.package
    resource_path = manifest.qualified_resource_path(resource_name)
    resource_output = package.resource_output_file_path(resource_path)
    image_path = manifest.source_path(texture_definition['source'])
    texture_definition['source'] = image_path

    yield {'name': 'texture:' + resource_path,
           'actions': [(build_texture, [manifest, resource_path, texture_definition])],
           'file_dep': [image_path],
           'targets': [resource_output]}
