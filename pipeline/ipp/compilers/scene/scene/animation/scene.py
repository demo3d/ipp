from ipp.schema.resource.scene.SceneSequence import *
from .action import ActionBuilder
from .track import TrackBuilder
from .camera import ActiveCameraChannelBuilder


class SceneSequenceBuilder:
    def __init__(self, animation_builder):
        # build scene tracks
        self.actions = []
        self.tracks = []
        self.sequence_duration = animation_builder.frames_to_milliseconds(animation_builder.bl_scene.frame_end)

        # scene active camera animation track
        bl_markers = sorted(
            [bl_marker for bl_marker in animation_builder.bl_scene.timeline_markers
             if bl_marker.camera],
            key=lambda m: m.frame)

        if len(bl_markers) > 0:
            active_camera_action = ActionBuilder(
                len(self.actions),
                '@camera_active',
                [ActiveCameraChannelBuilder(animation_builder, bl_markers)])
            self.actions.append(active_camera_action)

            active_camera_track = TrackBuilder(
                '@camera_active',
                [('@camera_active',
                  0, self.sequence_duration,
                  active_camera_action.index,
                  0, self.sequence_duration)])
            self.tracks.append(active_camera_track)

    def build(self, builder):
        action_offsets = [action_builder.build(builder) for action_builder in self.actions]
        SceneSequenceStartActionsVector(builder, len(action_offsets))
        for animation_action in reversed(action_offsets):
            builder.PrependUOffsetTRelative(animation_action)
        actions_offset = builder.EndVector(len(action_offsets))

        track_offsets = [track_builder.build(builder) for track_builder in self.tracks]
        SceneSequenceStartTracksVector(builder, len(track_offsets))
        for scene_track in reversed(track_offsets):
            builder.PrependUOffsetTRelative(scene_track)
        tracks_offset = builder.EndVector(len(track_offsets))

        SceneSequenceStart(builder)
        SceneSequenceAddActions(builder, actions_offset)
        SceneSequenceAddTracks(builder, tracks_offset)

        return SceneSequenceEnd(builder)
