#ifndef __SHADERPROGRAM_HPP__
#define __SHADERPROGRAM_HPP__

// Includes
#include "../utils/string_utils.hpp"

// OpenGL untils
GLuint createShaderFromSource(const char* source, GLenum type)
{
    // Create shader id
    GLuint id = glCreateShader(type);
    assert(id != 0, "glCreateShader failed!\n");

    // Compile
    glShaderSource(id, 1, &source, NULL);
    glCompileShader(id);

    // Check if compilation worked
    GLint isCompiled = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLength);

        TmpStr errorMsg(maxLength);
        glGetShaderInfoLog(id, maxLength, &maxLength, (char*)errorMsg);
        loggf("ERROR COMPILING SHADER:\n");
        loggf("Could not compile shader, error msg: \n %s\n", errorMsg.c_str());
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
        loggf("CreateShaderFromFile: could not get shadertype from filepath: %s\n", filepath);
        invalid_path("CreateShaderFromFile");
        return 0;
    }

    // Load shader file
    char* source = load_text_file_tmp(filepath);

    GLuint result = createShaderFromSource(source, shaderType);
    if (result == 0) {
        loggf("COMPILE SHADER FROM FILE: \"%s\" FAILED!\n", filepath);
    }
    return result;
}

#define MAX_SHADER_COUNT 6 
GLuint createShaderProgram(int fileCount, const char** filepaths)
{
    assert(fileCount < MAX_SHADER_COUNT, "CreateShaderProgram called with more than max shaders\n");

    // Create program
    GLuint id = glCreateProgram();
    assert(id != 0, "glCreateProgram failed\n");

    // Compile all shaders
    int shaderCount = fileCount;
    int shaderIDs[MAX_SHADER_COUNT];
    for (int i = 0; i < shaderCount; i++)
    {
        shaderIDs[i] = createShaderFromFile(filepaths[i]);
        if (shaderIDs[i] == 0) {
            loggf("Create shader program failed, could not compile file %s", filepaths[i]);
            break;
        }
    }

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

        TmpStr errorMsg(maxLength);
        glGetProgramInfoLog(id, maxLength, &maxLength, (GLchar*) errorMsg.c_str());
        loggf("PROGRAM LINKING FAILED!\n");
        loggf("Could not link program, error msg: \n %s\n", errorMsg.c_str());
    }

    // Cleanup shaders
    for (int i = 0; i < shaderCount; i++) {
        glDetachShader(id, shaderIDs[i]);
        glDeleteShader(shaderIDs[i]);
    }

    return id;
}


// SHADER PROGRAM
struct AttribInfo
{
    GLuint location;
    char* name;
    GLenum type;
    GLint size;
    Blk nameBlk;
};

struct UniformInfo
{
    GLint location;
    char* name;
    GLenum type;
    GLint size;
    Blk nameBlk;
};

struct ShaderProgram; // Forward declaration
typedef void (*ShaderProgramReloadCallback)(ShaderProgram*);
struct ShaderProgram
{
    int id;
    // Hot reloading
    DynArr<ListenerToken> tokens;
    DynArr<ShaderProgramReloadCallback> reloadCallbacks;
    DynArr<String> filepaths;
    // Infos
    DynArr<UniformInfo> uniformInfos;
    DynArr<AttribInfo> attribInfos; // Always sorted by location
    // For infos
    Allocator* alloc;
};

void print(ShaderProgram* p)
{
    loggf("Program id: \t%d\n", p->id);
    loggf("Listener tokens: (size %d)\n", p->tokens.size());
    for (ListenerToken& t : p->tokens) {
        loggf("\t%d\n", t);
    }
    loggf("Filepaths: (size %d)\n", p->filepaths.size());
    for (String& str : p->filepaths) {
        loggf("\t%s\n", str.c_str());
    }
    loggf("UniformInfos: (size %d)\n", p->uniformInfos.size());
    int i = 0;
    for (UniformInfo& info : p->uniformInfos) {
        loggf("  #%d\n", i++);
        loggf("\tname: %s\n", info.name);
        loggf("\tlocation: %d\n", info.location);
        loggf("\ttype: %d\n", info.type);
        loggf("\tsize: %d\n", info.size);
    }
    loggf("AttribInfos: (size %d)\n", p->attribInfos.size());
    i = 0;
    for (AttribInfo& info : p->attribInfos) {
        loggf("  #%d\n", i++);
        loggf("\tname: %s\n", info.name);
        loggf("\tlocation: %d\n", info.location);
        loggf("\ttype: %d\n", info.type);
        loggf("\tsize: %d\n", info.size);
    }
}

