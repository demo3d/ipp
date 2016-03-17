import logging as log
from ipp.schema.primitive.Mat4 import CreateMat4
from ipp.schema.resource.scene.ComponentKind import *
from ipp.schema.resource.scene.NodeComponent import *
from ipp.schema.resource.scene.NodeRotationMode import *
from ipp.schema.resource.scene.NodeTransform import *


rotation_mode_enum_map = {
    'QUATERNION': NodeRotationMode.Quaternion,
    'XYZ': NodeRotationMode.EulerXYZ,
    'XZY': NodeRotationMode.EulerXZY,
    'YXZ': NodeRotationMode.EulerYXZ,
    'YZX': NodeRotationMode.EulerYZX,
    'ZXY': NodeRotationMode.EulerZXY,
    'ZYX': NodeRotationMode.EulerZYX,
    'AXIS_ANGLE': NodeRotationMode.AxisAngle
}


class NodeBuilder:
    def __init__(self, scene_builder, bl_object):
        log.info("Exporting node component from blender object : %s", bl_object.name)

        self.parenting_inverse_matrix = bl_object.matrix_parent_inverse.copy()
        self.translation = (
            bl_object.location.x,
            bl_object.location.y,
            bl_object.location.z)

        self.rotation_mode = rotation_mode_enum_map[bl_object.rotation_mode]
        if self.rotation_mode == NodeRotationMode.Quaternion:
            self.rotation = (
                bl_object.rotation_quaternion.x,
                bl_object.rotation_quaternion.y,
                bl_object.rotation_quaternion.z,
                bl_object.rotation_quaternion.w)
        elif self.rotation_mode == NodeRotationMode.AxisAngle:
            self.rotation = (
                bl_object.rotation_axis_angle[1],
                bl_object.rotation_axis_angle[2],
                bl_object.rotation_axis_angle[3],
                bl_object.rotation_axis_angle[0])
        else:
            self.rotation = (
                bl_object.rotation_euler.x,
                bl_object.rotation_euler.y,
                bl_object.rotation_euler.z,
                0.0)

        self.scale = (
            bl_object.scale.x,
            bl_object.scale.y,
            bl_object.scale.z)
        
        self.parent_id = 0
        if bl_object.parent is not None:
            self.parent_id = scene_builder.entity_map.get(bl_object.parent, 0)

    def build(self, builder):
        NodeComponentStart(builder)
        NodeComponentAddParentEntity(builder, self.parent_id)
        NodeComponentAddTransformParentingInverseMatrix(
            builder,
            CreateMat4(builder, *(e for r in self.parenting_inverse_matrix.row for e in r)))
        NodeComponentAddTransform(builder,
                                  CreateNodeTransform(
                                      builder,
                                      self.translation[0],
                                      self.translation[1],
                                      self.translation[2],

                                      self.rotation[0],
                                      self.rotation[1],
                                      self.rotation[2],
                                      self.rotation[3],
                                      self.rotation_mode,

                                      self.scale[0],
                                      self.scale[1],
                                      self.scale[2]))
        return NodeComponentEnd(builder)

    @property
    def component_kind(self):
        return ComponentKind.NodeComponent
