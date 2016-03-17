from mathutils import Vector
from ipp.schema.primitive.Vec3 import *
from ipp.schema.resource.scene.ComponentKind import *
from ipp.schema.resource.scene.LightKind import *
from ipp.schema.resource.scene.DirectionalLight import *
from ipp.schema.resource.scene.LightComponent import *


class LightBuilder:
    def __init__(self, bl_object):
        bl_light = bl_object.data
        self.cast_shadow = bl_light.cycles.cast_shadow if getattr(bl_light, 'cycles', None) else True
        if bl_light.type == 'SUN':
            self.light_kind = LightKind.DirectionalLight
            self.direction = [0, 0, 1]
            self.color = (0.8, 0.8, 0.8)
            self.ambient_diffuse_intensity = bl_light.get('ambient_diffuse_intensity', 0.2)
        else:
            raise ValueError('Unsupported light type : {}'.format(bl_light.type))

    @property
    def component_kind(self):
        return ComponentKind.LightComponent

    def build(self, builder):
        if self.light_kind == LightKind.DirectionalLight:
            DirectionalLightStart(builder)
            DirectionalLightAddDirection(builder, CreateVec3(builder, *self.direction))
            DirectionalLightAddColor(builder, CreateVec3(builder, *self.color))
            DirectionalLightAddAmbientDiffuseIntensity(builder, self.ambient_diffuse_intensity)
            light_offset = DirectionalLightEnd(builder)
        else:
            assert False

        LightComponentStart(builder)
        LightComponentAddCastShadow(builder, self.cast_shadow)
        LightComponentAddLightType(builder, self.light_kind)
        LightComponentAddLight(builder, light_offset)
        return LightComponentEnd(builder)
