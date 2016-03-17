import flatbuffers
from ..generator import build_generator, open_blend


def build_scene(manifest, resource_path, scene_definition):
    from ipp.compilers.scene.scene.scene import SceneBuilder

    # merge extra scene definition data from build manifest
    if 'scene' in manifest.defaults:
        scene_globals = manifest.defaults['scene']
        # merge build manifest resource and material definitions with global
        # note that globals are merged after so manifest definitions have priority
        scene_definition['resources'] = scene_definition.get('resources', []) + scene_globals.get('resources', [])
        scene_definition['materials'] = scene_definition.get('materials', []) + scene_globals.get('materials', [])
        # use global object_filter if not specified in build manifest
        if 'object_filter' not in scene_definition and 'object_filter' in scene_globals:
            scene_definition['object_filter'] = scene_globals['object_filter']

    open_blend(scene_definition['source']['blend'])

    package = manifest.package
    scene_builder = SceneBuilder(manifest, scene_definition)
    with package.create(resource_path, mode='wb') as resource_file:
        builder = flatbuffers.Builder(1024 * 1024)
        builder.Finish(scene_builder.build(builder))
        resource_file.write(builder.Output())


@build_generator('scene')
def build_generator_scene(manifest, resource_name, scene_definition):
    package = manifest.package
    resource_path = manifest.qualified_resource_path(resource_name)
    resource_output = package.resource_output_file_path(resource_path)
    blend_path = manifest.source_path(scene_definition['source']['blend'])
    scene_definition['source']['blend'] = blend_path

    yield {'name': 'scene:' + resource_path,
           'actions': [(build_scene, [manifest, resource_path, scene_definition])],
           'file_dep': [blend_path],
           'targets': [resource_output]}
