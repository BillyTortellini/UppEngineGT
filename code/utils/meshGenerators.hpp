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
        Vertex(vec3(-1, 0, -1), vec3(0, 1, 0), vec2(-1, -1)),
        Vertex(vec3(-1, 0, 1), vec3(0, 1, 0), vec2(-1, 1)),
        Vertex(vec3(1, 0, 1), vec3(0, 1, 0), vec2(1, 1)),
        Vertex(vec3(1, 0, -1), vec3(0, 1, 0), vec2(1, -1))
    };
    u32 indexData[] = {
        0, 1, 2,
        0, 2, 3
    };

    using namespace MeshAttrib;
    init(m, alloc);
    setAttribs(m, 4, vertexData, {POS3, UV, NORMAL});
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
        Vertex(vec3 pos, vec3 color, vec2 uv) : pos(pos), color(color), uv(uv){}
        vec3 pos;
        vec3 color;
        vec2 uv;
    };
    // Fill vbo
    Vertex vertexData[] = {
        Vertex(vec3(-1.0f, -1.0f, -1.0f), vec3(1.0f, 0.0f, 0.0f), vec2(1, 1)),
        Vertex(vec3( 1.0f, -1.0f, -1.0f), vec3(1.0f, 0.0f, 0.0f), vec2(0, 0)),
        Vertex(vec3(-1.0f,  1.0f, -1.0f), vec3(1.0f, 0.0f, 0.0f), vec2(0, 0)),
        Vertex(vec3( 1.0f,  1.0f, -1.0f), vec3(1.0f, 0.0f, 0.0f), vec2(1, 0)),
        Vertex(vec3(-1.0f, -1.0f,  1.0f), vec3(1.0f, 0.0f, 0.0f), vec2(0, 0)),
        Vertex(vec3( 1.0f, -1.0f,  1.0f), vec3(1.0f, 0.0f, 0.0f), vec2(1, 0)),
        Vertex(vec3(-1.0f,  1.0f,  1.0f), vec3(1.0f, 0.0f, 0.0f), vec2(0, 1)),
        Vertex(vec3( 1.0f,  1.0f,  1.0f), vec3(1.0f, 0.0f, 0.0f), vec2(1, 1))
    };
    u32 indexData[] = 
    {
        0, 1, 4, 1, 5, 4,
        6, 3, 2, 6, 7, 3,
        4, 7, 6, 4, 5, 7, 
        5, 1, 3, 3, 7, 5,
        0, 3, 1, 0, 2, 3,
        4, 6, 2, 4, 2, 0
    };

    using namespace MeshAttrib;
    init(m, alloc);
    setAttribs(m, 8, vertexData, {POS3, COLOR3, UV});
    setIndices(m, 36, indexData);
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
