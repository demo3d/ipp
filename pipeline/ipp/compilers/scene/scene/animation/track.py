from ipp.schema.resource.scene.Strip import *
from ipp.schema.resource.scene.Track import *


class TrackBuilder:
    def __init__(self, name, strips):
        self.name = name
        self.strips = strips

    def build(self, builder):
        strips = []

        for strip_data in self.strips:
            strip_name = builder.CreateString(strip_data[0])
            StripStart(builder)
            StripAddName(builder, strip_name)
            StripAddTrackOffset(builder, strip_data[1])
            StripAddTrackDuration(builder, strip_data[2])
            StripAddActionIndex(builder, strip_data[3])
            StripAddActionOffset(builder, strip_data[4])
            StripAddActionDuration(builder, strip_data[5])
            strips.append(StripEnd(builder))

        TrackStartStripsVector(builder, len(strips))
        for strip in reversed(strips):
            builder.PrependUOffsetTRelative(strip)
        strips_vector = builder.EndVector(len(strips))

        track_name = builder.CreateString(self.name)

        TrackStart(builder)
        TrackAddName(builder, track_name)
        TrackAddStrips(builder, strips_vector)

        return TrackEnd(builder)
