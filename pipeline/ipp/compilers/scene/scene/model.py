import logging as log
from ipp.schema.resource.scene.ComponentKind import *
from ipp.schema.resource.scene.ModelComponent import *


class ModelBuilder:
    def __init__(self, scene_builder, bl_object):
        log.info("Exporting model component from blender object : %s", bl_object.name)
        self.mesh_resource_path = scene_builder.resource_path(bl_object)
        log.info("Linking object %s model to mesh %s", bl_object.name, self.mesh_resource_path)
        self.armature_id = 0

        bl_armature = bl_object.find_armature()
        if bl_armature:
            self.armature_id = scene_builder.entity_map[bl_armature]

        self.material_resource_path = scene_builder.material_resource_path(bl_object)

    @property
    def component_kind(self):
        return ComponentKind.ModelComponent

    def build(self, builder):
        mesh_resource_path_offset = builder.CreateString(self.mesh_resource_path)
        material_resource_path_offset = builder.CreateString(self.material_resource_path)

        ModelComponentStart(builder)
        ModelComponentAddMeshResourcePath(builder, mesh_resource_path_offset)
        ModelComponentAddMaterialResourcePath(builder, material_resource_path_offset)
        ModelComponentAddArmatureEntity(builder, self.armature_id)

        return ModelComponentEnd(builder)
