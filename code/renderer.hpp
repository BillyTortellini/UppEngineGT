#ifndef __RENDERER_HPP__
#define __RENDERER_HPP__

#include "uppLib.hpp"
#include "tmpAlloc.hpp"
#include "fileIO.hpp"

// GLOBALS:
Allocator* renderAlloc;

// Todo:
// -----
// View Matrix              X
// Projection Matrix        O
// Orthographic Matrix      O
// Transform (With Quat)    O

//  * Define a mesh
//  * Define a material

struct Camera3D
{
    vec3 pos;
    mat4 viewMatrix;
    mat4 projectionMatrix;
};

namespace MeshAttrib
{
    enum ENUM
    {
        POS2 = 0,
        POS3 = 1,
        NORMALS = 2,
        UV = 3,
        COLOR3 = 4,
        COLOR4 = 5,

        // Further:
        // Multiple UVs?
        // Floats, vec2s or vec3s for per vertex data?
        // For skinning an index array and weights

        // This must always stay last
        COUNT
    };
};

u32 attribSizeTable[] = {
    sizeof(vec2),  //POS2
    sizeof(vec3),  //POS3
    sizeof(vec3),  //NORMALS
    sizeof(vec2),  //UV
    sizeof(vec3),  
    sizeof(vec4)
};                 

GLenum attribTypeTable[] = {
    GL_FLOAT,  //POS2
    GL_FLOAT,  //POS3
    GL_FLOAT,  //NORMALS
    GL_FLOAT,  //UV
    GL_FLOAT,  // COLOR3
    GL_FLOAT   // COLOR4
};

int attribCountTable[] = {
    2,  //POS2
    3,  //POS3
    3,  //NORMALS
    2,  //UV
    3,  // COLOR3
    4   // COLOR4
};

u32 attribIndexTable[] = {
    0,
    1,
    2,
    3,
    4,
    5
};

// TODO: Should be able to do hot code reloading
// TODO: Get attributes from compiled shader
// TODO: Should be able to set uniforms from const char*
// TODO: Automatic uniform detection?
struct ShaderProgram
{
    int id;
};

// Mesh:
// Contains all data for a given surface
// Vertex infos:
//      - Position (2D or 3D)
//      - Normals
//      - UV Coordinates (Maybe multiple)
//      - Vertex Color
//      - Indices

// Necessary features
//  - Check if mesh has all available features for a shader
//  - Build vertex array object from MeshData

GLuint createShaderFromSource(const char* source, GLenum type)
{
    // Create shader id
    GLuint id = glCreateShader(type);
    if (id == 0) {
        invalid_path("glCreateShader failed\n");
    }

    // Compile
    {
        glShaderSource(id, 1, &source, NULL);
        glCompileShader(id);
    }

    // Check if compilation worked
    GLint isCompiled = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLength);

        SCOPE_EXIT_ROLLBACK;
        char* msg = (char*) tmpAlloc.alloc(maxLength+1);

        glGetShaderInfoLog(id, maxLength, &maxLength, msg);
        loggf("Could not compile shader, error msg: \n %s\n", msg);
        glDeleteShader(id);
        id = 0;
    }

    return id;
}

bool endsWith(const char* str, const char* end) {
    int endLen = (int) strlen(end);
    int strLen = (int) strlen(str);
    if (endLen > strLen) return false;
    return strcmp(end, &(str[strLen-endLen])) == 0;
}

GLuint createShaderFromFile(const char* filepath)
{
    // Check extension (Supported extensions are .frag, .vert)
    GLenum shaderType = 0;
    if (endsWith(filepath, ".frag")) {
        shaderType = GL_FRAGMENT_SHADER;
    }
    else if (endsWith(filepath, ".vert")) {
        shaderType = GL_VERTEX_SHADER;
    } 
    else if (endsWith(filepath, ".geom")) {
        shaderType = GL_GEOMETRY_SHADER;
    } 
    else if (endsWith(filepath, ".tese")) {
        shaderType = GL_TESS_EVALUATION_SHADER;
    } 
    else if (endsWith(filepath, ".tesc")) {
        shaderType = GL_TESS_CONTROL_SHADER;
    } 
    else {
        loggf("CreateShaderFromFile: could not get shadertype from filename: %s\n", filepath);
        invalid_path("CreateShaderFromFile");
        return 0;
    }

    // Load shader file
    SCOPE_EXIT_ROLLBACK;
    char* source = load_text_file_tmp(filepath);

    return createShaderFromSource(source, shaderType);
}

