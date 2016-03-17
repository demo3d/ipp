import logging as log
from ipp.schema.resource.scene.ComponentKind import *
from ipp.schema.resource.scene.ArmatureComponent import *


class ArmatureBuilder:
    def __init__(self, scene_builder, bl_object):
        self.armature_resource_path = scene_builder.resource_path(bl_object)

        log.info("Exporting armature component from object %s linked to resource %s",
                 bl_object.name,
                 self.armature_resource_path)

    @property
    def component_kind(self):
        return ComponentKind.ArmatureComponent

    def build(self, builder):
        armature_resource_path_offset = builder.CreateString(self.armature_resource_path)
        ArmatureComponentStart(builder)
        ArmatureComponentAddArmatureResourcePath(builder, armature_resource_path_offset)
        return ArmatureComponentEnd(builder)
