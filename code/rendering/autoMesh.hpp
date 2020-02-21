#ifndef __AUTO_MESH_HPP__
#define __AUTO_MESH_HPP__

#include "AutoShaderProgram.hpp"

struct AutoMesh
{
    MeshGPUBuffer buffer;
    DynArr<MeshVao> meshVaos;
};

void print(AutoMesh* m) 
{
    loggf("AutoMesh print!\n");
    loggf("MeshVao count: %d\n", m->meshVaos.size());
    for (MeshVao& mVao : m->meshVaos) {
        loggf("\tvao: %d\n", mVao.vao);
        loggf("\tAttribLoc count: %d\n", mVao.attribLocs.size());
        for (AttribLocation& atrLoc : mVao.attribLocs) {
            loggf("\t\tAttrib: %s\n", toStr(atrLoc.attrib));
            loggf("\t\tLocation: %d\n", atrLoc.location);
        }
    }
}

void init(AutoMesh* mesh, MeshData* meshData, Allocator* alloc)
{
    init(&mesh->buffer, meshData, alloc); 
    mesh->meshVaos.init(alloc, 4);
}

void init(AutoMesh* mesh, int indexCount, void* indexData, 
        int vertexCount, void* data, std::initializer_list<MeshAttrib::ENUM> attribs, Allocator* alloc)
{
    MeshData meshData;
    init(&meshData, alloc);
    SCOPE_EXIT(shutdown(&meshData););
    setAttribs(&meshData, vertexCount, data, attribs);
    setIndices(&meshData, indexCount, indexData);

    init(mesh, &meshData, alloc);
}

void shutdown(AutoMesh* mesh)
{
    for (MeshVao& m : mesh->meshVaos) {
        shutdown(&m);
    }
    mesh->meshVaos.shutdown();
    shutdown(&mesh->buffer);
}

bool isCompatible(MeshVao* meshVao, AutoShaderProgram* p)
{
    //loggf("Is compatible: meshVao->id: %d, meshVao->attribLocs.data.data: %p\n", 
    //        meshVao->vao, meshVao->attribLocs.data.data);
    // Check if vao has enough attribs
    if (meshVao->attribLocs.size() < p->attribLocs.size())
        return false;

    // Loop over all shader attributes
    int meshIndex = 0;
    int shaderIndex = 0;
    bool quit = false;
    while (!quit) 
    {
        AttribLocation* meshLoc = &meshVao->attribLocs[meshIndex];
        AttribLocation* shaderLoc = &p->attribLocs[shaderIndex];
        // If it fits, advance both indices
        if (meshLoc->location == shaderLoc->location &&
                meshLoc->attrib == shaderLoc->attrib) {
            meshIndex++;
            shaderIndex++;
        }
        else { 
            // Advance mesh index, because meshes can 
            // have more data then the shader needs
            meshIndex++; 
        }
        if (meshIndex == meshVao->attribLocs.size() ||
                shaderIndex == p->attribLocs.size()) {
            quit = true;
        }
    }

    if (shaderIndex == p->attribLocs.size()) {
        return true;
    }
    return false;
}

void draw(AutoMesh* mesh, AutoShaderProgram* p)
{
    if (p->program.id == 0) {
        return;
    }
    bind(p);

    // Loop through vaos if one fits
    for (MeshVao& meshVao : mesh->meshVaos)
    {
        if (isCompatible(&meshVao, p)) 
        {
            draw(&meshVao, mesh->buffer.indexBuffer.indexCount);
            return;
        }
    }

    // Else create new vao
    int attribLocCount = p->attribLocs.size();
    AttribLocation* attribLocs = (AttribLocation*)p->attribLocs.data.data;

    MeshVao meshVao;
    init(&meshVao, &mesh->buffer, attribLocCount, attribLocs, p->program.alloc);
    mesh->meshVaos.push_back(meshVao);
}






#endif
