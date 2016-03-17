import logging as log
import bmesh
from io import BytesIO
from struct import Struct, pack
from mathutils import Vector
from ipp.schema.resource.render.mesh import Mesh, VertexBuffer
from ipp.schema.resource.render.vertex import AttributeDefinition
from ipp.schema.resource.render.vertex.ElementType import ElementType
from ipp.schema.resource.render.vertex.AttributeName import AttributeName


class VertexBuilder:
    def __init__(self, normals=True,
                 uv_layers=0, color_layers=0,
                 skinning_groups=False, groups_per_vertex=0, groups_have_weight=False):
        self.normals = normals
        self.uv_layers = uv_layers
        self.color_layers = color_layers
        self.skinning_groups = skinning_groups
        self.groups_per_vertex = groups_per_vertex
        self.groups_have_weight = groups_have_weight

        # build struct format based on vertex definition args
        format_definition = '< 3f'
        if normals:
            format_definition += ' 3f'
        format_definition += ''.join([' 2f'] * uv_layers)
        format_definition += ''.join([' 3B'] * color_layers)
        if skinning_groups:
            format_definition += ' 4B 4f'
        if groups_per_vertex > 0:
            format_definition += ' %sB' % groups_per_vertex
            if groups_have_weight:
                format_definition += ' %sf' % groups_per_vertex

        self.vertex_struct = Struct(format_definition)
        self.vertices = BytesIO()

        self.attributes = [(AttributeName.Position, 0, ElementType.Float, 3, False)]

        if normals:
            self.attributes.append((AttributeName.Normal, 0, ElementType.Float, 3, False))

        for i in range(0, uv_layers):
            self.attributes.append((AttributeName.Uv, i, ElementType.Float, 2, False))
        for i in range(0, color_layers):
            self.attributes.append((AttributeName.Color, i, ElementType.UByte, 3, True))

        if skinning_groups:
            self.attributes.append((AttributeName.SkinningIndices, 0, ElementType.UByte, 4, False))
            self.attributes.append((AttributeName.SkinningWeights, 0, ElementType.Float, 4, False))

        if groups_per_vertex > 0:
            self.attributes.append((
                AttributeName.GroupIndices,
                0,
                ElementType.UByte,
                groups_per_vertex,
                False))

        if groups_have_weight:
            self.attributes.append((
                AttributeName.GroupWeights,
                0,
                ElementType.Float,
                groups_per_vertex,
                False))

    def append_vertex(self, co, *args):
        """
        Pack vertex data according to vertex format and append bytes to vertex buffer.
        """
        # need to convert args to element array according to vertex format
        args_offset = 0
        vertex_data = []

        # normal is a vec3
        if self.normals:
            vertex_data.extend((
                args[args_offset][0],
                args[args_offset][1],
                args[args_offset][2]))
            args_offset += 1

        # each uv layer has u,v tex coord argument
        for i in range(0, self.uv_layers):
            vertex_data.append(args[args_offset][0])
            vertex_data.append(args[args_offset][1])
            args_offset += 1

        # each color layer has r,g,b color argument
        for i in range(0, self.color_layers):
            vertex_data.append(args[args_offset][0])
            vertex_data.append(args[args_offset][1])
            vertex_data.append(args[args_offset][2])
            args_offset += 1

        # skinning groups are passed as array of 4 bone indices and 4 weights
        if self.skinning_groups:
            if len(args[args_offset]) != 4:
                raise ValueError(
                    'Vertex skinning group indices must have lenght 4 but received {}'.format(
                        len(args[args_offset])))
            for skinning_index in args[args_offset]:
                vertex_data.append(skinning_index)
            args_offset += 1
            if len(args[args_offset]) != 4:
                raise ValueError(
                    'Vertex skinning group weights must have lenght 4 but received {}'.format(
                        len(args[args_offset])))
            for skinning_weight in args[args_offset]:
                vertex_data.append(skinning_weight)
            args_offset += 1

        # vertex groups are passed as array of vertex group indices
        if self.groups_per_vertex > 0:
            if self.groups_per_vertex != len(args[args_offset]):
                raise ValueError(
                    'Vertex groups_per_vertex defined as {} but received {}'.format(
                        self.groups_per_vertex, len(args[args_offset])))
            for group_index in args[args_offset]:
                vertex_data.append(group_index)
            args_offset += 1
            # if groups_have_weight is true array of group weights with
            # groups_per_vertex must also be provided
            if self.groups_have_weight:
                if self.groups_per_vertex != len(args[args_offset]):
                    raise ValueError(
                        'Vertex groups_per_vertex %s does not match weihgt_count %s'.format(
                            self.groups_per_vertex, len(args[args_offset])))
                for group_weight in args[args_offset]:
                    vertex_data.append(group_weight)
                args_offset += 1

        # pack using vertex struct
        self.vertices.write(
            self.vertex_struct.pack(
                co[0], co[1], co[2],
                *vertex_data))

    def build(self, builder):
        VertexBuffer.VertexBufferStartAttributesVector(builder, len(self.attributes))
        for attribute in reversed(self.attributes):
            AttributeDefinition.CreateAttributeDefinition(builder, *attribute)
        attributes = builder.EndVector(len(self.attributes))

        data = self.vertices.getbuffer()
        VertexBuffer.VertexBufferStartBufferVector(builder, len(data))
        for b in bytearray(reversed(data)):
            builder.PrependByte(b)
        buffer = builder.EndVector(len(data))

        VertexBuffer.VertexBufferStart(builder)
        VertexBuffer.VertexBufferAddAttributes(builder, attributes)
        VertexBuffer.VertexBufferAddBuffer(builder, buffer)
        return VertexBuffer.VertexBufferEnd(builder)


