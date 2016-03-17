from ipp.schema.resource.scene.BonePoseKeyFrame import *
from ipp.schema.resource.scene.BonePoseChannel import *
from ipp.schema.resource.scene.Channel import *
from ipp.schema.resource.scene.ChannelKind import *


class BonePoseChannelBuilder:
    def __init__(self, bone_index, keyframes):
        self.bone_index = bone_index
        self.keyframes = keyframes

    def build(self, builder):
        BonePoseChannelStartKeyFramesVector(builder, len(self.keyframes))
        for keyframe in reversed(self.keyframes):
            CreateBonePoseKeyFrame(builder, *keyframe)
        keyframes_vector = builder.EndVector(len(self.keyframes))

        BonePoseChannelStart(builder)
        BonePoseChannelAddBone(builder, self.bone_index)
        BonePoseChannelAddKeyFrames(builder, keyframes_vector)
        channel_data_offset = BonePoseChannelEnd(builder)

        ChannelStart(builder)
        ChannelAddChannel(builder, channel_data_offset)
        ChannelAddChannelType(builder, ChannelKind.BonePoseChannel)
        return ChannelEnd(builder)