GLuint createShaderProgram(std::initializer_list<const char*> filenames)
{
    int shaderCount = (int)filenames.size();
    SCOPE_EXIT_ROLLBACK;
    int* shaderIDs = (int*) tmpAlloc.alloc(sizeof(int) * shaderCount);

    // Create Shaders from all files
    {
        int i = 0;
        for (auto& filename : filenames)
        {
            shaderIDs[i] = createShaderFromFile(filename);
            if (shaderIDs[i] == 0) {
                loggf("Create shader program failed, could not compile file %s", filename);
                break;
            }
            i++;
        }
    }

    // Create program 
    GLuint id = glCreateProgram();
    assert(id != 0, "glCreateProgram failed!");

    // Attach all shaders
    for (int i = 0; i < shaderCount; i++) {
        glAttachShader(id, shaderIDs[i]);
    }

    // Link shaders to program and check if errors occured
    glLinkProgram(id);
    int isLinked = 0;
    glGetProgramiv(id, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(id, GL_INFO_LOG_LENGTH, &maxLength);

        SCOPE_EXIT_ROLLBACK;
        char* buffer = (char*) tmpAlloc.alloc(maxLength);
        glGetProgramInfoLog(id, maxLength, &maxLength, buffer);
        loggf("Could not link program, error msg: \n %s\n", buffer);
    }

    // Cleanup shaders
    for (int i = 0; i < shaderCount; i++) {
        glDetachShader(id, shaderIDs[i]);
        glDeleteShader(shaderIDs[i]);
    }

    return id;
}

bool init(ShaderProgram* p, std::initializer_list<const char*> filenames)
{
    p->id = createShaderProgram(filenames);
    return true;
}

void shutdown(ShaderProgram* p)
{
    glDeleteProgram(p->id);
}

void printAllUniforms(ShaderProgram* p)
{
    i32 count;
    glGetProgramiv(p->id, GL_ACTIVE_UNIFORMS, &count);
    for (int i = 0; i < count; i++) 
    {
        char buffer[256];
        GLint size;
        GLenum type;
        glGetActiveUniform(p->id, (GLuint) i, 256, NULL, &size, &type, (GLchar*) buffer);
        loggf("\t Uniform #%d: %s\n", i, buffer);
    }
}

void print(const vec2& v) {
    loggf("{%2.3f, %2.3f}", v.x, v.y);
}

void print(const vec3& v) {
    loggf("{%2.3f, %2.3f, %2.3f}", v.x, v.y, v.z);
}

void print(const vec4& v) {
    loggf("{%2.3f, %2.3f, %2.3f, %2.3f}", v.x, v.y, v.z, v.w);
}

void print(const mat4& m) {
    loggf("(%2.3f, %2.3f, %2.3f, %2.3f)\n", m.columns[0].x, m.columns[1].x, m.columns[2].x, m.columns[3].x);
    loggf("(%2.3f, %2.3f, %2.3f, %2.3f)\n", m.columns[0].y, m.columns[1].y, m.columns[2].y, m.columns[3].y);
    loggf("(%2.3f, %2.3f, %2.3f, %2.3f)\n", m.columns[0].z, m.columns[1].z, m.columns[2].z, m.columns[3].z);
    loggf("(%2.3f, %2.3f, %2.3f, %2.3f)\n", m.columns[0].w, m.columns[1].w, m.columns[2].w, m.columns[3].w);
}

struct MeshData
{
    int vertexCount;
    int indexCount;
    Blk data[MeshAttrib::COUNT];
    Blk indexData;
};

void init(MeshData* m) 
{
    m->vertexCount = 0;
    m->indexCount = 0;
    m->indexData.data = nullptr;
    m->indexData.size = 0;
    for (int i = 0; i < MeshAttrib::COUNT; i++) {
        m->data[i].data = nullptr;
        m->data[i].size = 0;
    }
}

void shutdown (MeshData* m) 
{
    for (int i = 0; i < MeshAttrib::COUNT; i++) {
        if (m->data[i].data != nullptr) {
            renderAlloc->dealloc(m->data[i]);
        }
    }
    if (m->indexData.data != nullptr) {
        renderAlloc->dealloc(m->indexData);
    }
}

MeshData createMeshData() {
    MeshData m;
    init(&m);
    return m;
}

