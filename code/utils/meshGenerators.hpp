#ifndef __MESH_GENERATORS_HPP__
#define __MESH_GENERATORS_HPP__

#include "../rendering/renderer.hpp"

void createPlaneMeshData(MeshData* m, Allocator* alloc) 
{
    struct Vertex
    {
        Vertex(vec3 pos, vec3 normal, vec2 uv) : pos(pos), normal(normal), uv(uv){}
        vec3 pos;
        vec3 normal;
        vec2 uv;
    };

    Vertex vertexData[] = {
        Vertex(vec3(-1, 0, -1), vec3(0, 1, 0), vec2(0, 0)),
        Vertex(vec3(-1, 0, 1), vec3(0, 1, 0), vec2(0, 1)),
        Vertex(vec3(1, 0, 1), vec3(0, 1, 0), vec2(1, 1)),
        Vertex(vec3(1, 0, -1), vec3(0, 1, 0), vec2(1, 0))
    };
    u32 indexData[] = {
        0, 1, 2,
        0, 2, 3
    };

    using namespace MeshAttrib;
    init(m, alloc);
    setAttribs(m, 4, vertexData, {POS3, NORMAL, UV});
    setIndices(m, 6, indexData);
}

void createPlaneMesh(AutoMesh* m, Allocator* alloc) 
{
    MeshData planeData;
    createPlaneMeshData(&planeData, alloc);
    SCOPE_EXIT(shutdown(&planeData););

    init(m, &planeData, alloc);
}

void createCubeMeshData(MeshData* m, Allocator* alloc)
{
    struct Vertex
    {
        Vertex(){};
        Vertex(vec3 pos, vec3 normal, vec2 uv) : pos(pos), normal(normal), uv(uv){}
        vec3 pos;
        vec3 normal;
        vec2 uv;
    };
    // Fill vbo
    Vertex vertexData[] = {
        // Front face
        Vertex(vec3(-1.0f, -1.0f, 1.0f), vec3(0.0f, 0.0f, 1.0f), vec2(0, 0)),
        Vertex(vec3( 1.0f, -1.0f, 1.0f), vec3(0.0f, 0.0f, 1.0f), vec2(1, 0)),
        Vertex(vec3( 1.0f,  1.0f, 1.0f), vec3(0.0f, 0.0f, 1.0f), vec2(1, 1)),
        Vertex(vec3(-1.0f,  1.0f, 1.0f), vec3(0.0f, 0.0f, 1.0f), vec2(0, 1)),
        // Back face
        Vertex(vec3(-1.0f, -1.0f, -1.0f), vec3(0.0f, 0.0f, -1.0f), vec2(0, 0)),
        Vertex(vec3( 1.0f, -1.0f, -1.0f), vec3(0.0f, 0.0f, -1.0f), vec2(1, 0)),
        Vertex(vec3( 1.0f,  1.0f, -1.0f), vec3(0.0f, 0.0f, -1.0f), vec2(1, 1)),
        Vertex(vec3(-1.0f,  1.0f, -1.0f), vec3(0.0f, 0.0f, -1.0f), vec2(0, 1)),
        // Right face
        Vertex(vec3(1.0f, -1.0f,  1.0f), vec3(1.0f, 0.0f, 0.0f), vec2(0, 0)),
        Vertex(vec3(1.0f, -1.0f, -1.0f), vec3(1.0f, 0.0f, 0.0f), vec2(1, 0)),
        Vertex(vec3(1.0f,  1.0f, -1.0f), vec3(1.0f, 0.0f, 0.0f), vec2(1, 1)),
        Vertex(vec3(1.0f,  1.0f,  1.0f), vec3(1.0f, 0.0f, 0.0f), vec2(0, 1)),
        // Left face
        Vertex(vec3(-1.0f, -1.0f,  1.0f), vec3(-1.0f, 0.0f, 0.0f), vec2(0, 0)),
        Vertex(vec3(-1.0f, -1.0f, -1.0f), vec3(-1.0f, 0.0f, 0.0f), vec2(1, 0)),
        Vertex(vec3(-1.0f,  1.0f, -1.0f), vec3(-1.0f, 0.0f, 0.0f), vec2(1, 1)),
        Vertex(vec3(-1.0f,  1.0f,  1.0f), vec3(-1.0f, 0.0f, 0.0f), vec2(0, 1)),
        // Top face
        Vertex(vec3(-1.0f, 1.0f,  1.0f), vec3(0.0f, 1.0f, 0.0f), vec2(0, 0)),
        Vertex(vec3( 1.0f, 1.0f,  1.0f), vec3(0.0f, 1.0f, 0.0f), vec2(1, 0)),
        Vertex(vec3( 1.0f, 1.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f), vec2(1, 1)),
        Vertex(vec3(-1.0f, 1.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f), vec2(0, 1)),
        // Bottom face
        Vertex(vec3(-1.0f, -1.0f,  1.0f), vec3(0.0f, -1.0f, 0.0f), vec2(0, 0)),
        Vertex(vec3( 1.0f, -1.0f,  1.0f), vec3(0.0f, -1.0f, 0.0f), vec2(1, 0)),
        Vertex(vec3( 1.0f, -1.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f), vec2(1, 1)),
        Vertex(vec3(-1.0f, -1.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f), vec2(0, 1)),
    };
    u32 indexData[] = 
    {
        // Front
        0, 1, 2, 0, 2, 3,
        // Back
        4, 6, 5, 4, 7, 6,
        // Rigth
        8, 9, 10, 8, 10, 11,
        // Left
        12, 14, 13, 12, 15, 14,
        // Top
        16, 17, 18, 16, 18, 19,
        // Bottom
        20, 22, 21, 20, 23, 22
    };

    using namespace MeshAttrib;
    init(m, alloc);
    setAttribs(m, sizeof(vertexData)/sizeof(Vertex), vertexData, {POS3, NORMAL, UV});
    setIndices(m, sizeof(indexData)/sizeof(u32), indexData);
}

void createCubeMesh(AutoMesh* m, Allocator* alloc) 
{
    MeshData cubeMeshData;
    createCubeMeshData(&cubeMeshData, alloc);
    SCOPE_EXIT(shutdown(&cubeMeshData););

    init(m, &cubeMeshData, alloc);
}

void createPlane2DMeshData(MeshData* m, Allocator* alloc) 
{
    vec2 vertexData[] = {
        vec2(-1, -1),
        vec2(1, -1),
        vec2(1, 1),
        vec2(-1, 1),
    };
    u32 indexData[] = {
        0, 1, 2,
        0, 2, 3
    };

    using namespace MeshAttrib;
    init(m, alloc);
    setAttribs(m, 4, vertexData, {POS2});
    setIndices(m, 6, indexData);
}

void createQuadMesh(AutoMesh* m, Allocator* alloc) 
{
    MeshData plane2DData;
    createPlane2DMeshData(&plane2DData, alloc);
    SCOPE_EXIT(shutdown(&plane2DData););

    init(m, &plane2DData, alloc);
}

















#endif
