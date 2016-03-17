from ipp.schema.resource.scene.Action import *


class ActionBuilder:
    def __init__(self, index, name, channels):
        self.index = index
        self.name = name
        self.channels = channels

    def build(self, builder):
        name_offset = builder.CreateString(self.name)

        channel_offsets = [channel.build(builder) for channel in self.channels]
        ActionStartChannelsVector(builder, len(channel_offsets))
        for channel in reversed(channel_offsets):
            builder.PrependUOffsetTRelative(channel)
        channels_offset = builder.EndVector(len(channel_offsets))

        ActionStart(builder)
        ActionAddName(builder, name_offset)
        ActionAddChannels(builder, channels_offset)
        return ActionEnd(builder)
