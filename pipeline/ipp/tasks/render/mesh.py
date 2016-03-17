import re
import flatbuffers
import bpy
from ..generator import build_generator, open_blend


def build_mesh(package, resource_path, mesh_definition):
    """
    Compile mesh data from blender object specified by mesh_definition dict.

    mesh_definition contains following entries:
        * source :
            * blend - absolute path to source blend file
            * scene - scene name string to which mesh belongs to
            * object - mesh scene object name string
        * armature - armature object which will be used to export skinning weights
        * uv_layers - array of strings with names of mesh uv layers
        * color_layers - array of strings with names of mesh color layers
        * groups_per_vertex - int number of maximum groups per vertex (excluding skinning)
        * groups_filter - regex string filter that rejects resource groups
        * groups_have_weight - bool indicationg if group weights are exported with group indices

    Returns resource path.
    """
    from ipp.compilers.render.mesh import export_mesh, create_triangulated_bmesh
    from ipp.compilers.render.armature import ArmatureBuilder

    mesh_source = mesh_definition['source']

    open_blend(mesh_source['blend'])

    bl_scene = bpy.data.scenes[mesh_source['scene']]
    bl_object = bl_scene.objects[mesh_source['object']]
    assert bl_object.type == 'MESH'

    bl_bmesh = create_triangulated_bmesh(bl_object.data)

    normals = mesh_definition.get('normals', True)
    uv_layers = mesh_definition.get('uv_layers', [])
    color_layers = mesh_definition.get('color_layers', [])
    groups_per_vertex = mesh_definition.get('groups_per_vertex', 0)
    groups_have_weight = mesh_definition.get('groups_have_weight', False)

    skinning_index_map = None
    if 'armature' in mesh_definition:
        bl_armature = bl_scene.objects[mesh_definition['armature']]
        assert bl_armature.type == 'ARMATURE'
        skinning_index_map = ArmatureBuilder(bl_armature).get_group_skinning_index_map(bl_object)

    groups_index_map = None
    groups_filter = mesh_definition.get('groups_filter', 'a^')
    if groups_per_vertex > 0:
        groups_filter_re = re.compile(groups_filter)
        groups = [g for g in bl_object.vertex_groups if not groups_filter_re.match(g.name)]
        groups_index_map = {g.index: index for index, g in enumerate(groups)}
        assert len(groups_index_map) > 0

    with package.create(resource_path, mode='wb') as resource_file:
        builder = flatbuffers.Builder(1024 * 1024)
        builder.Finish(
            export_mesh(
                builder,
                bl_bmesh,
                normals,
                skinning_index_map,
                uv_layers,
                color_layers,
                groups_per_vertex,
                groups_index_map,
                groups_have_weight))
        resource_file.write(builder.Output())


@build_generator('mesh')
def build_generator_mesh(manifest, resource_name, mesh_definition):
    package = manifest.package
    resource_path = manifest.qualified_resource_path(resource_name)
    resource_output = package.resource_output_file_path(resource_path)
    blend_path = manifest.source_path(mesh_definition['source']['blend'])
    mesh_definition['source']['blend'] = blend_path

    yield {'name': 'mesh:' + resource_path,
           'actions': [(build_mesh, [package, resource_path, mesh_definition])],
           'file_dep': [blend_path],
           'targets': [resource_output]}