void setAttribs(MeshData* m, int count, void* data, std::initializer_list<MeshAttrib::ENUM> attribs)
{
    int attrCount = (int)attribs.size();
    assert(attrCount != 0 && count > 0 && attrCount < MeshAttrib::COUNT, "Attach(Mesh*..) was called wrong!\n");
    // Put attribs from initializer list into array
    MeshAttrib::ENUM types[MeshAttrib::COUNT];
    {
        int i = 0;
        for(MeshAttrib::ENUM type : attribs) 
        {
            types[i] = type;
            i++;
        }
    }

    // Calculate stride
    u32 stride = 0;
    for (int i = 0; i < attrCount; i++) {
        stride += attribSizeTable[types[i]];
    }

    // Add each type to the mesh
    int offset = 0;
    for (int i = 0; i < attrCount; i++) 
    {
        Blk& attrData = m->data[types[i]];
        int size = attribSizeTable[types[i]];
        assert(attrData.data == nullptr, "Attach called with already available mesh data");

        // Alloc data
        attrData = renderAlloc->alloc(size * count);
        byte* to = (byte*) attrData.data;
        byte* from = (byte*) data;

        // Copy all vertex data
        for (int j = 0; j < count; j++) {
            memcpy(&(to[j*size]), &(from[j*stride + offset]), size);
        }
        offset += size;
    }

    // Set vertex count
    if (m->vertexCount == 0) {
        m->vertexCount = count;
    }
    else {
        assert(m->vertexCount == count, "Attach(Mesh*) called with different vertex counts!\n");
    }
}

void setIndices(MeshData* m, int count, void* data)
{
    assert(m->indexCount == 0, "Indices were already set for this mesth\n!");
    m->indexCount = count;
    m->indexData = renderAlloc->alloc(sizeof(u32) * count);
    memcpy(m->indexData.data, data, sizeof(u32) * count);
}

struct OpenGLState
{
    GLuint currentVao;
    GLuint currentProgram;
};

OpenGLState glState;

void bindVao(GLuint vao) {
    if (glState.currentVao != vao) {
        glBindVertexArray(vao);
        glState.currentVao = vao;
    }
}

void bindProgram(GLuint id) {
    if (glState.currentProgram != id) {
        glUseProgram(id);
        glState.currentProgram = id;
    }
}

void bind(ShaderProgram* p) {
    bindProgram(p->id);
}

struct Mesh
{
    GLuint vao;
    GLuint ebo;
    GLuint vbo[MeshAttrib::COUNT];
    int elementCount;
};

void bind(Mesh* m) {
    bindVao(m->vao);
}

