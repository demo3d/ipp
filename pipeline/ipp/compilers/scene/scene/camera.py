import logging as log
from ipp.schema.primitive.Vec2 import CreateVec2
from ipp.schema.resource.scene.ComponentKind import *
from ipp.schema.resource.scene.CameraComponent import *
from ipp.schema.resource.scene.SensorFit import *


class CameraBuilder:
    def __init__(self, bl_object):
        log.info("Exporting camera component from blender object : %s", bl_object.name)
        bl_camera = bl_object.data

        assert bl_camera.type == 'PERSP' or bl_camera.type == 'ORTHO', \
            "Unsupported camera : {} with type : {}".format(bl_object.name, bl_camera.type)

        if bl_camera.type == 'ORTHO':
            pixsize = bl_camera.ortho_scale
        else:
            pixsize = bl_camera.sensor_height if bl_camera.sensor_fit == 'VERTICAL' else bl_camera.sensor_width
            pixsize = (pixsize * bl_camera.clip_start) / bl_camera.lens

        sensor_fit_enum_map = {
            'AUTO': SensorFit.Auto,
            'HORIZONTAL': SensorFit.Horizontal,
            'VERTICAL': SensorFit.Vertical
        }

        self.pixel_size = pixsize
        self.sensor_fit = sensor_fit_enum_map[bl_camera.sensor_fit]
        self.camera_shift = (bl_camera.shift_x, bl_camera.shift_y)
        self.clip_near = bl_camera.clip_start
        self.clip_far = bl_camera.clip_end

    def build(self, builder):
        CameraComponentStart(builder)
        CameraComponentAddPixelSize(builder, self.pixel_size)
        CameraComponentAddSensorFit(builder, self.sensor_fit)
        CameraComponentAddCameraShift(builder, CreateVec2(builder, *self.camera_shift))
        CameraComponentAddClipNear(builder, self.clip_near)
        CameraComponentAddClipFar(builder, self.clip_far)
        return CameraComponentEnd(builder)

    @property
    def component_kind(self):
        return ComponentKind.CameraComponent
