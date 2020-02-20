#ifndef __MESH_HPP__
#define __MESH_HPP__

// Mesh Attrib + infos
namespace MeshAttrib
{
    enum ENUM
    {
        POS2 = 0,
        POS3 = 1,
        NORMAL = 2,
        UV = 3, 
        COLOR3 = 4,
        COLOR4 = 5,

        // This must always stay last
        COUNT
    };
};

struct MeshAttribInfo
{
    MeshAttribInfo(){};
    MeshAttribInfo(u32 size, GLenum type, GLint count, GLint index)
        :size(size), type(type), count(count), index(index) {};
    u32 size;
    GLenum type;
    GLint count;
    GLint index;
};

const MeshAttribInfo meshAttribInfoTable[] = 
{
    MeshAttribInfo(sizeof(vec2), GL_FLOAT, 2, 0),    
    MeshAttribInfo(sizeof(vec3), GL_FLOAT, 3, 1),    
    MeshAttribInfo(sizeof(vec3), GL_FLOAT, 3, 2),    
    MeshAttribInfo(sizeof(vec2), GL_FLOAT, 2, 3),    
    MeshAttribInfo(sizeof(vec3), GL_FLOAT, 3, 4),    
    MeshAttribInfo(sizeof(vec4), GL_FLOAT, 4, 5),    
};



// MESH DATA
struct AttribBlk
{
    AttribBlk() {}
    AttribBlk(MeshAttrib::ENUM attrib, const Blk& blk)
        : attrib(attrib), blk(blk) {}
    MeshAttrib::ENUM attrib;
    Blk blk;
};

struct MeshData
{
    int vertexCount;
    int indexCount;
    DynArr<AttribBlk> attribBlks;
    Blk indexData;
    Allocator* alloc;
};

void init(MeshData* m, Allocator* alloc) 
{
    memset(m, 0, sizeof(MeshData));
    m->alloc = alloc;
    m->attribBlks.init(alloc, 8);
}

void shutdown (MeshData* m) 
{
    // Dealloc all attribs
    for (AttribBlk& b : m->attribBlks) {
        m->alloc->dealloc(b.blk);
    }
    m->attribBlks.shutdown();
    // Dealloc indices
    if (m->indexData.data != nullptr) {
        m->alloc->dealloc(m->indexData);
    }
}

int findAttribIndex(MeshData* m, MeshAttrib::ENUM attrib)
{
    for (int i = 0; i < m->attribBlks.size(); i++) {
        if (m->attribBlks[i].attrib == attrib) {
            return i;
        }
    }

    return -1;
}

void setAttribs(MeshData* m, int count, void* data, std::initializer_list<MeshAttrib::ENUM> attribs)
{
    // Calculate stride
    u32 stride = 0;
    for (MeshAttrib::ENUM attrib : attribs) {
        stride += meshAttribInfoTable[attrib].size;
    }

    // Put attribs from initializer list into array
    int offset = 0;
    for (MeshAttrib::ENUM attrib : attribs) 
    {
        // Check if attribData already exists
        assert(findAttribIndex(m, attrib) == -1,
                "Attach called with already available mesh data");

        // Alloc data
        int size = meshAttribInfoTable[attrib].size;
        AttribBlk a;
        a.attrib = attrib;
        a.blk = m->alloc->alloc(size * count);

        // Copy data into new buffers vertex by vertex
        byte* to = (byte*) a.blk.data;
        byte* from = (byte*) data;
        for (int i = 0; i < count; i++) {
            memcpy(&(to[i*size]), &(from[i*stride + offset]), size);
        }

        // Add AttribBlk to meshData
        m->attribBlks.push_back(a);

        offset += size;
    }

    // Set vertex count
    if (m->vertexCount == 0) {
        m->vertexCount = count;
    }
    else {
        assert(m->vertexCount == count, 
            "Attach(Mesh*) called with different vertex counts!\n");
    }
}

void setIndices(MeshData* m, int count, void* data)
{
    assert(m->indexCount == 0, "Indices were already set for this mesth\n!");
    m->indexCount = count;
    m->indexData = m->alloc->alloc(sizeof(u32) * count);
    memcpy(m->indexData.data, data, sizeof(u32) * count);
}




// MESH GPU BUFFER
struct AttribGPUBuffer
{
    MeshAttrib::ENUM attrib;
    GLuint vbo;
    int vertexCount;
};

void init(AttribGPUBuffer* a, MeshAttrib::ENUM attrib, 
        void* data, int vertexCount)
{
    // Set members
    a->attrib = attrib;
    a->vertexCount = vertexCount;
    
    // Gen buffer
    glGenBuffers(1, &a->vbo);
    assert(a->vbo != 0, "glGenBuffers failed!\n");

    // Init GPU memory
    MeshAttribInfo info = meshAttribInfoTable[attrib];
    glBindBuffer(GL_ARRAY_BUFFER, a->vbo);
    glBufferData(GL_ARRAY_BUFFER, info.size * vertexCount, 
            data, GL_STATIC_DRAW);
    
    // Unbind to make sure nothing messes with our data
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void shutdown(AttribGPUBuffer* a) {
    glDeleteBuffers(1, &a->vbo);
}

struct IndexGPUBuffer
{
    GLuint ebo;
    int indexCount;
};

void init(IndexGPUBuffer* i, void* data, int indexCount)
{
    // Init members
    i->indexCount = indexCount;

    // Generate buffer
    glGenBuffers(1, &i->ebo);
    assert(i->ebo != 0, "glGenBuffers failed on ebo\n");

    // Copy data to buffer
    bindVao(0); // So that binding the element buffer does not screw stuff up
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, i->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * indexCount, 
            data, GL_STATIC_DRAW);
    
    // Unbind to make sure nothing messes with our data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void shutdown(IndexGPUBuffer* i) {
    glDeleteBuffers(1, &i->ebo);
}

