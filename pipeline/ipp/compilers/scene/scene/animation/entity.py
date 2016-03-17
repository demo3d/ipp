import logging as log
from ipp.schema.resource.scene.EntitySequence import *
from ipp.schema.resource.scene.InterpolationMode import *
from ipp.compilers.render.armature import ArmatureBuilder, uniform_scale
from .action import ActionBuilder
from .track import TrackBuilder
from .armature import BonePoseChannelBuilder
from .node import NodeHiddenChannelBuilder, NodeTransformChannelBuilder, \
    interpolation_map, data_path_transform_property_map


def build_entity_action(animation_builder, index, bl_action, bl_armature):
    channels = []
    action_contains_poses = False

    # export recognized fcurves from blender action to channels
    for bl_fcurve in bl_action.fcurves:
        # pose animation is exported separately (not from fcurves)
        if bl_fcurve.data_path.startswith('pose.bones['):
            action_contains_poses = True
            continue

        if bl_fcurve.data_path == 'hide':
            keyframes = []
            for bl_keyframe in bl_fcurve.keyframe_points:
                if bl_keyframe.interpolation != 'CONSTANT':
                    raise ValueError(
                        "Only CONSTANT interpolation supported for bool keyframes,"
                        "found {} instead".format(bl_keyframe.interpolation))
                keyframes.append((
                    animation_builder.frames_to_milliseconds(bl_keyframe.co[0]),
                    bl_keyframe.co[1] != 0))
            channels.append(NodeHiddenChannelBuilder(keyframes))
            continue
        else:
            transform_property_map = data_path_transform_property_map.get(bl_fcurve.data_path, None)
            if transform_property_map:
                transform_property = transform_property_map[bl_fcurve.array_index]
                keyframes = []
                for bl_keyframe in bl_fcurve.keyframe_points:
                    time = animation_builder.frames_to_milliseconds(bl_keyframe.co[0])
                    value = bl_keyframe.co[1]
                    interpolation = interpolation_map.get(bl_keyframe.interpolation)
                    if interpolation is None:
                        raise ValueError(
                            "Blender FCurve {}[{}] interpolation mode not supported : {}".format(
                                bl_fcurve.data_path,
                                bl_fcurve.array_index,
                                bl_keyframe.interpolation))
                    if interpolation == InterpolationMode.Bezier:
                        keyframes.append((
                            time, value, interpolation,
                            animation_builder.frames_to_milliseconds(bl_keyframe.handle_left[0]),
                            bl_keyframe.handle_left[1],
                            animation_builder.frames_to_milliseconds(bl_keyframe.handle_right[0]),
                            bl_keyframe.handle_right[1]))
                    else:
                        keyframes.append((time, value, interpolation, 0, 0, 0, 0))
                channels.append(NodeTransformChannelBuilder(transform_property, keyframes))
                continue

            log.debug("Found unrecognized FCurve %s in Blender Action %s, skipping.",
                      bl_fcurve.data_path, bl_action.name)

    # exporting armature bone channels requires a different approach from fcurve channel
    # need to go trough every scene frame and sample bone values at each frame
    if bl_armature and action_contains_poses:
        armature = ArmatureBuilder(bl_armature)

        log.debug("Exporting armature : %s animation action : %s with frame range : %s-%s",
            bl_armature.name,
            bl_action.name,
            bl_action.frame_range[0],
            bl_action.frame_range[1])

        # need to change frame/action state to export data so store old values to restore them later
        prev_action = bl_armature.animation_data.action
        prev_frame = animation_builder.bl_scene.frame_current
        bl_armature.animation_data.action = bl_action

        try:
            # TODO: Eliminate frames that can be approximated with interpolation
            #       instead of exporting all frames
            bone_index_map = {bone.name: index for index, bone in enumerate(armature.bones)}
            bone_keyframes = {bl_bone: []
                for bl_bone in bl_armature.pose.bones
                if bl_bone.name in bone_index_map}

            action_start = int(bl_action.frame_range[0])
            action_end = int(bl_action.frame_range[1])
            for frame in range(action_start, action_end):
                animation_builder.bl_scene.frame_set(frame)
                for bl_bone, keyframes in bone_keyframes.items():
                    if bl_bone.parent:
                        pose_matrix = bl_bone.parent.matrix.inverted() * bl_bone.matrix
                    else:
                        pose_matrix = bl_bone.matrix
                    translation, rotation, scale = pose_matrix.decompose()

                    keyframes.append((
                        animation_builder.frames_to_milliseconds(frame),
                        translation.x, translation.y, translation.z,
                        rotation.x, rotation.y, rotation.z, rotation.w,
                        uniform_scale(scale)))

            for bl_bone, keyframes in bone_keyframes.items():
                channels.append(BonePoseChannelBuilder(bone_index_map[bl_bone.name], keyframes))

        finally:
            # restore frame/action state
            bl_armature.animation_data.action = prev_action
            animation_builder.bl_scene.frame_set(prev_frame)

    return ActionBuilder(index, bl_action.name, channels)