void loadAttribInfos(ShaderProgram* p)
{
    p->attribInfos.reset();

    // Get attrib count
    GLint attribCount;
    glGetProgramiv(p->id, GL_ACTIVE_ATTRIBUTES, &attribCount);
    p->attribInfos.reserve(attribCount);

    // Loop over all attribs
    for(int i = 0; i < attribCount; i++)
    {
        AttribInfo info;
        GLchar nameBuffer[256];

        // Get Infos
        glGetActiveAttrib(p->id, (GLuint)i, 
                256, NULL, &info.size, &info.type, nameBuffer);
        info.location = glGetAttribLocation(p->id, nameBuffer);
        assert(info.location != -1, "glGetAttribLocation failed\n");

        // Copy attribute name
        info.nameBlk = p->alloc->alloc(strlen(nameBuffer)+1);
        info.name = (char*) info.nameBlk;
        strcpy(info.name, nameBuffer);

        // Put attribute info in dynamic array
        p->attribInfos.push_back(info);
    }

    // Sort attribs
    auto attribCmp = [](AttribInfo* a, AttribInfo* b) {
        if (a->location < b->location) return -1;
        return 1;
    };
    p->attribInfos.sort(attribCmp);
}

void loadUniformInfos(ShaderProgram* p)
{
    // Reset for safety
    p->uniformInfos.reset();

    // Get uniform count
    GLint uniformCount;
    glGetProgramiv(p->id, GL_ACTIVE_UNIFORMS, &uniformCount);
    if (uniformCount == 0) {
        return;
    }
    p->uniformInfos.reserve(uniformCount);

    // Loop over all uniforms
    for (int i = 0; i < uniformCount; i++) 
    {
        UniformInfo info;
        char nameBuffer[256];
        glGetActiveUniform(p->id, (GLuint) i, 256, NULL, 
                &info.size, &info.type, (GLchar*) nameBuffer);

        info.location = glGetUniformLocation(p->id, nameBuffer);
        assert(info.location != -1, 
                "glGetUniformLocation failed with string: %s\n", nameBuffer);

        // Copy uniform name
        info.nameBlk = p->alloc->alloc(strlen(nameBuffer)+1);
        info.name = (char*) info.nameBlk;
        strcpy(info.name, nameBuffer);

        // Put uniform data in dynamic array
        p->uniformInfos.push_back(info);
    }
}

void onShaderFileChanged(const char* filename, void* userData)
{
    ShaderProgram* p = (ShaderProgram*) userData;
    loggf("On shader file changed: %s\n", filename);
    p->uniformInfos.reset();
    p->attribInfos.reset();

    // Recompile program
    glDeleteProgram(p->id);
    p->id = 0;
    // Create 
    SCOPE_EXIT_ROLLBACK;
    char** filepaths = (char**) tmpAlloc.alloc(p->filepaths.size() * sizeof(char**));
    for(int i = 0; i < p->filepaths.size(); i++) {
        filepaths[i] = p->filepaths[i].c_str();
    }
    p->id = createShaderProgram(p->filepaths.size(), (const char**)filepaths);
    
    // Reload program infos
    if (p->id == 0) {
        return;
    }
    loadUniformInfos(p);
    loadAttribInfos(p);

    // Call reload callbacks
    loggf("Custom callback coutn: %d\n", p->reloadCallbacks.size());
    //for (ShaderProgramReloadCallback callback : p->reloadCallbacks) {
    //    callback(p);
    //}
}

