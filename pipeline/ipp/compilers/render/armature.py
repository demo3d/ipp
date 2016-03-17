import logging as log
from ipp.schema.resource.render.armature.Armature import *
from ipp.schema.resource.render.armature.BindPose import *
from ipp.schema.resource.render.armature.Bone import *


def uniform_scale(scale_vector):
    """
    Convert scale vector to uniform scale value and log if error in some axis is above tolerance
    """
    error_tolerance = 0.001  # tolerate small differences between scaling axes
    scale = (scale_vector.x + scale_vector.y + scale_vector.z) / 3
    if abs(scale - scale_vector.x) > error_tolerance or \
       abs(scale - scale_vector.y) > error_tolerance or \
       abs(scale - scale_vector.z) > error_tolerance:
        log.debug("Non-uniform scale : %s found, avaraging to %s",
                 scale_vector, scale)
    return scale


class BoneData:
    def __init__(self, name, is_deform, parent_index, translation, rotation, scale):
        self.name = name
        self.is_deform = is_deform
        self.parent_index = parent_index
        self.translation = translation
        self.rotation = rotation
        self.scale = scale


class ArmatureBuilder:
    def __init__(self, bl_armature):
        self.bl_armature = bl_armature
        log.debug("Building bone structures for armature %s", bl_armature.name)

        # find deformation bones
        deformation_bones = sorted((bone for bone in bl_armature.data.bones if bone.use_deform),
                                   key=lambda bone:bone.name)
        log.debug("Deformation bones : %s", [bone.name for bone in deformation_bones])

        # find deformation bones + their ancestors
        animation_bone_set = {ancestor for bone in deformation_bones for ancestor in bone.parent_recursive}
        animation_bone_set.update(deformation_bones)

        # sort animation bones topologically to allow linear array update
        # subsort alphabetically to ensure deterministic bone list order
        animation_bones = sorted([bone for bone in animation_bone_set if bone.parent is None], key=lambda b: b.name)
        animation_bone_set.difference_update(animation_bones)
        while len(animation_bone_set) > 0:
            for bone in sorted((b for b in animation_bone_set if b.parent in animation_bones), key=lambda b: b.name):
                animation_bones.append(bone)
            animation_bone_set.difference_update(animation_bones)
        log.debug("Animation bones : %s", [bone.name for bone in animation_bones])

        # build bone list
        self.bones = []
        for bone in animation_bones:
            bone_translation, bone_rotation, bone_scale = bone.matrix_local.decompose()
            self.bones.append(
                BoneData(
                    bone.name,
                    bone.use_deform,
                    animation_bones.index(bone.parent) if bone.parent is not None else -1,
                    bone_translation,
                    bone_rotation,
                    uniform_scale(bone_scale)))

    def get_group_skinning_index_map(self, bl_object):
        """
        :return: Map of object vertex group index -> bone skinning index
        """
        skinning_bones = [bone for bone in self.bones if bone.is_deform]
        skinning_index_map = {bone.name: i for i, bone in enumerate(skinning_bones)}
        return {g.index: skinning_index_map[g.name]
                for g in bl_object.vertex_groups
                if g.name in skinning_index_map}

    def build(self, builder):
        bone_offsets = []
        for bone in self.bones:
            bone_name = builder.CreateString(bone.name)
            BoneStart(builder)
            BoneAddName(builder, bone_name)
            BoneAddParentIndex(builder, bone.parent_index)
            BoneAddIsDeformation(builder, bone.is_deform)
            BoneAddBindPose(builder, CreateBindPose(
                builder,
                bone.translation.x,
                bone.translation.y,
                bone.translation.z,
                bone.rotation.x,
                bone.rotation.y,
                bone.rotation.z,
                bone.rotation.w,
                bone.scale))
            bone_offsets.append(BoneEnd(builder))

        ArmatureStartBonesVector(builder, len(bone_offsets))
        for bone in reversed(bone_offsets):
            builder.PrependUOffsetTRelative(bone)
        bones_offset = builder.EndVector(len(bone_offsets))

        ArmatureStart(builder)
        ArmatureAddBones(builder, bones_offset)
        return ArmatureEnd(builder)