class EntitySequenceBuilder:
    def __init__(self, animation_builder, entity_builder):
        self.entity_id = entity_builder.id
        self.actions = []
        self.tracks = []

        bl_object = entity_builder.bl_object

        # build entity actions
        bl_anim_data = bl_object.animation_data
        if bl_anim_data is None or len(bl_anim_data.nla_tracks) == 0 and bl_anim_data.action is None:
            return
        bl_tracks = []
        if bl_anim_data.use_nla and bl_anim_data.action is None:
            bl_tracks = [track for track in bl_anim_data.nla_tracks]
            # if there is a solo track in nla tracks just use that track
            for track in bl_tracks:
                if track.is_solo:
                    bl_tracks = [track]
                    break
            bl_actions = set(bl_strip.action for bl_track in bl_tracks for bl_strip in bl_track.strips)
        elif bl_anim_data.action is not None:
            bl_actions = [bl_anim_data.action]
        else:
            raise ValueError(
                "Blender object {} (entity id {}) does not contain an active action or "
                "use NLA stack ?".format(bl_object.name, self.entity_id))

        action_map = {}
        for bl_action in bl_actions:
            bl_armature = bl_object if bl_object.type == 'ARMATURE' else None

            action = build_entity_action(
                animation_builder,
                len(self.actions),
                bl_action,
                bl_armature)
            action_map[bl_action] = action
            self.actions.append(action)

        # build object tracks from blender tracks
        if len(bl_tracks) == 0:
            # if object has not tracks then use active action as a full single strip track with full action duration
            assert len(self.actions) == 1
            assert bl_actions[0] == bl_anim_data.action

            bl_action = bl_anim_data.action
            action_duration = bl_action.frame_range[1] - bl_action.frame_range[0]
            action_duration = animation_builder.frames_to_milliseconds(action_duration)
            self.tracks.append(TrackBuilder(bl_action.name, [(
                bl_action.name,
                0, action_duration,
                0,
                animation_builder.frames_to_milliseconds(bl_action.frame_range[0]),
                animation_builder.frames_to_milliseconds(bl_action.frame_range[1]))]))
        else:
            for bl_track in bl_tracks:
                log.debug('Exporting blender track %s for %s',
                    bl_track.name,
                    entity_builder.bl_object.name)
                if len(bl_track.strips) == 0:
                    continue
                strips = []
                for bl_strip in bl_track.strips:
                    assert bl_strip.type == 'CLIP', \
                        "Only clip NLA strips supported, found {} in {}".format(
                            bl_strip.type,
                            bl_strip.name)
                    strips.append((
                        bl_strip.name,
                        animation_builder.frames_to_milliseconds(bl_strip.frame_start),
                        animation_builder.frames_to_milliseconds(
                            bl_strip.frame_end - bl_strip.frame_start),
                        action_map[bl_strip.action].index,
                        animation_builder.frames_to_milliseconds(bl_strip.action_frame_start),
                        animation_builder.frames_to_milliseconds(
                            bl_strip.action_frame_end - bl_strip.action_frame_start)))
                self.tracks.append(TrackBuilder(bl_track.name, strips))

    def build(self, builder):
        action_offsets = [action_builder.build(builder) for action_builder in self.actions]
        EntitySequenceStartActionsVector(builder, len(action_offsets))
        for action in reversed(action_offsets):
            builder.PrependUOffsetTRelative(action)
        actions_offset = builder.EndVector(len(action_offsets))

        track_offsets = [track.build(builder) for track in self.tracks]
        EntitySequenceStartTracksVector(builder, len(track_offsets))
        for track in reversed(track_offsets):
            builder.PrependUOffsetTRelative(track)
        tracks_offset = builder.EndVector(len(track_offsets))

        EntitySequenceStart(builder)
        EntitySequenceAddEntity(builder, self.entity_id)
        EntitySequenceAddActions(builder, actions_offset)
        EntitySequenceAddTracks(builder, tracks_offset)

        return EntitySequenceEnd(builder)
