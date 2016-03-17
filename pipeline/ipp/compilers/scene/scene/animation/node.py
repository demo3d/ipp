from ipp.schema.resource.scene.Channel import *
from ipp.schema.resource.scene.ChannelKind import *
from ipp.schema.resource.scene.BoolKeyFrame import *
from ipp.schema.resource.scene.FloatKeyFrame import *
from ipp.schema.resource.scene.NodeHiddenChannel import *
from ipp.schema.resource.scene.NodeTransformProperty import *
from ipp.schema.resource.scene.NodeTransformChannel import *
from ipp.schema.resource.scene.InterpolationMode import *


class NodeHiddenChannelBuilder:
    def __init__(self, keyframes):
        self.keyframes = keyframes

    def build(self, builder):
        NodeHiddenChannelStartKeyFramesVector(builder, len(self.keyframes))
        for keyframe in reversed(self.keyframes):
            CreateBoolKeyFrame(builder, *keyframe)
        keyframes_vector = builder.EndVector(len(self.keyframes))

        NodeHiddenChannelStart(builder)
        NodeHiddenChannelAddKeyFrames(builder, keyframes_vector)
        channel_data_offset = NodeHiddenChannelEnd(builder)

        ChannelStart(builder)
        ChannelAddChannel(builder, channel_data_offset)
        ChannelAddChannelType(builder, ChannelKind.NodeHiddenChannel)
        return ChannelEnd(builder)


class NodeTransformChannelBuilder:
    def __init__(self, transform_property, keyframes):
        self.transform_property = transform_property
        self.keyframes = keyframes

    def build(self, builder):
        NodeHiddenChannelStartKeyFramesVector(builder, len(self.keyframes))
        for keyframe in reversed(self.keyframes):
            CreateFloatKeyFrame(builder, *keyframe)
        keyframes_vector = builder.EndVector(len(self.keyframes))

        NodeTransformChannelStart(builder)
        NodeTransformChannelAddProperty(builder, self.transform_property)
        NodeTransformChannelAddKeyFrames(builder, keyframes_vector)
        channel_data_offset = NodeTransformChannelEnd(builder)

        ChannelStart(builder)
        ChannelAddChannel(builder, channel_data_offset)
        ChannelAddChannelType(builder, ChannelKind.NodeTransformChannel)
        return ChannelEnd(builder)


data_path_transform_property_map = {
    'location': (
        NodeTransformProperty.TranslationX,
        NodeTransformProperty.TranslationY,
        NodeTransformProperty.TranslationZ),
    'rotation_euler': (
        NodeTransformProperty.RotationEulerX,
        NodeTransformProperty.RotationEulerY,
        NodeTransformProperty.RotationEulerZ),
    'rotation_axis_angle': (
        NodeTransformProperty.RotationAxisAngleX,
        NodeTransformProperty.RotationAxisAngleY,
        NodeTransformProperty.RotationAxisAngleZ,
        NodeTransformProperty.RotationAxisAngleAngle),
    'rotation_quaternion': (
        NodeTransformProperty.RotationQuaternionX,
        NodeTransformProperty.RotationQuaternionY,
        NodeTransformProperty.RotationQuaternionZ,
        NodeTransformProperty.RotationQuaternionW),
    'scale': (
        NodeTransformProperty.ScaleX,
        NodeTransformProperty.ScaleX,
        NodeTransformProperty.ScaleX)}


interpolation_map = {
    'CONSTANT': InterpolationMode.Constant,
    'LINEAR': InterpolationMode.Linear,
    'BEZIER': InterpolationMode.Bezier}
