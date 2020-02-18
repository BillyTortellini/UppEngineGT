#ifndef __RENDERER_HPP__
#define __RENDERER_HPP__

#include "uppLib.hpp"
#include "../utils/tmpAlloc.hpp"
#include "../fileIO.hpp"
#include "../fileListener.hpp"
#include "../utils/string.hpp"

// Next steps:
//  - Mesh creation in new file
//  - Movement in 3d
//  - Phong shader
//  - renderer set camera
//  - Draw with mesh + transform + shaderProgram 
//  - Use renderer from inside game (Dynamically loaded)
//  - Debug rendering (2D, text, lines...)
//  - Texture creation from shader

//  DONE:
//  -----
//  - ShaderProgram set important uniforms automatically 
//  - ShaderProgram detects attributes and sets them automatically

// Todo:
// -----
// View Matrix              X
// Projection Matrix        X
// Orthographic Matrix      O
// Transform (With Quat)    O

// GLOBALS:
Allocator* renderAlloc;
u32 currentFrame;
extern GameState* gameState;

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

struct Camera3D
{
    vec3 pos;
    vec3 dir;
    mat4 view;
    mat4 projection;
    mat4 vp;
    u32 lastUpdateFrame;
};

void init(Camera3D* cam) {
    cam->projection = projection(0.01f, 100.0, d2r(90), (float) gameState->windowState.width / gameState->windowState.height);
    cam->lastUpdateFrame = currentFrame-1;
    cam->pos = vec3(0);
    cam->dir = vec3(0);
}

Camera3D* camera;

void setCamera(Camera3D* cam) {
    camera = cam;
}

void prepare(Camera3D* cam)
{
    if (cam->lastUpdateFrame == currentFrame)
        return;
    cam->vp = cam->projection * cam->view;
    cam->lastUpdateFrame = currentFrame;
}

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

        // This must always stay last
        COUNT
    };
};

struct AttribInfo
{
    AttribInfo(){};
    AttribInfo(u32 size, GLenum type, GLint count, GLint index)
        :size(size), type(type), count(count), index(index) {};
    u32 size;
    GLenum type;
    GLint count;
    GLint index;
};

AttribInfo attribInfoTable[] = {
    AttribInfo(sizeof(vec2), GL_FLOAT, 2, 0),    
    AttribInfo(sizeof(vec3), GL_FLOAT, 3, 1),    
    AttribInfo(sizeof(vec3), GL_FLOAT, 3, 2),    
    AttribInfo(sizeof(vec2), GL_FLOAT, 2, 3),    
    AttribInfo(sizeof(vec3), GL_FLOAT, 3, 4),    
    AttribInfo(sizeof(vec4), GL_FLOAT, 4, 5),    
};

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
    char buffer[256];
    strcpy(buffer, "ressources/shaders/");
    strcat(buffer, filepath);
    char* source = load_text_file_tmp(buffer);

    return createShaderFromSource(source, shaderType);
}