bool init(ShaderProgram* p, std::initializer_list<const char*> filenames, 
        Allocator* allocator)
{
    assert(filenames.size() < MAX_SHADER_COUNT, "Max shader count!\n");

    // Init members
    p->tokens.init(allocator, (int)filenames.size());
    p->reloadCallbacks.init(allocator, 0);
    p->uniformInfos.init(allocator, 0);
    p->attribInfos.init(allocator, 0);
    p->filepaths.init(allocator, 0);
    p->alloc = allocator;
    
    // Add file listeners
    for (const char* filename : filenames)
    {
        String filepath;
        filepath.init(p->alloc, "ressources/shaders/");
        filepath.cat(filename);
        p->filepaths.push_back(filepath);
        ListenerToken token = 
            createFileListener(filepath.c_str(), &onShaderFileChanged, p);
        assert(token != INVALID_TOKEN, "createFileListener failed!\n");
        p->tokens.push_back(token);
    }

    // Compile program
    SCOPE_EXIT_ROLLBACK;
    char** filepaths = (char**)tmpAlloc.alloc(p->filepaths.size() * sizeof(char**));
    for(int i = 0; i < p->filepaths.size(); i++) {
        filepaths[i] = p->filepaths[i].c_str();
    }
    p->id = createShaderProgram(p->filepaths.size(), (const char**)filepaths);

    // Load infos if compilation was succesfull
    if (p->id == 0) {
        return false;
    }
    loadUniformInfos(p);
    loadAttribInfos(p);

    return true;
}

void shutdown(ShaderProgram* p)
{
    if (p->id != 0) {
        glDeleteProgram(p->id);
    }
    // Remove file listeners
    for (ListenerToken& t : p->tokens) {
        deleteFileListener(t);
    }
    p->tokens.shutdown();
    // Dealloc filenames
    for (String& str : p->filepaths) {
        str.shutdown();
    }
    p->filepaths.shutdown();
    // Dealloc uniform infos
    for (UniformInfo& info : p->uniformInfos) {
        p->alloc->dealloc(info.nameBlk);       
    }
    p->uniformInfos.shutdown();
    // Dealloc attribute infos
    for (AttribInfo& info : p->attribInfos) {
        p->alloc->dealloc(info.nameBlk);       
    }
    p->attribInfos.shutdown();
    p->reloadCallbacks.shutdown();
}

void bind(ShaderProgram* p) {
    bindProgram(p->id);
}

GLuint getAttribLocation(ShaderProgram* p, const char* name)
{
    TmpStr lower = name;
    toLower((char*)lower);
    for(AttribInfo& info : p->attribInfos)
    {
        TmpStr atrName = info.name;
        toLower((char*)atrName);
        if (strcmp(atrName.c_str(), lower.c_str()) == 0) {
            return info.location;
        }
    }
    loggf("Attrib location of \"%s\" not found.\n", name);
    return 0;
}

UniformInfo* getUniformInfo(ShaderProgram* p, const char* name)
{
    for (UniformInfo& info : p->uniformInfos) {
        if (strcmp(info.name, name) == 0) {
            return &info;
        }
    }

    return nullptr;
}

#define GEN_UNIFORM_SETTER(dataType, glType, setter) \
    void setUniform(ShaderProgram* p, const char* name, dataType t) \
{ \
    bindProgram(p->id); \
    UniformInfo* info = getUniformInfo(p, name); \
    if (info == nullptr) { \
        loggf("Uniform \"%s\" not in shaderprogram\n", name); \
        return; \
    } \
    if (info->type != glType) { \
        loggf("Uniform \"%s\" type did not match\n", name); \
        return; \
    } \
    setter; \
}

GEN_UNIFORM_SETTER(int, GL_INT, glUniform1i(info->location, t));
GEN_UNIFORM_SETTER(u32, GL_UNSIGNED_INT, glUniform1ui(info->location, t));
GEN_UNIFORM_SETTER(float, GL_FLOAT, glUniform1f(info->location, t));
GEN_UNIFORM_SETTER(const vec2&, GL_FLOAT_VEC2, glUniform2fv(info->location, 1, (GLfloat*) &t));
GEN_UNIFORM_SETTER(const vec3&, GL_FLOAT_VEC3, glUniform3fv(info->location, 1, (GLfloat*) &t));
GEN_UNIFORM_SETTER(const vec4&, GL_FLOAT_VEC4, glUniform4fv(info->location, 1, (GLfloat*) &t));
GEN_UNIFORM_SETTER(const mat2&, GL_FLOAT_MAT2, glUniformMatrix2fv(info->location, 1, GL_FALSE, (GLfloat*) &t));
GEN_UNIFORM_SETTER(const mat3&, GL_FLOAT_MAT3, glUniformMatrix3fv(info->location, 1, GL_FALSE, (GLfloat*) &t));
GEN_UNIFORM_SETTER(const mat4&, GL_FLOAT_MAT4, glUniformMatrix4fv(info->location, 1, GL_FALSE, (GLfloat*) &t));

#undef GEN_UNIFORM_SETTER





#endif
