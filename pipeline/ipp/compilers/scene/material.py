from enum import Enum
from ipp.schema.resource.scene.MaterialTexture import *
from ipp.schema.resource.scene.MaterialFloatUniform import *
from ipp.schema.resource.scene.MaterialPropertyKind import *
from ipp.schema.resource.scene.MaterialProperty import *
from ipp.schema.resource.scene.MaterialDefinition import *


def build_material_texture_property(builder, resource_path):
    resource_path_offset = builder.CreateString(resource_path)

    MaterialTextureStart(builder)
    MaterialTextureAddResourcePath(builder, resource_path_offset)
    return MaterialTextureEnd(builder)


def build_material_float_property(builder, float_list, expected_list_length):
    if len(float_list) != expected_list_length:
        raise ValueError(
            "Expected {} floats for float property but got {}".format(
                len(float_list), expected_list_length))

    MaterialFloatUniformStartValueVector(builder, expected_list_length)
    for elem in reversed(float_list):
        builder.PrependFloat32(elem)
    float_list_offset = builder.EndVector(expected_list_length)

    MaterialFloatUniformStart(builder)
    MaterialFloatUniformAddValue(builder, float_list_offset)
    return MaterialFloatUniformEnd(builder)


class PropertyKind(Enum):
    Texture = (MaterialPropertyKind.MaterialTexture, build_material_texture_property)
    Float = (MaterialPropertyKind.MaterialFloatUniform, build_material_float_property, 1)
    FVec2 = (MaterialPropertyKind.MaterialFloatUniform, build_material_float_property, 2)
    FVec3 = (MaterialPropertyKind.MaterialFloatUniform, build_material_float_property, 3)
    FVec4 = (MaterialPropertyKind.MaterialFloatUniform, build_material_float_property, 4)
    Mat4 = (MaterialPropertyKind.MaterialFloatUniform, build_material_float_property, 16)

    def build(self, builder, value):
        return self.value[1](builder, value, *self.value[2:])


class MaterialPropertyBuilder:
    def __init__(self, name, value, kind):
        self.name = name
        self.value = value
        self.kind = kind

    def build(self, builder):
        name_offset = builder.CreateString(self.name)
        value_offset = self.kind.build(builder, self.value)
        MaterialPropertyStart(builder)
        MaterialPropertyAddName(builder, name_offset)
        MaterialPropertyAddValueType(builder, self.kind.value[0])
        MaterialPropertyAddValue(builder, value_offset)
        return MaterialPropertyEnd(builder)


class MaterialDefinitionBuilder:
    def __init__(self, manifest, material_definition):
        self.effect_resource_path = manifest.qualified_resource_path(material_definition['effect_resource'])
        self.transparent = material_definition.get('transparent', False)
        self.shadow_caster = material_definition.get('shadow_caster', not self.transparent)
        self.material_properties = {}
        for property_name, property_definition in material_definition['properties'].items():
            property_kind = property_definition['kind']
            if property_kind == 'texture':
                self.material_properties[property_name] = MaterialPropertyBuilder(
                    property_name,
                    manifest.qualified_resource_path(property_definition['resource']),
                    PropertyKind.Texture)
            elif property_kind == 'float':
                self.material_properties[property_name] = MaterialPropertyBuilder(
                    property_name,
                    [float(property_definition['value']),],
                    PropertyKind.Float)
            elif property_kind == 'vec2':
                self.material_properties[property_name] = MaterialPropertyBuilder(
                    property_name,
                    [float(element) for element in property_definition['value']],
                    PropertyKind.FVec2)
            elif property_kind == 'vec3':
                self.material_properties[property_name] = MaterialPropertyBuilder(
                    property_name,
                    [float(element) for element in property_definition['value']],
                    PropertyKind.FVec3)
            elif property_kind == 'vec4':
                self.material_properties[property_name] = MaterialPropertyBuilder(
                    property_name,
                    [float(element) for element in property_definition['value']],
                    PropertyKind.FVec4)
            elif property_kind == 'mat4':
                self.material_properties[property_name] = MaterialPropertyBuilder(
                    property_name,
                    [float(element) for element in property_definition['value']],
                    PropertyKind.Mat4)

    def build(self, builder):
        effect_resource_path_offset = builder.CreateString(self.effect_resource_path)
        material_property_offsets = [
            property.build(builder)
            for property in self.material_properties.values()]

        MaterialDefinitionStartPropertiesVector(builder, len(material_property_offsets))
        for property_offset in reversed(material_property_offsets):
            builder.PrependUOffsetTRelative(property_offset)
        material_properties_offset = builder.EndVector(len(material_property_offsets))

        MaterialDefinitionStart(builder)
        MaterialDefinitionAddEffectResourcePath(builder, effect_resource_path_offset)
        MaterialDefinitionAddShadowCaster(builder, self.shadow_caster)
        MaterialDefinitionAddTransparent(builder, self.transparent)
        MaterialDefinitionAddProperties(builder, material_properties_offset)
        return MaterialDefinitionEnd(builder)
