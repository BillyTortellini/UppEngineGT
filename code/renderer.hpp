#ifndef __RENDERER_HPP__
#define __RENDERER_HPP__

#include "umath.hpp"
#include "datatypes.hpp"

// Todo:
//  * Render a cube
//  * Arcball camera
//  * Define a mesh
//  * Define a material

struct Camera3D
{
    vec3 pos;
    mat4 viewMatrix;
    mat4 projectionMatrix;
};

void init(Camera3D* c, float near, float far, float fovX, float aspectRatio)
{
    float fovY = fovX / aspectRatio;
    float r = sinf(fovX/2.0f) * near;
    float t = sinf(fovY/2.0f) * near;
    c->projectionMatrix.columns[0] = vec4(near/r, 0.0f, 0.0f, 0.0f);
    c->projectionMatrix.columns[1] = vec4(0.0f, near/t, 0.0f, 0.0f);
    c->projectionMatrix.columns[2] = vec4(0.0f, 0.0f, -(far+near)/(far-near), -1.0f);
    c->projectionMatrix.columns[3] = vec4(0.0f, 0.0f, (-2.0f*near*far)/(far-near), 0.0f);
}

void lookAt(Camera3D* c, const vec3& pos, const vec3& dir, const vec3& up) 
{
    vec3 d = normalizeSafe(dir);
    vec3 u = normalizeSafe(up);
    vec3 r = cross(d, u);
    u = cross(r, d);
    c->viewMatrix = mat4(transpose(mat3(r, u, d)));
    c->viewMatrix.columns[3] = vec4(-pos, 1.0f);
    c->pos = pos;
}

// Camera:
// Model to View Matrix
// Projection Matrix
// Position
// Look at direction

struct ShaderProgram
{
    int id;
    int filenameCount;
    String* filenames;
};

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
    int* shaderIDs = (int*) malloc(sizeof(int) * p->filenameCount);

    // Loop over all files
    for (int i = 0; i < p->filenameCount; i++)
    {
        String* filename = p->filenames + i;
        
        // Check extension (Valid extensions are .frag, .vert)
        GLenum shaderType = 0;
        if (endsWith(filename, ".frag")) {
            shaderType = GL_FRAGMENT_SHADER;
        }
        else if (endsWith(filename, ".vert")) {
            shaderType = GL_VERTEX_SHADER;
        } 
        else {
            debugPrintf("Init shaderprogram: could not get shadertype from filename: %s\n", c_str(filename));
            debugWaitForConsoleInput();
            continue;
        }
        shaderIDs[i] = glCreateShader(shaderType);

        // Load shader file
        char* fileContent = load_text_file(c_str(filename));
        if (fileContent == nullptr) {
            debugPrintf("Could not load shaderfile: %s\n", c_str(filename));
            debugWaitForConsoleInput();
            continue;
        }

        // Compile shader
        GLchar* source = (GLchar*) fileContent;
        glShaderSource(shaderIDs[i], 1, &source, NULL);
        glCompileShader(shaderIDs[i]);

        GLint isCompiled = 0;
        glGetShaderiv(shaderIDs[i], GL_COMPILE_STATUS, &isCompiled);
        if (isCompiled == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetShaderiv(shaderIDs[i], GL_INFO_LOG_LENGTH, &maxLength);

            char buffer[2048];
            glGetShaderInfoLog(shaderIDs[i], 2048, &maxLength, buffer);
            debugPrintf("Could not compile shader \"%s\", error msg: \n %s\n", c_str(filename), buffer);
            debugWaitForConsoleInput();
            continue;
        }

        unload_text_file(fileContent);
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

    // Cleanup
    free(shaderIDs);

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

int vbo, vao, ebo;
ShaderProgram s;
Camera3D camera;
bool initRenderer()
{
    init(&camera, 0.01f, 100.0f, 90.0f, 1.3f);
    init(&s, {"ressources/shaders/color.frag", "ressources/shaders/color.vert"});
    printAllUniforms(&s);
    //shutdown(&s);

    glGenVertexArrays(1, (GLuint*) &vao);
    glBindVertexArray(vao);

    // Fill vbo
    float vertexData[] = {
        -0.5f, -0.5f,
        1.0f, 0.0f, 0.0f,
        0.5f, -0.5f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.5f,
        0.0f, 0.0f, 1.0f
    };

    glGenBuffers(1, (GLuint*) &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*) 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*) (sizeof(float) * 2));
    glEnableVertexAttribArray(1);

    u32 elementData[] = {0, 1, 2};
    glGenBuffers(1, (GLuint*) &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elementData), elementData, GL_STATIC_DRAW);

    //wglSwapIntervalExt(1);
    wglSwapIntervalExt(-1);

    return true;
}

extern GameState gameState;

void render()
{
    glViewport(0, 0, gameState.windowState.width, gameState.windowState.height);

    glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(vao);
    glUseProgram(s.id);

    //POINT p;
    //GetCursorPos(&p);
    //ScreenToClient(hwnd, &p);
    //vec2 mPos = vec2((float)p.x, (float)p.y);
    vec2 mPos = vec2(gameState.input.mouseX, gameState.input.mouseY);
    mPos.x /= gameState.windowState.width;
    mPos.y /= gameState.windowState.height;
    mPos = mPos * 2.0f - 1.0f;
    mPos.y *= -1.0f;

    // CAMERA
    lookAt(&camera, vec3(0.0f, 0.0f, -3.0f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f));
    mat4 vp = camera.projectionMatrix * camera.viewMatrix;
    vp = mat4(1.0f);
    glUniformMatrix4fv(1, 1, GL_FALSE, (GLfloat*) &vp);
    // CAMERA END

    vec2 pos = mPos;
    pos.x -= 0.2f;
    glUniform2fv(0, 1, (GLfloat*) &pos);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, NULL);

    pos = mPos;
    pos.x += 0.2f;
    glUniform2fv(0, 1, (GLfloat*) &pos);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, NULL);

    SwapBuffers(deviceContext);
    //glFinish();
}





#endif
