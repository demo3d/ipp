import logging as log
import re
import bpy
from ipp.schema.resource.scene.Scene import *
from .animation.animation import AnimationBuilder
from .entity import EntityBuilder


class SceneBuilder:
    def __init__(self, manifest, scene_definition):
        self.scene_definition = scene_definition
        self.bl_scene = bpy.data.scenes[scene_definition['source']['scene']]

        log.info("Exporting scene : %s with object filter : %s and marker filter : %s",
                 self.bl_scene.name,
                 scene_definition.get('object_filter', 'a^'))

        self.object_filter = re.compile(scene_definition.get('object_filter', 'a^'))
        self.resource_map = []
        for resource_definition in scene_definition['resources']:
            pattern = re.compile(resource_definition['pattern'])
            resource_path = manifest.qualified_resource_path(resource_definition['resource'])
            self.resource_map.append((pattern, resource_path))

        self.model_material_map = []
        for material_definition in scene_definition['materials']:
            pattern = re.compile(material_definition['pattern'])
            material_path = manifest.qualified_resource_path(material_definition['resource'])
            self.model_material_map.append((pattern, material_path))

        bl_objects = {bl_object for bl_object in self.bl_scene.objects}

        def remove_descendants(bl_object):
            bl_objects.difference_update(bl_object.children)
            for bl_child in bl_object.children:
                remove_descendants(bl_child)

        for bl_object in self.bl_scene.objects:
            if not self.object_filter.match(bl_object.name):
                continue
            bl_objects.remove(bl_object)
            remove_descendants(bl_object)

        # linked objects can be parented to proxied objects but proxied objects are not members of
        #  scene replaces parents with matching proxy object trough parent_map
        bl_proxy_map = {bl_object.proxy or bl_object: bl_object for bl_object in bl_objects}
        bl_parent_map = {bl_object: bl_proxy_map[bl_object.parent] if bl_object.parent else None
                         for bl_object in bl_objects}

        bl_objects_remaining = {bl_object for bl_object in self.bl_scene.objects
                                if bl_parent_map[bl_object] is not None}
        bl_objects_satisfied = [bl_object for bl_object in self.bl_scene.objects
                                if bl_parent_map[bl_object] is None]

        bl_objects = []
        while len(bl_objects_satisfied) > 0:
            bl_objects.extend(bl_objects_satisfied)
            bl_objects_satisfied.clear()
            for bl_object in bl_objects_remaining:
                if bl_parent_map[bl_object] in bl_objects:
                    bl_objects_satisfied.append(bl_object)
            bl_objects_remaining.difference_update(bl_objects_satisfied)

        if len(bl_objects_remaining) != 0:
            log.warning("Unresolved order dependancies for : %s", bl_objects_remaining)
            bl_objects.extend(bl_objects_remaining)

        self.entities = []
        self.entity_map = {}

        for bl_object in bl_objects:
            entity_builder = EntityBuilder(self, len(self.entities) + 1, bl_object)
            self.entities.append(entity_builder)
            self.entity_map[bl_object] = entity_builder.id
            if bl_object.proxy:
                self.entity_map[bl_object.proxy] = entity_builder.id

        if self.bl_scene.camera:
            frame_current = self.bl_scene.frame_current
            self.bl_scene.frame_set(0)
            self.initial_active_camera = self.entity_map[self.bl_scene.camera]
            self.bl_scene.frame_set(frame_current)
        else:
            self.initial_active_camera = 0

        self.animation_builder = AnimationBuilder(self)

    def resource_path(self, bl_object):
        """
        Return first matching resource path in resource_map for bl_object.name.
        """
        for pattern, path in self.resource_map:
            if pattern.match(bl_object.name):
                return path
        raise ValueError(
            'Object {} resource not found in {}.'.format(
                bl_object.name,
                self.resource_map))

    def material_resource_path(self, bl_object):
        """
        Return a resource path instance from model_material_map that matches bl_object name.
        """
        for pattern, material_resource_path in self.model_material_map:
            if pattern.match(bl_object.name):
                return material_resource_path
        raise ValueError('Model {} material not found.'.format(bl_object.name))

    def build(self, builder):
        # scene entities
        entity_offsets = [entity.build(builder) for entity in self.entities]

        SceneStartEntitiesVector(builder, len(entity_offsets))
        for entity in reversed(entity_offsets):
            builder.PrependUOffsetTRelative(entity)
        entities_offset = builder.EndVector(len(entity_offsets))

        # scene animation
        animation_offset = self.animation_builder.build(builder)

        # scene object
        SceneStart(builder)
        SceneAddEntities(builder, entities_offset)
        SceneAddAnimation(builder, animation_offset)
        SceneAddInitialActiveCamera(builder, self.initial_active_camera)

        return SceneEnd(builder)