#define MAX_SHADER_COUNT 6
GLuint createShaderProgram(int fileCount, const char** filenames)
{
    assert(fileCount < MAX_SHADER_COUNT, "CreateShaderProgram called with more than max shaders\n");

    // Compile all shaders
    int shaderCount = fileCount;
    int shaderIDs[MAX_SHADER_COUNT];
    for (int i = 0; i < shaderCount; i++)
    {
        shaderIDs[i] = createShaderFromFile(filenames[i]);
        if (shaderIDs[i] == 0) {
            loggf("Create shader program failed, could not compile file %s", filenames[i]);
            break;
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

        //SCOPE_EXIT_ROLLBACK;
        //char* buffer = (char*) tmpAlloc.alloc(maxLength);
        char buffer[2048];
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

// TODO: Should be able to do hot code reloading X
// TODO: Get attributes from compiled shader 
// TODO: Should be able to set uniforms from const char*
// TODO: Automatic uniform detection?

namespace AutoUniformType
{
    enum ENUM
    {
        MODEL_MATRIX = 0,
        VIEW_MATRIX,
        PROJECTION_MATRIX,
        MVP_MATRIX,
        VP_MATRIX,
        CAMERA_POS,
        TIME,

        COUNT // MUST STAY LAST
    };
}

struct AutoUniform
{
    GLint location;
    AutoUniformType::ENUM type;
};

struct AttribLoc
{
    AttribLoc(){};
    AttribLoc(MeshAttrib::ENUM attrib, GLuint location)
        : attrib(attrib), location(location) {}
    MeshAttrib::ENUM attrib;
    GLuint location;
};

struct ShaderProgram
{
    int id;
    // Hot reloading
    int tokenCount;
    ListenerToken tokens[MAX_SHADER_COUNT];
    const char* filenames[MAX_SHADER_COUNT];
    // Auto uniform 
    int perModelCount;
    int perFrameCount;
    AutoUniform perModelUniforms[AutoUniformType::COUNT];
    AutoUniform perFrameUniforms[AutoUniformType::COUNT];
    // Prepare
    u32 lastUpdateFrame;
    // Automatic Attribs
    int attribLocCount;
    AttribLoc attribLocs[MeshAttrib::COUNT]; // Sorted by attrib
};

void bind(ShaderProgram* p) {
    bindProgram(p->id);
}

struct SupportedAutoUniform
{
    SupportedAutoUniform() {};
    SupportedAutoUniform(const char* name,
            AutoUniformType::ENUM type, 
            GLenum glType, 
            bool perFrame)
        : name(name), type(type), glType(glType), perFrame(perFrame) {};
    const char* name;
    AutoUniformType::ENUM type;
    GLenum glType;
    bool perFrame;
};

SupportedAutoUniform supportedAutoUniforms[] = {
    SupportedAutoUniform("u_model", AutoUniformType::MODEL_MATRIX, GL_FLOAT_MAT4, false), 
    SupportedAutoUniform("u_modelmat", AutoUniformType::MODEL_MATRIX, GL_FLOAT_MAT4, false), 
    SupportedAutoUniform("u_modelmatrix", AutoUniformType::MODEL_MATRIX, GL_FLOAT_MAT4, false), 
    SupportedAutoUniform("u_model", AutoUniformType::MODEL_MATRIX, GL_FLOAT_MAT4, false), 

    SupportedAutoUniform("u_view", AutoUniformType::VIEW_MATRIX, GL_FLOAT_MAT4, true), 
    SupportedAutoUniform("u_v", AutoUniformType::VIEW_MATRIX, GL_FLOAT_MAT4, true), 
    SupportedAutoUniform("u_viewmat", AutoUniformType::VIEW_MATRIX, GL_FLOAT_MAT4, true), 
    SupportedAutoUniform("u_viewmatrix", AutoUniformType::VIEW_MATRIX, GL_FLOAT_MAT4, true), 

    SupportedAutoUniform("u_projection", AutoUniformType::PROJECTION_MATRIX, GL_FLOAT_MAT4, true), 
    SupportedAutoUniform("u_p", AutoUniformType::PROJECTION_MATRIX, GL_FLOAT_MAT4, true), 
    SupportedAutoUniform("u_projectionmat", AutoUniformType::PROJECTION_MATRIX, GL_FLOAT_MAT4, true), 
    SupportedAutoUniform("u_projectionmatrix", AutoUniformType::PROJECTION_MATRIX, GL_FLOAT_MAT4, true), 

    SupportedAutoUniform("u_mvp", AutoUniformType::MVP_MATRIX, GL_FLOAT_MAT4, false), 
    SupportedAutoUniform("u_mvpmat", AutoUniformType::MVP_MATRIX, GL_FLOAT_MAT4, false), 
    SupportedAutoUniform("u_mvpmatrix", AutoUniformType::MVP_MATRIX, GL_FLOAT_MAT4, false), 

    SupportedAutoUniform("u_vp", AutoUniformType::VP_MATRIX, GL_FLOAT_MAT4, true), 
    SupportedAutoUniform("u_vpmat", AutoUniformType::VP_MATRIX, GL_FLOAT_MAT4, true), 
    SupportedAutoUniform("u_vpmatrix", AutoUniformType::VP_MATRIX, GL_FLOAT_MAT4, true), 

    SupportedAutoUniform("u_campos", AutoUniformType::CAMERA_POS, GL_FLOAT_VEC3, true), 
    SupportedAutoUniform("u_cam", AutoUniformType::CAMERA_POS, GL_FLOAT_VEC3, true), 
    SupportedAutoUniform("u_camera", AutoUniformType::CAMERA_POS, GL_FLOAT_VEC3, true), 
    SupportedAutoUniform("u_camerapos", AutoUniformType::CAMERA_POS, GL_FLOAT_VEC3, true), 
    SupportedAutoUniform("u_cameraposition", AutoUniformType::CAMERA_POS, GL_FLOAT_VEC3, true), 

    SupportedAutoUniform("u_time", AutoUniformType::TIME, GL_FLOAT, true),
    SupportedAutoUniform("u_t", AutoUniformType::TIME, GL_FLOAT, true),
    SupportedAutoUniform("u_now", AutoUniformType::TIME, GL_FLOAT, true),
};

void detectAutoUniforms(ShaderProgram* program)
{
    if (program->id == 0) {
        return;
    }

    program->perFrameCount = 0;
    program->perModelCount = 0;

    // Loop over all uniforms
    int uniformCount;
    glGetProgramiv(program->id, GL_ACTIVE_UNIFORMS, &uniformCount);
    for (int i = 0; i < uniformCount; i++) 
    {
        char uniformName[256];
        GLint size;
        GLenum type;
        glGetActiveUniform(program->id, (GLuint) i, 256, NULL, &size, &type, (GLchar*) uniformName);

        if (size != 1) {
            continue;
        }

        GLint uniformLocation = glGetUniformLocation(program->id, uniformName);
        assert(uniformLocation != -1, "glGetUniformLocation failed with string: %s\n", uniformName);

        // Set uniform name to all lower charcters
        toLower(uniformName);

        // Loop over all supported uniforms and check if they match
        int supportedCount = sizeof(supportedAutoUniforms)/sizeof(SupportedAutoUniform);
        for (int j = 0; j < supportedCount; j++)
        { 
            SupportedAutoUniform& sup = supportedAutoUniforms[j];
            if (strcmp(sup.name, uniformName) == 0 && sup.glType == type)
            {
                AutoUniform* uniform;
                if (sup.perFrame) {
                    assert(program->perFrameCount + 1 < AutoUniformType::COUNT, "Max per frame autouniforms reached\n");
                    uniform = program->perFrameUniforms + program->perFrameCount;
                    program->perFrameCount++;
                }
                else {
                    assert(program->perModelCount + 1 < AutoUniformType::COUNT, "Max per model autouniforms reached\n");
                    uniform = program->perModelUniforms + program->perModelCount;
                    program->perModelCount++;
                }

                uniform->type = sup.type;
                uniform->location = uniformLocation;
                loggf("AutoUniform found: %s\n", uniformName);
                break;
            }
        }
    }
}

struct MeshAttribName
{
    MeshAttribName();
    MeshAttribName(const char* name, MeshAttrib::ENUM attrib, GLenum type)
        : name(name), attrib(attrib), type(type) {}
    const char* name;
    MeshAttrib::ENUM attrib;
    GLenum type;
};

// All lowercase because search is case insensitive
MeshAttribName meshAttribNames[] =
{
    // Positions
    MeshAttribName("a_pos", MeshAttrib::POS2, GL_FLOAT_VEC2),
    MeshAttribName("a_pos", MeshAttrib::POS3, GL_FLOAT_VEC3),
    MeshAttribName("a_p", MeshAttrib::POS2, GL_FLOAT_VEC2),
    MeshAttribName("a_p", MeshAttrib::POS3, GL_FLOAT_VEC3),
    MeshAttribName("a_p2", MeshAttrib::POS2, GL_FLOAT_VEC2),
    MeshAttribName("a_p3", MeshAttrib::POS3, GL_FLOAT_VEC3),
    MeshAttribName("a_pos2", MeshAttrib::POS2, GL_FLOAT_VEC2),
    MeshAttribName("a_pos3", MeshAttrib::POS3, GL_FLOAT_VEC3),
    // Normals
    MeshAttribName("a_norm", MeshAttrib::NORMALS, GL_FLOAT_VEC3),
    MeshAttribName("a_normal", MeshAttrib::NORMALS, GL_FLOAT_VEC3),
    MeshAttribName("a_normals", MeshAttrib::NORMALS, GL_FLOAT_VEC3),
    MeshAttribName("a_n", MeshAttrib::NORMALS, GL_FLOAT_VEC3),
    // UVs
    MeshAttribName("a_uv", MeshAttrib::UV, GL_FLOAT_VEC2),
    MeshAttribName("a_uvs", MeshAttrib::UV, GL_FLOAT_VEC2),
    MeshAttribName("a_texcoord", MeshAttrib::UV, GL_FLOAT_VEC2),
    MeshAttribName("a_texcoords", MeshAttrib::UV, GL_FLOAT_VEC2),
    MeshAttribName("a_texturecoordinates", MeshAttrib::UV, GL_FLOAT_VEC2),
    MeshAttribName("a_coords", MeshAttrib::UV, GL_FLOAT_VEC2),
    // Colors
    MeshAttribName("a_colour", MeshAttrib::COLOR3, GL_FLOAT_VEC3),
    MeshAttribName("a_colour", MeshAttrib::COLOR4, GL_FLOAT_VEC4),
    MeshAttribName("a_color", MeshAttrib::COLOR3, GL_FLOAT_VEC3),
    MeshAttribName("a_color", MeshAttrib::COLOR4, GL_FLOAT_VEC4),
    MeshAttribName("a_col", MeshAttrib::COLOR3, GL_FLOAT_VEC3),
    MeshAttribName("a_col", MeshAttrib::COLOR4, GL_FLOAT_VEC4),
    MeshAttribName("a_c", MeshAttrib::COLOR3, GL_FLOAT_VEC3),
    MeshAttribName("a_c", MeshAttrib::COLOR4, GL_FLOAT_VEC4),
};

int attribLocComparator(const void* ap, const void* bp)
{
    AttribLoc& a = *((AttribLoc*) ap);
    AttribLoc& b = *((AttribLoc*) bp);
    if (a.location < b.location) return -1;
    if (a.location == b.location) return 0;
    if (a.location > b.location) return 1;
    return 0;
}

void detectShaderAttribs(ShaderProgram* p)
{
    if (p->id == 0) {
        return;
    }

    p->attribLocCount = 0;

    GLint count;
    glGetProgramiv(p->id, GL_ACTIVE_ATTRIBUTES, &count);
    for(int i = 0; i < count; i++)
    {
        GLchar attribName[256];
        GLint attribSize;
        GLenum attribType;
        GLint attribLocation;
        // Get Infos
        glGetActiveAttrib(p->id, (GLuint)i, 
                256, NULL, &attribSize, &attribType, attribName);
        attribLocation = glGetAttribLocation(p->id, attribName);
        assert(attribLocation != -1, "glGetAttribLocation failed\n");

        toLower(attribName);
        int attribNameCount = sizeof(meshAttribNames)/sizeof(MeshAttribName);
        for (int j = 0; j < attribNameCount; j++)
        {
            MeshAttribName name = meshAttribNames[j];
            AttribInfo info = attribInfoTable[name.attrib];
            if (name.type == attribType && 
                    attribSize == 1 &&
                    strcmp(attribName, name.name) == 0)
            {
                loggf("AttribName found: %s\n", attribName);
                assert(p->attribLocCount < MeshAttrib::COUNT,
                        "attrib count reached\n");
                p->attribLocs[p->attribLocCount++] = 
                    AttribLoc(name.attrib, attribLocation);
                break;
            }
        }
    }

    // Sort attribs
    qsort(p->attribLocs, p->attribLocCount, 
            sizeof(AttribLoc), &attribLocComparator);
}

struct Transform
{
    Transform(){}
    Transform(const vec3& p) : pos(p) {}
    mat4 toModelMat() const {
        return translate(pos);
    }

    vec3 pos;
};

void prepare(ShaderProgram* program)
{
    if (program->id == 0) {
        bindVao(0);
        return;
    }

    bind(program);
    if (program->lastUpdateFrame == currentFrame) 
        return;
    program->lastUpdateFrame = currentFrame;

    // Prepare camera
    if (camera == nullptr) {
        loggf("Prepare Shaderprogram called with camera not set\n");
        return;
    }
    prepare(camera);

    for (int i = 0; i < program->perFrameCount; i++) 
    {
        AutoUniform& u = program->perFrameUniforms[i];
        switch (u.type)
        {
            case AutoUniformType::VIEW_MATRIX:
                glUniformMatrix4fv(u.location, 1, GL_FALSE, (GLfloat*) &camera->view);
                break;
            case AutoUniformType::PROJECTION_MATRIX:
                glUniformMatrix4fv(u.location, 1, GL_FALSE, (GLfloat*) &camera->projection);
                break;
            case AutoUniformType::VP_MATRIX:
                glUniformMatrix4fv(u.location, 1, GL_FALSE, (GLfloat*) &camera->vp);
                break;
            case AutoUniformType::CAMERA_POS:
                glUniform3fv(u.location, 3, (GLfloat*) &camera->pos);
                break;
            case AutoUniformType::TIME:
                glUniform1f(u.location, (GLfloat) gameState->time.now);
                break;
        }
    }
}

void prepare(ShaderProgram* program, const Transform& transform)
{
    if (program->id == 0) {
        bindVao(0);
        return;
    }

    prepare(program);
    mat4 model = transform.toModelMat();
    mat4 mvp = camera->vp * model;
    for (int i = 0; i < program->perModelCount; i++) 
    {
        AutoUniform& u = program->perModelUniforms[i];
        switch (u.type)
        {
            case AutoUniformType::MODEL_MATRIX:
                glUniformMatrix4fv(u.location, 1, GL_FALSE, (GLfloat*) &model);
                break;
            case AutoUniformType::MVP_MATRIX:
                glUniformMatrix4fv(u.location, 1, GL_FALSE, (GLfloat*) &mvp);
                break;
        }
    }
}

void onShaderFileChanged(const char* filename, void* shaderProgram)
{
    ShaderProgram* p = (ShaderProgram*) shaderProgram;
    p->lastUpdateFrame = currentFrame-1;
    loggf("On shader file changed: %s, tokenCount: %d\n", 
            filename, p->tokenCount);

    glDeleteProgram(p->id);
    p->id = createShaderProgram(p->tokenCount, p->filenames);
    detectAutoUniforms(p);
    detectShaderAttribs(p);
}

bool init(ShaderProgram* p, int fileCount, const char** filenames)
{
    assert(fileCount < MAX_SHADER_COUNT, "Max shader count reaached\n");

    p->lastUpdateFrame = currentFrame-1;
    p->id = createShaderProgram(fileCount, filenames);
    // Add file listeners
    for (int i = 0; i < fileCount; i++) {
        p->filenames[i] = filenames[i];
        char buffer[256];
        strcpy(buffer, "ressources/shaders/");
        strcat(buffer, filenames[i]);
        p->tokens[i] = createFileListener(buffer, &onShaderFileChanged, p);
        assert(p->tokens[i] != INVALID_TOKEN, "createFileListener failed!\n");
    }
    p->tokenCount = fileCount;

    detectAutoUniforms(p);
    detectShaderAttribs(p);

    return true;
}

void shutdown(ShaderProgram* p)
{
    glDeleteProgram(p->id);
    for (int i = 0; i < p->tokenCount; i++) {
        deleteFileListener(p->tokens[i]);
    }
}

bool init(ShaderProgram* p, std::initializer_list<const char*> filenames)
{
    const char* nameBuffer[MAX_SHADER_COUNT];
    int i = 0;
    for (const char* filename : filenames) {
        nameBuffer[i] = filename;
        i++;
    }

    return init(p, (u32)filenames.size(), nameBuffer);
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
        stride += attribInfoTable[types[i]].size;
    }

    // Add each type to the mesh
    int offset = 0;
    for (int i = 0; i < attrCount; i++) 
    {
        Blk& attrData = m->data[types[i]];
        int size = attribInfoTable[types[i]].size;
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

struct MeshRef
{
    GLuint vao;
    int attribLocCount;
    AttribLoc attribLocs[MeshAttrib::COUNT];
};

#define MAX_MESH_REFERENCES 8
struct Mesh
{
    // Attribute information
    int attribCount;
    MeshAttrib::ENUM attribs[MeshAttrib::COUNT];
    // Opengl Information
    GLuint ebo;
    GLuint vbo[MeshAttrib::COUNT];
    int indexCount;
    // References
    int referenceCount;
    MeshRef references[MAX_MESH_REFERENCES];
};

void prepare(Mesh* m, ShaderProgram* p) 
{
    if (p->id == 0) {
        bindVao(0);
        return;
    }

    // Loop through references to check if one fits
    int index = -1;
    for (int i = 0; i < m->referenceCount; i++)
    {
        MeshRef* ref = &(m->references[i]);
        if (ref->attribLocCount < p->attribLocCount) {
            continue;
        }
        // Loop over all shader attributes
        int meshIndex = 0;
        int shaderIndex = 0;
        bool quit = false;
        while (!quit)
        {
            AttribLoc* meshLoc = &(ref->attribLocs[meshIndex]);
            AttribLoc* shaderLoc = &(p->attribLocs[shaderIndex]);
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
            if (meshIndex == ref->attribLocCount ||
                    shaderIndex == p->attribLocCount) {
                quit = true;
            }
        }
        if (shaderIndex == p->attribLocCount) {
            index = i;
            break;
        }
    }

    // If no reference fits, create new reference
    if (index == -1)
    {
        assert(m->referenceCount != MAX_MESH_REFERENCES, 
                "Max references reached\n");
        index = m->referenceCount;
        m->referenceCount++;
        MeshRef* ref = &(m->references[index]);

        glGenVertexArrays(1, &(ref->vao));
        assert(ref->vao != 0, "glGenVertexArrays failed!\n");
        bindVao(ref->vao);

        for (int i = 0; i < p->attribLocCount; i++)
        {
            AttribLoc attribLoc = p->attribLocs[i];
            bool found = false;
            for (int j = 0; j < m->attribCount; j++) 
            {
                if (attribLoc.attrib == m->attribs[j]) 
                {
                    // Add attrib to mesh ref
                    assert(ref->attribLocCount != MeshAttrib::COUNT,
                            "Mesh reference contains to many attribs\n");
                    ref->attribLocs[ref->attribLocCount++] = attribLoc;

                    AttribInfo info = attribInfoTable[attribLoc.attrib];
                    glBindBuffer(GL_ARRAY_BUFFER, m->vbo[j]);
                    // Set Attrib pointer
                    glVertexAttribPointer(attribLoc.location, info.count, 
                            info.type, GL_FALSE, info.size, 0);
                    glEnableVertexAttribArray(attribLoc.location);

                    found = true;
                    break;
                }
            }
            if (!found) {
                glDeleteVertexArrays(1, &ref->vao);
                m->referenceCount--;
                invalid_path("Mesh and shaderprogram not compatible!\n");
                return;
            }
        }

        // Bind ebo to vao
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ebo);
    }

    bindVao(m->references[index].vao);
}

void init(Mesh* m, MeshData* meshData) 
{
    m->referenceCount = 0;
    m->ebo = 0;
    m->attribCount = 0;
    for (int i = 0; i < MeshAttrib::COUNT; i++) {
        m->vbo[i] = 0;
    }

    bindVao(0); // So that binding the element buffer does not screw stuff up
    // Create vbos
    for (int i = 0; i < MeshAttrib::COUNT; i++) 
    {
        if (meshData->data[i].data == nullptr) {
            continue;
        }

        // Add new attrib
        assert(m->attribCount < MeshAttrib::COUNT, "init Mesh failed, attr#\n");
        m->attribs[m->attribCount] = (MeshAttrib::ENUM) i;
        GLuint& vbo = m->vbo[m->attribCount]; 
        m->attribCount++;

        // Gen buffer
        glGenBuffers(1, &vbo);
        assert(vbo != 0, "glGenBuffers failed!\n");

        // Copy data
        AttribInfo info = attribInfoTable[i];
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, info.size * meshData->vertexCount, meshData->data[i].data, GL_STATIC_DRAW);
    }

    // Create ebo
    m->indexCount = meshData->indexCount;
    glGenBuffers(1, (GLuint*) &(m->ebo));
    assert(m->ebo != 0, "glGenBuffers failed on ebo\n");

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * meshData->indexCount, (void*) meshData->indexData, GL_STATIC_DRAW);

    // Unbind to make sure nothing messes with our data
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
    for (int i = 0; i < m->referenceCount; i++) {
        glDeleteVertexArrays(1, &(m->references[i].vao));
    }
    glDeleteBuffers(1, &(m->ebo));
}

void draw(ShaderProgram* program, Mesh* mesh)
{
    prepare(mesh, program);
    glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, NULL);
}

void draw(ShaderProgram* program, Mesh* mesh, const Transform& transform)
{
    prepare(program, transform); // Sets programs uniforms
    prepare(mesh, program); // binds mesh and makes sure the location match up
    glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, NULL);
}

void createCubeMesh(Mesh* m)
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
    init(m, 36, elementData, 8, vertexData, {POS3, COLOR3});
}

bool initRenderer(Allocator* alloc)
{
    // Init Renderer
    camera = nullptr;
    currentFrame = 0;
    renderAlloc = alloc;

    // Set Opengl State
    wglSwapIntervalExt(-1);
    glViewport(0, 0, gameState->windowState.width, gameState->windowState.height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    glEnable(GL_CULL_FACE);

    // Set initial openglState
    bindProgram(0);
    bindVao(0);

    return true;
}

void startFrame()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    currentFrame++;
}




#endif
