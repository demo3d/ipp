import flatbuffers
import bpy
from ..generator import build_generator, open_blend


def build_armature(package, resource_path, armature_definition):
    """
    Compile armature data from blender object specified by armature_definition dict.

    armature_definition contains following entries:
        * source - dict :
            * file - blend file source
            * scene - string name of scene to which armature belongs to
            * object - string name of armature object

    Returns resource path.
    """
    from ipp.compilers.render.armature import ArmatureBuilder

    armature_source = armature_definition['source']
    open_blend(armature_source['blend'])
    bl_scene = bpy.data.scenes[armature_source['scene']]
    bl_object = bl_scene.objects[armature_source['object']]
    assert bl_object.type == 'ARMATURE'

    armature_builder = ArmatureBuilder(bl_object)
    with package.create(resource_path, mode='wb') as resource_file:
        builder = flatbuffers.Builder(1024 * 256)
        builder.Finish(armature_builder.build(builder))
        resource_file.write(builder.Output())


@build_generator('armature')
def build_generator_armature(manifest, resource_name, armature_definition):
    package = manifest.package
    resource_path = manifest.qualified_resource_path(resource_name)
    resource_output = package.resource_output_file_path(resource_path)
    blend_path = manifest.source_path(armature_definition['source']['blend'])
    armature_definition['source']['blend'] = blend_path

    yield {'name': 'armature:' + resource_path,
           'actions': [(build_armature, [package, resource_path, armature_definition])],
           'file_dep': [blend_path],
           'targets': [resource_output]}