void init(Mesh* m, MeshData* meshData) 
{
    m->ebo = 0;
    m->vao = 0;
    for (int i = 0; i < MeshAttrib::COUNT; i++) {
        m->vbo[i] = 0;
    }

    glGenVertexArrays(1, &m->vao);
    assert(m->vao != 0, "glGenVertexArrays failed\n");

    bindVao(m->vao);

    // Create vbos
    for (int i = 0; i < MeshAttrib::COUNT; i++) 
    {
        if (meshData->data[i].data == nullptr) {
            continue;
        }

        glGenBuffers(1, (GLuint*) &(m->vbo[i]));
        assert(m->vbo[i] != 0, "glGenBuffers failed!\n");

        // Copy data
        int size = attribSizeTable[i];
        glBindBuffer(GL_ARRAY_BUFFER, m->vbo[i]);
        glBufferData(GL_ARRAY_BUFFER, size * meshData->vertexCount, meshData->data[i].data, GL_STATIC_DRAW);

        // Set Attrib pointer
        glVertexAttribPointer(attribIndexTable[i], attribCountTable[i], attribTypeTable[i], GL_FALSE, size, 0);
        glEnableVertexAttribArray(attribIndexTable[i]);
    }

    // Create ebo
    glGenBuffers(1, (GLuint*) &(m->ebo));
    assert(m->ebo != 0, "glGenBuffers failed on ebo\n");

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * meshData->indexCount, (void*) meshData->indexData, GL_STATIC_DRAW);

    bindVao(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void init(Mesh* m, int indexCount, void* indexData, int vertexCount, void* vertexData, std::initializer_list<MeshAttrib::ENUM> types)
{
    // Initialize mesh data
    MeshData meshData = createMeshData();
    SCOPE_EXIT(shutdown(&meshData));
    setAttribs(&meshData, vertexCount, vertexData, types); 
    setIndices(&meshData, indexCount, indexData);
    init(m, &meshData);
}

void shutdown(Mesh* m) 
{
    for (int i = 0; i < MeshAttrib::COUNT; i++) {
        if (m->vbo[i] != 0) {
            glDeleteBuffers(1, &(m->vbo[i]));
        }
    }
    glDeleteBuffers(1, &(m->ebo));
    glDeleteVertexArrays(1, &(m->vao));
}

int vbo, vao, ebo;
ShaderProgram s;
Mesh cubeMesh;
Camera3D camera;
extern GameState gameState;
bool initRenderer(Allocator* alloc)
{
    renderAlloc = alloc;

    // Set initial openglState
    bindProgram(0);
    bindVao(0);

    init(&s, {"ressources/shaders/color.frag", "ressources/shaders/color.vert"});
    printAllUniforms(&s);

    camera.projectionMatrix = projection(0.01f, 100.0, d2r(90), (float) gameState.windowState.width / gameState.windowState.height);

    // Set Opengl State
    wglSwapIntervalExt(-1);
    glViewport(0, 0, gameState.windowState.width, gameState.windowState.height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);

    // Create mesh data
    {   
        struct Vertex
        {
            Vertex(){};
            Vertex(vec3 pos, vec3 color) : pos(pos), color(color){}
            vec3 pos;
            vec3 color;
        };
        // Fill vbo
        Vertex vertexData[] = {
            Vertex(vec3(-1.0f, -1.0f, -1.0f), vec3(1.0f, 0.0f, 0.0f)),
            Vertex(vec3( 1.0f, -1.0f, -1.0f), vec3(1.0f, 0.0f, 0.0f)),
            Vertex(vec3(-1.0f,  1.0f, -1.0f), vec3(1.0f, 0.0f, 0.0f)),
            Vertex(vec3( 1.0f,  1.0f, -1.0f), vec3(1.0f, 0.0f, 0.0f)),
            Vertex(vec3(-1.0f, -1.0f,  1.0f), vec3(1.0f, 0.0f, 0.0f)),
            Vertex(vec3( 1.0f, -1.0f,  1.0f), vec3(1.0f, 0.0f, 0.0f)),
            Vertex(vec3(-1.0f,  1.0f,  1.0f), vec3(1.0f, 0.0f, 0.0f)),
            Vertex(vec3( 1.0f,  1.0f,  1.0f), vec3(1.0f, 0.0f, 0.0f))
        };
        u32 elementData[] = 
        {
            0, 1, 4, 1, 5, 4,
            6, 3, 2, 6, 7, 3,
            4, 7, 6, 4, 5, 7, 
            5, 1, 3, 3, 7, 5,
            0, 3, 1, 0, 2, 3,
            4, 6, 2, 4, 2, 0
        };
        using namespace MeshAttrib;
        init(&cubeMesh, 36, elementData, 8, vertexData, {POS3, COLOR3});
    }

    return true;
}

vec2 camSphere(0.0f);
void render()
{
    if (gameState.windowState.wasResized) {
        glViewport(0, 0, gameState.windowState.width, gameState.windowState.height);
        camera.projectionMatrix = projection(0.01f, 100.0f, d2r(90), (float)gameState.windowState.width/gameState.windowState.height);
    }

    // Update camera
    {
        float sensitivity = 0.003f;
        camSphere += vec2(gameState.input.deltaX, gameState.input.deltaY) * sensitivity;
        camSphere = sphericalNorm(camSphere);
        //camSphere = vec2(d2r(100.0f*(float)gameState.time.now), sinf((float)gameState.time.now) * d2r(30));

        vec3 offset(1.5f, 0, 0);
        vec3 pos = sp2eu(camSphere) * -3.0f + offset;
        camera.viewMatrix = lookAt(pos, offset);
    }

    // RENDER
    glClear(GL_COLOR_BUFFER_BIT);

    bind(&s);
    bind(&cubeMesh);

    // Set Uniforms
    mat4 vp = camera.projectionMatrix * camera.viewMatrix;
    glUniformMatrix4fv(0, 1, GL_FALSE, (GLfloat*) &vp);

    // 3 Cubes
    vec3 p = vec3(0.0f);
    glUniform3fv(1, 1, (GLfloat*) &p);
    glDrawElements(GL_TRIANGLES, 6*2*3, GL_UNSIGNED_INT, NULL);

    p = vec3(1.0f, 0.0f, 0.0f);
    glUniform3fv(1, 1, (GLfloat*) &p);
    glDrawElements(GL_TRIANGLES, 6*2*3, GL_UNSIGNED_INT, NULL);

    p = vec3(-1.0f, 0.0f, 0.0f);
    glUniform3fv(1, 1, (GLfloat*) &p);
    glDrawElements(GL_TRIANGLES, 6*2*3, GL_UNSIGNED_INT, NULL);

    SwapBuffers(deviceContext);
    //glFinish();
}





#endif
