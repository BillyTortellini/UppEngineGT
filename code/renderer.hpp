#ifndef __RENDERER_HPP__
#define __RENDERER_HPP__

#include "umath.hpp"
#include "datatypes.hpp"

// Todo:
//  * Define a mesh
//  * Define a material

struct Camera3D
{
    vec3 pos;
    mat4 viewMatrix;
    mat4 projectionMatrix;
};

enum MESH_ATTRIBUTES
{
    POS_2D_INDEX = 0,
    POS_3D_INDEX = 1,
    NORMALS_INDEX = 2,
    UV_INDEX = 3,
    COLOR_INDEX = 4,

    // This must always stay last
    MESH_ATTRIB_COUNT
};

// TODO: Should be able to do hot code reloading
// TODO: Get attributes from compiled shader
struct ShaderProgram
{
    int id;
    int filenameCount;
    String* filenames;

    int requiredCount;
    MESH_ATTRIBUTES requiredAttributes[MESH_ATTRIB_COUNT];
    int requiredAttribIndices[MESH_ATTRIB_COUNT];
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

struct MeshData
{
    byte* attributes[MESH_ATTRIB_COUNT];
    bool available[MESH_ATTRIB_COUNT];
};

#define MAX_MESH_DATA_COUNT 512
struct Renderer
{
    MeshData meshDatas[MAX_MESH_DATA_COUNT]; 
    // Here will be other shaderprograms,
    // Material data and stuff like this
};

// TODO: This is currently a global, lets see if we will change that in the future
Renderer renderer;

GLuint createShaderFromSource(const char* source, GLenum type)
{
    // Create shader id
    GLuint id = glCreateShader(type);
    if (id == 0) {
        INVALID_CODE_PATH;
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

        char buffer[2048];
        glGetShaderInfoLog(id, 2048, &maxLength, buffer);
        debugPrintf("Could not compile shader, error msg: \n %s\n", buffer);
        debugWaitForConsoleInput();
        glDeleteShader(id);
        id = 0;
    }

    return id;
}

// Creates 
GLuint createShaderFromFile(String* filepath)
{
    // Check extension (Supported extensions are .frag, .vert)
    GLenum shaderType = 0;
    if (endsWith(filepath, ".frag")) {
        shaderType = GL_FRAGMENT_SHADER;
    }
    else if (endsWith(filepath, ".vert")) {
        shaderType = GL_VERTEX_SHADER;
    } 
    else {
        debugPrintf("CreateShaderFromFile: could not get shadertype from filename: %s\n", c_str(filepath));
        debugWaitForConsoleInput();
        return 0;
    }

    // Load shader file
    char* fileContent = load_text_file(c_str(filepath));
    if (fileContent == nullptr) {
        debugPrintf("Could not load shaderfile: %s\n", c_str(filepath));
        debugWaitForConsoleInput();
        continue;
    }
    SCOPE_EXIT(unload_text_file(fileContent));

    return createShaderFromSource(fileContent, shaderType);
}

bool init(ShaderProgram* p, std::initializer_list<const char*> filenames)
{
    // Allocate space for filename count
    int shaderCount = (int)filenames.size();
    p->filenameCount = shaderCount;
    p->filenames = (String*) malloc(sizeof(String) * p->filenameCount);

    // Copy filenames from initializer list
    {
        int j = 0;
        for (const char* filename : filenames) {
            init(p->filenames + j, filename);
            j++;
        }
    }

    // Allocate space for shader ids
    // TODO use some sort of allocator here
    int* shaderIDs = (int*) malloc(sizeof(int) * p->filenameCount);
    SCOPE_EXIT(free(shaderIDs););

    // Create Shaders from all files
    for (int i = 0; i < p->filenameCount; i++)
    {
        String* filename = p->filenames + i;
        shaderIDs[i] = createShaderFromFile(filename);
        if (shaderIDs[i] == 0) {
            debugPrintf("Could not compile shaderfile :%s\n", c_str(filename));
            debugWaitForConsoleInput();
        }
    }

    p->id = glCreateProgram();
    // Attach all shaders
    for (int i = 0; i < shaderCount; i++) {
        glAttachShader(p->id, shaderIDs[i]);
    }

    // Link shaders to program and check if errors occured
    glLinkProgram(p->id);
    int isLinked = 0;
    glGetProgramiv(p->id, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(p->id, GL_INFO_LOG_LENGTH, &maxLength);

        char buffer[2048];
        glGetProgramInfoLog(p->id, 2048, &maxLength, buffer);
        debugPrintf("Could not link program, error msg: \n %s\n", buffer);
        debugWaitForConsoleInput();
    }

    // Cleanup shaders
    for (int i = 0; i < shaderCount; i++) {
        glDetachShader(p->id, shaderIDs[i]);
        glDeleteShader(shaderIDs[i]);
    }

    return true;
}

void shutdown(ShaderProgram* p)
{
    glDeleteProgram(p->id);
    for (int i = 0; i < p->filenameCount; i++) {
        shutdown(p->filenames + i);
    }
    free(p->filenames);
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
        debugPrintf("\t Uniform #%d: %s\n", i, buffer);
    }
}

void print(const vec2& v) {
    debugPrintf("{%2.3f, %2.3f}", v.x, v.y);
}