def compile_mesh(builder, vertex_builder, indices):
    vertices_table = vertex_builder.build(builder)
    indices_buffer = pack("{0}H".format(len(indices)), *indices)

    Mesh.MeshStartIndicesVector(builder, len(indices_buffer))
    for index in reversed(indices_buffer):
        builder.PrependByte(index)
    indices_vector = builder.EndVector(len(indices_buffer))

    Mesh.MeshStart(builder)
    Mesh.MeshAddVertices(builder, vertices_table)
    Mesh.MeshAddIndices(builder, indices_vector)
    Mesh.MeshAddTriangleCount(builder, len(indices) // 3)

    return Mesh.MeshEnd(builder)


def create_triangulated_bmesh(bl_mesh):
    """
    Create a triangulated bmesh from blender 'MESH' object.

    :param bl_mesh: blender Mesh object
    :rtype: bmesh.BMesh
    """
    log.debug("Creating bmesh for blender object %s", bl_mesh.name)
    bl_bmesh = bmesh.new()
    bl_bmesh.from_mesh(bl_mesh)
    bmesh.ops.triangulate(bl_bmesh, faces=bl_bmesh.faces)
    # make sure mesh vertices have valid indexes
    bl_bmesh.verts.ensure_lookup_table()
    bl_bmesh.verts.index_update()
    return bl_bmesh


def export_mesh(builder,
                bl_bmesh,
                normals=True,
                group_skinning_index_map=None,
                uv_layers=(), color_layers=(),
                groups_per_vertex=0,
                groups_index_map=None,
                groups_have_weight=False):
    """
    Process bmesh to build schema mesh data object.

    :param bl_bmesh: triangulated blender bmesh instance to export
    :param group_skinning_index_map: dict of group index -> skinning index for deformation groups used for skinning
    :param normals: bool indicating if vertex normals will be exported
    :param uv_layers: list of UV coord layer name strings
    :param color_layers: list of vertex color layer name strings
    :param groups_per_vertex: number of groups per each vertex exported
    :param groups_index_map: dict of group id -> group index for all vertex groups exported
    :param groups_have_weight: bool, indicates if vertex groups exported should also export weights or just indices
    :rtype: bytearray - mesh binary data
    """
    #
    # Blender stores UV and Color data per-face but we need it per-vertex
    #
    # To convert from per-face data we build a map of all vertices with
    # per-face data, if some vertex has multiple different per-face entries
    # vertex is cloned and the face points to the new vertex with valid data
    #
    log.debug("Extracting bmesh data")

    # convert uv_layers and color_layers from names to bmesh layers
    uv_layers = tuple(bl_bmesh.loops.layers.uv[uv_layer] for uv_layer in uv_layers)
    color_layers = tuple(bl_bmesh.loops.layers.color[color_layer] for color_layer in color_layers)

    # list of triangle faces
    indices = []

    # list of vertices uv and color data
    verts = [None] * len(bl_bmesh.verts)
    # map of original vert index -> duplicate vert list
    verts_dups = {}

    # loop trough all faces and face loops
    for face in bl_bmesh.faces:
        for loop in face.loops:
            uvs = tuple(loop[uv_layer].uv for uv_layer in uv_layers)
            colors = tuple(loop[color_layer] for color_layer in color_layers)
            colors = tuple(
                (int(c.r * 255), int(c.g * 255), int(c.b * 255))
                for c in colors)
            vert_index = loop.vert.index
            vert = (vert_index, uvs, colors)

            # update vert_index to point to a correct face vertex
            if verts[vert_index] is None:
                # if vert_index doesn't have a value assigned assign this vert
                verts[vert_index] = vert
            elif verts[vert_index] != vert:
                # when vert at vert_index doesn't match need to use a duplicate
                # search existing duplicates to see if they have a match
                if vert_index not in verts_dups:
                    verts_dups[vert_index] = []
                vert_dups = verts_dups[vert_index]
                vert_index = -1
                for dup_index in vert_dups:
                    if verts[dup_index] == vert:
                        # use existing duplicate vert
                        vert_index = dup_index
                        break
                if vert_index == -1:
                    # no matching duplicate found, create new duplicate
                    vert_index = len(verts)
                    vert_dups.append(vert_index)
                    verts.append(vert)

            # append face index with correct vertex
            indices.append(vert_index)

    log.info("Added %s vertices in converting bmesh UVs/colors from per-face to per-vertex",
             len(verts) - len(bl_bmesh.verts))

    # build vertices in to vertex data
    vertex_builder = VertexBuilder(
        normals,
        len(uv_layers), len(color_layers),
        group_skinning_index_map is not None,
        groups_per_vertex, groups_have_weight)

    for index, uvs, colors in verts:
        vert = bl_bmesh.verts[index]
        vert_data = [vert.co]

        if normals:
            vert_data.append(vert.normal)

        if uvs:
            vert_data.extend(uvs)

        if colors:
            vert_data.extend(colors)

        groups_layer = bl_bmesh.verts.layers.deform.active
        if group_skinning_index_map is not None or groups_per_vertex > 0:
            if groups_layer is None:
                raise ValueError("Mesh vertex groups not present but requested !")

        if groups_layer is not None:
            groups = vert[groups_layer].items()

            if group_skinning_index_map is not None:
                # get deformation weights sorted by weight
                skinning_groups = sorted(
                    ((group_skinning_index_map[g], w) for g, w in groups),
                    key=lambda g: g[1],
                    reverse=True)
                # resize skinning_groups to 4 elements by trimming/padding
                skinning_group_count = len(skinning_groups)
                if skinning_group_count > 4:
                    log.info(
                        "Mesh vertex %s has %s skinning groups assigned which is over maximum (4) trimming to match.",
                        vert.index, skinning_group_count)
                    skinning_groups = skinning_groups[0:4]
                else:
                    skinning_groups += ((0, 0),) * (4 - skinning_group_count)
                # get normalized weights
                skinning_indices = tuple(index for index, _ in skinning_groups)

                skinning_weights = Vector((weight for _, weight in skinning_groups))
                skinning_weights.normalize()
                # add skinning indices/weights
                vert_data.extend((skinning_indices, skinning_weights))

            if groups_per_vertex > 0:
                # ignore deformation groups
                if group_skinning_index_map is not None:
                    groups = [(groups_index_map[g], w)
                              for g, w in groups
                              if g in group_skinning_index_map]
                # check that we don't have more groups than groups_per_vertex
                group_count = len(groups)
                if group_count > groups_per_vertex:
                    raise ValueError(
                        "Vertex {} has more groups assigned than groups_per_vertex : {}".format(
                            group_count, groups_per_vertex))
                # add empty groups if not enough groups found
                if group_count < groups_per_vertex:
                    groups += ((0, 0),) * (groups_per_vertex - group_count)
                # add groups to vertex data
                vert_data.append(tuple(index for index, _ in groups))
                if groups_have_weight:
                    vert_data.append(tuple(weight for _, weight in groups))
        vertex_builder.append_vertex(*vert_data)

    log.info("Created mesh vertex data for %s vertices", len(verts))
    return compile_mesh(builder, vertex_builder, indices)
