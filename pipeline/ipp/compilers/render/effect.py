from ipp.schema.resource.render.effect.EffectPass import *
from ipp.schema.resource.render.effect.Effect import *


def compile_effect(builder, manifest, data):
    """
    Create a bytearray containing flatbuffers data for Effect object compiled from effect_source.

    data contains two members :
    - vertex_attributes : list of vertex attribute strings

    - passes : a dict of uniquely named effect pass dicts
        * vertex_header - string prepended to vertex shader source before compilation
        * vertex_resources - resource_path strings for vertex shader sources
        * fragment_header - string prepended to fragment shader source before compilation
        * fragment_resources - list of resource_path strings for fragment shader sources
    """
    vertex_attribute_offsets = [
        builder.CreateString(attribute)
        for attribute in data['vertex_attributes']]

    EffectStartVertexAttributesVector(builder, len(vertex_attribute_offsets))
    for attribute_offset in reversed(vertex_attribute_offsets):
        builder.PrependUOffsetTRelative(attribute_offset)
    vertex_attributes_offset = builder.EndVector(len(vertex_attribute_offsets))

    effect_pass_offsets = []
    for pass_name, pass_data in data['passes'].items():
        pass_name_offset = builder.CreateString(pass_name)

        vertex_header = pass_data['vertex_header'] + '\n'
        vertex_header_offset = builder.CreateString(vertex_header)
        vertex_resource_path_offsets = [
            builder.CreateString(
                manifest.qualified_resource_path(resource_path))
            for resource_path in pass_data['vertex_resources']]

        EffectPassStartVertexResourcePathsVector(builder, len(vertex_resource_path_offsets))
        for path_offset in reversed(vertex_resource_path_offsets):
            builder.PrependUOffsetTRelative(path_offset)
        vertex_resource_paths_offset = builder.EndVector(len(vertex_resource_path_offsets))

        fragment_header = pass_data['fragment_header'] + '\n'
        fragment_header_offset = builder.CreateString(fragment_header)
        fragment_resource_path_offsets = [
            builder.CreateString(
                manifest.qualified_resource_path(resource_path))
            for resource_path in pass_data['fragment_resources']]

        EffectPassStartFragmentResourcePathsVector(builder, len(fragment_resource_path_offsets))
        for path_offset in reversed(fragment_resource_path_offsets):
            builder.PrependUOffsetTRelative(path_offset)
        fragment_resource_paths_offset = builder.EndVector(len(fragment_resource_path_offsets))

        EffectPassStart(builder)
        EffectPassAddName(builder, pass_name_offset)
        EffectPassAddVertexHeader(builder, vertex_header_offset)
        EffectPassAddVertexResourcePaths(builder, vertex_resource_paths_offset)
        EffectPassAddFragmentHeader(builder, fragment_header_offset)
        EffectPassAddFragmentResourcePaths(builder, fragment_resource_paths_offset)

        effect_pass_offsets.append(EffectPassEnd(builder))

    EffectStartPassesVector(builder, len(effect_pass_offsets))
    for effect_pass in reversed(effect_pass_offsets):
        builder.PrependUOffsetTRelative(effect_pass)
    effect_passes_offset = builder.EndVector(len(effect_pass_offsets))

    EffectStart(builder)
    EffectAddVertexAttributes(builder, vertex_attributes_offset)
    EffectAddPasses(builder, effect_passes_offset)
    return EffectEnd(builder)