struct MeshGPUBuffer
{
    DynArr<AttribGPUBuffer> attribBuffers;
    IndexGPUBuffer indexBuffer;
};

void init(MeshGPUBuffer* g, MeshData* meshData, Allocator* alloc)
{
    // Safety initializsations
    memset(g, 0, sizeof(MeshGPUBuffer));

    // Init members
    g->attribBuffers.init(alloc, 4);

    // Create vbos
    for (AttribBlk& attribBlk : meshData->attribBlks) 
    {
        AttribGPUBuffer buffer;
        init(&buffer, attribBlk.attrib, attribBlk.blk.data, meshData->vertexCount);
        g->attribBuffers.push_back(buffer);
    }

    // Create ebo
    init(&g->indexBuffer, meshData->indexData.data, meshData->indexCount);
}

void shutdown(MeshGPUBuffer* g) 
{
    for (AttribGPUBuffer& attribBuffer : g->attribBuffers) {
        shutdown(&attribBuffer);
    }
    g->attribBuffers.shutdown();
    shutdown(&g->indexBuffer);
}

int findAttribIndex(MeshGPUBuffer* g, MeshAttrib::ENUM attrib)
{
    for (int i = 0; i < g->attribBuffers.size(); i++) {
        if (g->attribBuffers[i].attrib == attrib) {
            return i;
        }
    }

    return -1;
}




// MESH
struct AttribLocation
{
    AttribLocation() {}
    AttribLocation(MeshAttrib::ENUM attrib, GLuint location)
        :attrib(attrib), location(location) {};
    MeshAttrib::ENUM attrib;
    GLuint location;
};

struct MeshVao
{
    GLuint vao;
    DynArr<AttribLocation> attribLocs; // Always sorted
};



void init(MeshVao* m, MeshGPUBuffer* buffer, 
        int attribLocCount, AttribLocation* attribLocs, Allocator* alloc) 
{
    // Init members
    m->vao = 0;
    m->attribLocs.init(alloc, 4);

    // Create vao
    glGenVertexArrays(1, &m->vao);
    assert(m->vao != 0, "glGenVertexArrays failed!\n");
    bindVao(m->vao);

    for (int i = 0; i < attribLocCount; i++)
    {
        AttribLocation attribLoc = attribLocs[i];
        // Find attrib in buffer 
        int index = findAttribIndex(buffer, attribLoc.attrib);
        assert(index != -1, "Init meshVao called with attrib not in buffer\n");

        // Bind vbo
        GLint vbo = buffer->attribBuffers[index].vbo;
        const MeshAttribInfo& info = meshAttribInfoTable[attribLoc.attrib];
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        // Set Attrib pointer
        glVertexAttribPointer(attribLoc.location, info.count, 
                info.type, GL_FALSE, info.size, 0);
        glEnableVertexAttribArray(attribLoc.location);

        m->attribLocs.push_back(attribLoc);
    }

    // Sort
    auto attribLocationCmp = [](AttribLocation* a, AttribLocation* b) {
        if (a->location < b->location) return -1;
        return 1;
    };
    m->attribLocs.sort(attribLocationCmp);
    
    // Bind element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->indexBuffer.ebo);

    // Unbind so nobody messes with our stuff
    bindVao(0);
}

void init(MeshVao* m, MeshGPUBuffer* buffer, 
        std::initializer_list<AttribLocation> locations, Allocator* alloc) 
{
    SCOPE_EXIT_ROLLBACK;
    int attribLocCount = (int)locations.size();
    AttribLocation* attribLocs = (AttribLocation*)
        tmpAlloc.alloc(sizeof(AttribLocation) * locations.size());
    int i = 0;
    for (AttribLocation loc : locations) {
        attribLocs[i] = loc;
    }

    init(m, buffer, attribLocCount, attribLocs, alloc);
}


void shutdown(MeshVao* mesh) {
    glDeleteVertexArrays(1, &mesh->vao);
    mesh->attribLocs.shutdown();
}

void bind(MeshVao* m) {
    bindVao(m->vao);
}

void draw(MeshVao* v, int indexCount) {
    bind(v);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void*)0);
}



struct Mesh
{
    MeshGPUBuffer buffer;
    MeshVao meshVao;
};

void init(Mesh* m, MeshData* meshData, 
        std::initializer_list<AttribLocation> locations, Allocator* alloc) 
{
    init(&m->buffer, meshData, alloc);
    init(&m->meshVao, &m->buffer, locations, alloc);
}

void shutdown(Mesh* m) {
    shutdown(&m->meshVao);
    shutdown(&m->buffer);
}

void bind(Mesh* m) {
    bind(&m->meshVao);
}

void draw(Mesh* m) {
    draw(&m->meshVao, m->buffer.indexBuffer.indexCount);
}






#endif
