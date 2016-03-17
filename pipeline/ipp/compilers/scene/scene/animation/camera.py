from ipp.schema.resource.scene.IntKeyFrame import *
from ipp.schema.resource.scene.ActiveCameraChannel import *
from ipp.schema.resource.scene.Channel import *
from ipp.schema.resource.scene.ChannelKind import *


class ActiveCameraChannelBuilder:
    def __init__(self, animation_builder, bl_markers):
        assert len(bl_markers) > 0

        self.keyframes = []
        entity_map = animation_builder.scene_builder.entity_map
        for bl_marker in bl_markers:
            self.keyframes.append((
                animation_builder.frames_to_milliseconds(bl_marker.frame),
                entity_map[bl_marker.camera]))

    def build(self, builder):
        ActiveCameraChannelStartKeyFramesVector(builder, len(self.keyframes))
        for keyframe in reversed(self.keyframes):
            CreateIntKeyFrame(builder, *keyframe)
        keyframes_vector = builder.EndVector(len(self.keyframes))

        ActiveCameraChannelStart(builder)
        ActiveCameraChannelAddKeyFrames(builder, keyframes_vector)
        channel_data_offset = ActiveCameraChannelEnd(builder)

        ChannelStart(builder)
        ChannelAddChannel(builder, channel_data_offset)
        ChannelAddChannelType(builder, ChannelKind.ActiveCameraChannel)
        return ChannelEnd(builder)
