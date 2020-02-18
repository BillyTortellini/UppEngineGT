#ifndef __GAME_CPP__
#define __GAME_CPP__

// Own file includes
#include "uppLib.hpp"
#include "platform.hpp"
#include "fileListener.hpp"
#include "rendering/openGLFunctions.hpp"
#include "utils/tmpAlloc.hpp"
#include "utils/arcBallController.hpp"
#include "rendering/renderer.hpp"

// TODO:
// -----
//  - Replace System allocator with a list allocator
//      + block allocator for small allocations
//  - Something like std::vector (Dynamic array)
//  - In tmpAlloc replace the stackAlloc with a pointer
//  - Texture loading and saving (bmp-files)
//  - What would be nice:
//    - An easy way to set up a rendering pipeline
//    - Rendering to framebuffers
//    - HDR framebuffers
//    - Blur (Maybe compute buffers)
//    - Tone mapping
//    - Deferred shading
//    - Ambient occlusion
//    - Baked light maps
//    - Add Gamma correction
//    - Shadow mapping
//    - Normal mapping
//    - Multisampling 
//    - Texture loading/saving and stuff (header stuff)

GameState* gameState;

struct GameData
{
    SystemAllocator sysAlloc;
    ShaderProgram s;
    Mesh cubeMesh;
    ArcBallController controller;
    Camera3D camera;
    ShaderProgram sky;
};
GameData* gameData;

void gameAfterReset() 
{
    // Init renderer
    new(&gameData->sysAlloc) SystemAllocator;
    initTmpAlloc(&gameData->sysAlloc);
    initRenderer(&gameData->sysAlloc);
    setCamera(&gameData->camera);
    
    init(&gameData->sky, {"sky.vert", "sky.frag"});
}

void gameBeforeReset() {
    shutdownTmpAlloc();
    shutdown(&gameData->sky);
}

void gameInit() 
{
    gameAfterReset();

    // Set game options
    gameState->windowState.continuousDraw = true;
    gameState->windowState.fps = 60;

    // Create Shader
    init(&gameData->s, {"color.frag", "color.vert"});

    // Create mesh
    createCubeMesh(&gameData->cubeMesh);

    // Create/set camera and controller
    init(&gameData->camera);
    init(&gameData->controller, &gameData->camera, 0.005f, 0.2f); 
}

void gameShutdown() {
    shutdown(&gameData->s);
    shutdown(&gameData->cubeMesh);
    gameBeforeReset();
}

void gameTick() 
{
    Input* input = &gameState->input;
    if (input->keyPressed[KEY_ESCAPE]) {
        gameState->windowState.quit = true;
    }
    if (input->keyPressed[KEY_F5]) {
        gameState->windowState.hideCursor = !gameState->windowState.hideCursor;
        loggf("Hide cursor set to: %s\n", gameState->windowState.hideCursor ? "TRUE" : "FALSE");
    }
    if (input->keyPressed[KEY_F11]) {
        gameState->windowState.fullscreen = !gameState->windowState.fullscreen;
    }
    //if (input->keyPressed[KEY_A]) {
    //    loggf("KEY PRESSED SHIFT\n");
    //}

    // Resize if necessary
    if (gameState->windowState.wasResized) 
    {
        glViewport(0, 0, gameState->windowState.width, gameState->windowState.height);
        if (camera != nullptr) {
            camera->projection = projection(0.01f, 100.0f, d2r(90), 
                    (float)gameState->windowState.width/gameState->windowState.height);
        }
    }
    update(&gameData->controller, input);
    startFrame();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    draw(&gameData->sky, &gameData->cubeMesh, Transform(vec3(5.0f)));

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    draw(&gameData->s, &gameData->cubeMesh, Transform(vec3(0)));
    draw(&gameData->s, &gameData->cubeMesh, Transform(vec3(3.0f)));
    draw(&gameData->s, &gameData->cubeMesh, Transform(vec3(-3.0f)));
    draw(&gameData->s, &gameData->cubeMesh, Transform(vec3(-3.0f, -3.0f, 3.0f)));
    draw(&gameData->s, &gameData->cubeMesh, Transform(vec3(3.0f, -3.0f, -3.0f)));
    draw(&gameData->s, &gameData->cubeMesh, Transform(vec3(3.0f, -3.0f, -3.0f)));
}