void print(const vec3& v) {
    debugPrintf("{%2.3f, %2.3f, %2.3f}", v.x, v.y, v.z);
}

void print(const vec4& v) {
    debugPrintf("{%2.3f, %2.3f, %2.3f, %2.3f}", v.x, v.y, v.z, v.w);
}

void print(const mat4& m) {
    debugPrintf("(%2.3f, %2.3f, %2.3f, %2.3f)\n", m.columns[0].x, m.columns[1].x, m.columns[2].x, m.columns[3].x);
    debugPrintf("(%2.3f, %2.3f, %2.3f, %2.3f)\n", m.columns[0].y, m.columns[1].y, m.columns[2].y, m.columns[3].y);
    debugPrintf("(%2.3f, %2.3f, %2.3f, %2.3f)\n", m.columns[0].z, m.columns[1].z, m.columns[2].z, m.columns[3].z);
    debugPrintf("(%2.3f, %2.3f, %2.3f, %2.3f)\n", m.columns[0].w, m.columns[1].w, m.columns[2].w, m.columns[3].w);
}

int vbo, vao, ebo;
ShaderProgram s;
Camera3D camera;
extern GameState gameState;
bool initRenderer()
{
    init(&s, {"ressources/shaders/color.frag", "ressources/shaders/color.vert"});
    printAllUniforms(&s);
    //shutdown(&s);
    camera.projectionMatrix = projection(0.01f, 100.0, d2r(90), (float) gameState.windowState.width / gameState.windowState.height);

    glGenVertexArrays(1, (GLuint*) &vao);
    glBindVertexArray(vao);

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

    glGenBuffers(1, (GLuint*) &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*) 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*) (sizeof(float) * 3));
    glEnableVertexAttribArray(1);

    u32 elementData[] = 
    {
        1, 0, 2, 2, 3, 1,
        6, 5, 4, 6, 7, 5,
        6, 7, 2, 2, 3, 7,
        3, 1, 5, 5, 7, 3,
        5, 1, 0, 4, 5, 0,
        4, 0, 2 ,4, 2, 6
    };
    glGenBuffers(1, (GLuint*) &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elementData), elementData, GL_STATIC_DRAW);

    //wglSwapIntervalExt(1);
    wglSwapIntervalExt(-1);

    // TEST
    camera.viewMatrix = lookAt(vec3(0.0f, 0.0f, 3.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

    vec3 points[] = {
        vec3(0.0f, 0.0f, 0.0f),
        vec3(1.0f, 0.0f, -1.0f),
        vec3(1.0f, -1.0f, 1.0f)
    };

    debugPrintf("View matrix:\n");
    print(camera.viewMatrix);

    debugPrintf("\nProjection matrix:\n");
    print(camera.projectionMatrix);

    debugPrintf("\nVP matrix:\n");
    mat4 vp = camera.projectionMatrix * camera.viewMatrix;
    print(vp);
    debugPrintf("\n");

    debugPrintf("Points:\n");
    for (int i = 0; i < 3; i++) {
        vec3 p = points[i];
        debugPrintf("\t");
        print(p);
        debugPrintf("\n");
    }

    debugPrintf("After viewMatrix:\n");
    for (int i = 0; i < 3; i++) {
        vec3 p = points[i];
        debugPrintf("\t");
        print(camera.viewMatrix * p);
        debugPrintf("\n");
    }

    debugPrintf("After projection + view:\n");
    for (int i = 0; i < 3; i++) {
        vec4 p = vec4(points[i], 1.0f);
        debugPrintf("\t");
        print((vp * p));
        debugPrintf("\n");
    }

    debugPrintf("Homogenized\n");
    for (int i = 0; i < 3; i++) {
        vec4 p = vec4(points[i], 1.0f);
        debugPrintf("\t");
        print(homogenize(vp * p));
        debugPrintf("\n");
    }

    return true;
}

vec2 camSphere(0.0f);
void render()
{
    glViewport(0, 0, gameState.windowState.width, gameState.windowState.height);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(vao);
    glUseProgram(s.id);

    //POINT p;
    vec2 mPos = vec2(gameState.input.mouseX, gameState.input.mouseY);
    mPos.x /= gameState.windowState.width;
    mPos.y /= gameState.windowState.height;
    mPos = mPos * 2.0f - 1.0f;
    mPos.y *= -1.0f;

    // CAMERA
    float sensitivity = 0.003f;
    camSphere += vec2(-gameState.input.deltaX, gameState.input.deltaY) * sensitivity;
    camSphere = sphericalNorm(camSphere);

    camSphere = vec2(d2r(100.0f*(float)gameState.time.now), sinf((float)gameState.time.now) * d2r(30));
    vec3 pos = sp2eu(camSphere) * -3.0f;

    if (gameState.windowState.wasResized) {
        camera.projectionMatrix = projection(0.01f, 100.0f, d2r(90), (float)gameState.windowState.width/gameState.windowState.height);
    }

    camera.viewMatrix = lookAt(pos, vec3(0));
    mat4 vp = camera.projectionMatrix * camera.viewMatrix;
    glUniformMatrix4fv(0, 1, GL_FALSE, (GLfloat*) &vp);

    // CAMERA END
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
