import logging as log
from ipp.schema.resource.scene.Component import *
from ipp.schema.resource.scene.Entity import *
from .model import ModelBuilder
from .armature import ArmatureBuilder
from .camera import CameraBuilder
from .light import LightBuilder
from .node import NodeBuilder


class EntityBuilder:
    def __init__(self, scene_builder, id, bl_object):
        self.bl_object = bl_object
        self.id = id
        self.component_builders = [NodeBuilder(scene_builder, bl_object)]

        if bl_object.type == 'MESH':
            self.component_builders.append(ModelBuilder(scene_builder, bl_object))
        elif bl_object.type == 'ARMATURE':
            self.component_builders.append(ArmatureBuilder(scene_builder, bl_object))
        elif bl_object.type == 'CAMERA':
            self.component_builders.append(CameraBuilder(bl_object))
        elif bl_object.type == 'LAMP':
            self.component_builders.append(LightBuilder(bl_object))
        elif bl_object.type == 'EMPTY':
            pass
        else:
            log.warning('Unknown blender object type %s', bl_object.type)

    def build(self, builder):
        components = []
        for component_builder in self.component_builders:
            component_object = component_builder.build(builder)
            ComponentStart(builder)
            ComponentAddComponent(builder, component_object)
            ComponentAddComponentType(builder, component_builder.component_kind)
            components.append(ComponentEnd(builder))

        EntityStartComponentsVector(builder, len(components))
        for component in reversed(components):
            builder.PrependUOffsetTRelative(component)
        components_vector = builder.EndVector(len(components))

        name = builder.CreateString(self.bl_object.name)

        EntityStart(builder)
        EntityAddId(builder, self.id)
        EntityAddName(builder, name)
        EntityAddComponents(builder, components_vector)

        return EntityEnd(builder)