// BINDINGS FOR DLL LOADING
extern "C"
{
    DECLARE_EXPORT void gameAudio(GameState* state, byte* stream, int length) {}
    DECLARE_EXPORT void gameInit(GameState* state) 
    {
        gameState = state;
        gameData = (GameData*) gameState->memory.memory;
        gameInit();
    }

    DECLARE_EXPORT void gameTick(GameState* state) {
        gameState = state;
        gameData = (GameData*) gameState->memory.memory;
        gameTick();
    }

    DECLARE_EXPORT void gameShutdown(GameState* state) {
        gameState = state;
        gameData = (GameData*) gameState->memory.memory;
        gameShutdown();
    }

    DECLARE_EXPORT void gameBeforeReset(GameState* state) {
        gameState = state;
        gameData = (GameData*) gameState->memory.memory;
        gameBeforeReset();
    }

    DECLARE_EXPORT void gameAfterReset(GameState* state) {
        gameState = state;
        gameData = (GameData*) gameState->memory.memory;
        gameAfterReset();
    }

    DECLARE_EXPORT void gameLoadFunctionPtrs(void** functions) 
    {
        int i = 0;
        loggFunc loggFuncPtr = (loggFunc)functions[i++];
        invalid_pathFunc invalid_pathPtr = (invalid_pathFunc) functions[i++];
        setDebugFunctions(loggFuncPtr, invalid_pathPtr);
        loggf("GameLoadFunctionPtrs\n");
        i++; // Init file listener
        i++; // check file changed
        createFileListener = (createFileListenerFunc) functions[i++];
        deleteFileListener = (deleteFileListenerFunc) functions[i++];
        glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC) functions[i++];
        glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC) functions[i++];
        glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC) functions[i++];
        glGenBuffers = (PFNGLGENBUFFERSPROC) functions[i++];
        glBindBuffer = (PFNGLBINDBUFFERPROC) functions[i++];
        glBufferData = (PFNGLBUFFERDATAPROC) functions[i++];
        glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) functions[i++];
        glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) functions[i++];
        glUseProgram = (PFNGLUSEPROGRAMPROC) functions[i++];
        glCreateShader = (PFNGLCREATESHADERPROC) functions[i++];
        glShaderSource = (PFNGLSHADERSOURCEPROC) functions[i++];
        glCompileShader = (PFNGLCOMPILESHADERPROC) functions[i++];
        glDeleteShader = (PFNGLDELETESHADERPROC) functions[i++];
        glCreateProgram = (PFNGLCREATEPROGRAMPROC) functions[i++];
        glDeleteProgram = (PFNGLDELETEPROGRAMPROC) functions[i++];
        glAttachShader = (PFNGLATTACHSHADERPROC) functions[i++];
        glDetachShader = (PFNGLDETACHSHADERPROC) functions[i++];
        glLinkProgram = (PFNGLLINKPROGRAMPROC) functions[i++];
        glGetShaderiv = (PFNGLGETSHADERIVPROC) functions[i++];
        glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) functions[i++];
        glGetProgramiv = (PFNGLGETPROGRAMIVPROC) functions[i++];
        glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) functions[i++];
        glGetActiveUniform = (PFNGLGETACTIVEUNIFORMPROC) functions[i++];
        glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) functions[i++];
        glUniform1f = (PFNGLUNIFORM1FPROC) functions[i++];
        glUniform2f = (PFNGLUNIFORM2FPROC) functions[i++];
        glUniform3f = (PFNGLUNIFORM3FPROC) functions[i++];
        glUniform4f = (PFNGLUNIFORM4FPROC) functions[i++];
        glUniform1i = (PFNGLUNIFORM1IPROC) functions[i++];
        glUniform2i = (PFNGLUNIFORM2IPROC) functions[i++];
        glUniform3i = (PFNGLUNIFORM3IPROC) functions[i++];
        glUniform4i = (PFNGLUNIFORM4IPROC) functions[i++];
        glUniform1ui = (PFNGLUNIFORM1UIPROC) functions[i++];
        glUniform2ui = (PFNGLUNIFORM2UIPROC) functions[i++];
        glUniform3ui = (PFNGLUNIFORM3UIPROC) functions[i++];
        glUniform4ui = (PFNGLUNIFORM4UIPROC) functions[i++];
        glUniform1fv = (PFNGLUNIFORM1FVPROC) functions[i++];
        glUniform2fv = (PFNGLUNIFORM2FVPROC) functions[i++];
        glUniform3fv = (PFNGLUNIFORM3FVPROC) functions[i++];
        glUniform4fv = (PFNGLUNIFORM4FVPROC) functions[i++];
        glUniform1iv = (PFNGLUNIFORM1IVPROC) functions[i++];
        glUniform2iv = (PFNGLUNIFORM2IVPROC) functions[i++];
        glUniform3iv = (PFNGLUNIFORM3IVPROC) functions[i++];
        glUniform4iv = (PFNGLUNIFORM4IVPROC) functions[i++];
        glUniform1uiv = (PFNGLUNIFORM1UIVPROC) functions[i++];
        glUniform2uiv = (PFNGLUNIFORM2UIVPROC) functions[i++];
        glUniform3uiv = (PFNGLUNIFORM3UIVPROC) functions[i++];
        glUniform4uiv = (PFNGLUNIFORM4UIVPROC) functions[i++];
        glUniformMatrix2fv = (PFNGLUNIFORMMATRIX2FVPROC) functions[i++];
        glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC) functions[i++];
        glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) functions[i++];
        glUniformMatrix2x3fv = (PFNGLUNIFORMMATRIX2X3FVPROC) functions[i++];
        glUniformMatrix3x2fv = (PFNGLUNIFORMMATRIX3X2FVPROC) functions[i++];
        glUniformMatrix2x4fv = (PFNGLUNIFORMMATRIX2X4FVPROC) functions[i++];
        glUniformMatrix4x2fv = (PFNGLUNIFORMMATRIX4X2FVPROC) functions[i++];
        glUniformMatrix3x4fv = (PFNGLUNIFORMMATRIX3X4FVPROC) functions[i++];
        glUniformMatrix4x3fv = (PFNGLUNIFORMMATRIX4X3FVPROC) functions[i++];
        glGetActiveAttrib = (PFNGLGETACTIVEATTRIBPROC) functions[i++];
        glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC) functions[i++];
        glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) functions[i++];
        glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC) functions[i++];
        glGetStringi = (PFNGLGETSTRINGIPROC) functions[i++];
        wglSwapIntervalExt = (PFNWGLSWAPINTERVALEXTPROC) functions[i++];
        wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC) functions[i++];
    }
}






#endif
