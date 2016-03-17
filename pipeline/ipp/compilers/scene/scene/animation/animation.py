import logging as log
from ipp.schema.resource.scene.Animation import *
from .entity import EntitySequenceBuilder
from .scene import SceneSequenceBuilder


class AnimationBuilder:
    def __init__(self, scene_builder):
        self.scene_builder = scene_builder
        self.bl_scene = scene_builder.bl_scene
        log.info("Exporting scene %s animation", self.bl_scene.name)

        # scene export FPS used to convert from scene frame to scene time
        self.fps = scene_builder.bl_scene.render.fps

        # scene duration
        self.sequence_duration = self.frames_to_milliseconds(scene_builder.bl_scene.frame_end)

        # entity sequences
        self.entities_sequence = {}
        for entity in scene_builder.entities:
            self.entities_sequence[entity.id] = EntitySequenceBuilder(self, entity)

        # scene sequence
        self.scene_sequence = SceneSequenceBuilder(self)

    def frames_to_milliseconds(self, frame):
        """
        Convert frame count to milliseconds based on scene frames per second.
        """
        return int((frame / self.fps) * 1000)

    def build(self, builder):
        entity_sequence_offsets = [entity_sequence.build(builder) for entity_sequence in self.entities_sequence.values()]
        AnimationStartEntitiySequencesVector(builder, len(entity_sequence_offsets))
        for entity_sequence in reversed(entity_sequence_offsets):
            builder.PrependUOffsetTRelative(entity_sequence)
        entity_sequences_offset = builder.EndVector(len(entity_sequence_offsets))

        scene_sequence_offset = self.scene_sequence.build(builder)

        AnimationStart(builder)
        AnimationAddSceneSequence(builder, scene_sequence_offset)
        AnimationAddEntitiySequences(builder, entity_sequences_offset)
        AnimationAddDuration(builder, self.sequence_duration)

        return AnimationEnd(builder)
