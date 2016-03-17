import flatbuffers
from ..generator import build_generator


def build_effect(manifest, resource_path, effect_data):
    """
    Create effect resource at resource_path from data dict
    """
    from ipp.compilers.render.effect import compile_effect

    package = manifest.package
    effect_data.pop('kind')
    with package.create(resource_path, mode='wb') as resource:
        builder = flatbuffers.Builder(1024 * 128)
        builder.Finish(compile_effect(builder, manifest, effect_data))
        resource.write(builder.Output())


@build_generator('effect')
def build_generator_effect(manifest, resource_name, effect_data):
    """
    Generate doit task definitions that build effect resource from effect_data.
    """
    package = manifest.package
    resource_path = manifest.qualified_resource_path(resource_name)
    resource_output = package.resource_output_file_path(resource_path)

    yield {
        'name': 'effect:' + resource_path,
        'actions': [(build_effect, [manifest, resource_path, effect_data])],
        'targets': [resource_output]}
