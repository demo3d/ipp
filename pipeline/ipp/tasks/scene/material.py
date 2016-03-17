import flatbuffers
from ..generator import build_generator


def build_material(manifest, resource_path, material_definition):
    from ipp.compilers.scene.material import MaterialDefinitionBuilder
    package = manifest.package
    material_builder = MaterialDefinitionBuilder(manifest, material_definition)
    with package.create(resource_path, mode='wb') as resource_file:
        builder = flatbuffers.Builder(1024 * 512)
        builder.Finish(material_builder.build(builder))
        resource_file.write(builder.Output())


@build_generator('material')
def build_generator_material(manifest, resource_name, material_definition):
    package = manifest.package
    resource_path = manifest.qualified_resource_path(resource_name)
    resource_output = package.resource_output_file_path(resource_path)

    yield {'name': 'material:' + resource_path,
           'actions': [(build_material, [manifest, resource_path, material_definition])],
           'targets': [resource_output]}
