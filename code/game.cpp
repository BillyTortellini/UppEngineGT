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
#include "rendering/texture.hpp"

// TODO:
// -----
//  - Textures + Framebuffer rendering
//  - Something like std::vector (Dynamic array)
//  - AutoShaderProgram instead of ShaderProgram --> Try to get extern GameState state out of renderer!
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

struct GameData
{
    // GameAllocator
    StackAllocator _gameStack;
    Blk _tmpAllocBlk;
    BlockAllocator _b32;
    BlockAllocator _b64;
    BlockAllocator _b256;
    BlockAllocator _b1024;
    BlockAllocator _b4096;
    BlockAllocator _b8192;
    ListAllocator _listAlloc;
    SegregateAllocator<BlockAllocator, ListAllocator> _seg8192;
    SegregateAllocator<BlockAllocator, decltype(_seg8192)> _seg4096;
    SegregateAllocator<BlockAllocator, decltype(_seg4096)> _seg1024;
    SegregateAllocator<BlockAllocator, decltype(_seg1024)> _seg256;
    SegregateAllocator<BlockAllocator, decltype(_seg256)> _seg64;
    SegregateAllocator<BlockAllocator, decltype(_seg64)> gameAlloc;

    // Game variables
    Mesh cubeMesh;
    Mesh planeMesh;
    Mesh plane2DMesh;
    ArcBallController controller;
    Camera3D camera;
};

GameData* gameData;
GameState* gameState;
decltype(GameData::gameAlloc)* gameAlloc;

void initUniforms(GameState* state) 
{
    gameState = state;
    gameData = (GameData*) gameState->memory.memory;
    gameAlloc = &gameData->gameAlloc;
}

void createGameAlloc()
{
    GameData* d = gameData;
    // Check if gameMemory is big enough
    assert(gameState->memory.size > 1024*1024*256 + 1024*1024*8, "Not enough memory in memory\n");

    // Create game stack allocator
    Blk stackData = Blk(gameState->memory.memory, gameState->memory.size);
    // Skip some bytes so that we can add stuff to gamedata later
    u64 offset = sizeof(GameData) * 16;
    stackData.data = roundToAligned((void*)((u64)stackData.data + offset));
    stackData.size -= offset * 2; // This could be done lot better because rounding
    d->_gameStack.init(stackData);

    // Create temp alloc bloc
    d->_tmpAllocBlk = d->_gameStack.alloc(1024 * 1024 * 256); // 256 MB

    // Block allocators for small allocations
    d->_b32.init(&d->_gameStack, 32, 8192*4*4); 
    d->_b64.init(&d->_gameStack, 64, 8192*2*4); 
    d->_b256.init(&d->_gameStack, 256, 8192*2); 
    d->_b1024.init(&d->_gameStack, 1024, 8192); 
    d->_b4096.init(&d->_gameStack, 4096, 2048);
    d->_b8192.init(&d->_gameStack, 8192, 1024);
    // For big allocations use the list allocator
    d->_listAlloc.init(d->_gameStack.allocAll());

    d->_seg8192.init(8192, &d->_b8192, &d->_listAlloc); 
    d->_seg4096.init(4096, &d->_b4096, &d->_seg8192);
    d->_seg1024.init(1024, &d->_b1024, &d->_seg4096);
    d->_seg256.init(256, &d->_b256, &d->_seg1024);
    d->_seg64.init(64, &d->_b64, &d->_seg256);
    d->gameAlloc.init(32, &d->_b32, &d->_seg64);
}

GLuint createFramebuffer()
{
    //GLuint fbo; // Framebuffer object
    //glGenFramebuffer(1, &fbo);
    //glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    //assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer not complete\n");

    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    //GLuint rbo; // Renderbuffer object
    //glGenRenderbuffers(1, &rbo);
    //glDeleteRenderbuffers(1, &rbo);

    //glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    //glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
    //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    //glDeleteFramebuffers(1, &fbo);
    return 0;
}

ShaderProgram imageShader;
Texture testTexture;
ShaderProgram skyShader;
ShaderProgram colorShader;

void gameAfterReset() 
{
    // Init renderer
    initTmpAlloc(gameData->_tmpAllocBlk);
    initRenderer(&gameData->gameAlloc);
    setCamera(&gameData->camera);

    // Create Shader
    init(&colorShader, {"color.frag", "color.vert"});
    init(&skyShader, {"sky.vert", "sky.frag"});
    init(&imageShader, {"image.vert", "image.frag"});

    // Load texture
    init(&testTexture, "test.bmp");
}

void gameBeforeReset() 
{
    shutdown(&skyShader);
    shutdown(&imageShader);
    shutdown(&colorShader);
    shutdown(&testTexture);

    shutdownTmpAlloc();
}

void gameInit() 
{
    createGameAlloc();
    gameAfterReset();

    // Set game options
    gameState->windowState.continuousDraw = true;
    gameState->windowState.fps = 60;

    // Create mesh
    createCubeMesh(&gameData->cubeMesh);
    createPlaneMesh(&gameData->planeMesh);
    createPlane2DMesh(&gameData->plane2DMesh);

    // Create/set camera and controller
    init(&gameData->camera);
    init(&gameData->controller, &gameData->camera, 0.005f, 0.2f); 
}

void gameShutdown() 
{
    shutdown(&gameData->cubeMesh);
    shutdown(&gameData->planeMesh);
    gameBeforeReset();
}

//void setUniform(ShaderProgram* p, const char* name, int val) 
//{
//    GLint loc = glGetUniformLocation(p->id, name);
//    if (loc == -1) {
//        loggf("Uniform %s not found\n", name);
//        return;
//    }
//
//    glUniform1i(loc, val);
//}

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

    // Resize if necessary
    if (gameState->windowState.wasResized) 
    {
        glViewport(0, 0, gameState->windowState.width, gameState->windowState.height);
        if (camera != nullptr) { camera->projection = projection(0.01f, 100.0f, d2r(90), 
                (float)gameState->windowState.width/gameState->windowState.height);
        }
    }
    update(&gameData->controller, input);
    startFrame();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    draw(&skyShader, &gameData->cubeMesh, Transform(vec3(5.0f)));

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    draw(&skyShader, &gameData->cubeMesh, Transform(vec3(0)));
    prepare(&imageShader, &gameData->planeMesh, Transform(vec3(0)));
    GLint loc = bind(&testTexture);
    //setUniform(&imageShader, "image", loc);
    draw(&gameData->planeMesh);

    draw(&colorShader, &gameData->cubeMesh, Transform(vec3(3.0f)));
    draw(&colorShader, &gameData->cubeMesh, Transform(vec3(-3.0f)));
    draw(&colorShader, &gameData->cubeMesh, Transform(vec3(-3.0f, -3.0f, 3.0f)));
    draw(&colorShader, &gameData->cubeMesh, Transform(vec3(3.0f, -3.0f, -3.0f)));
    draw(&colorShader, &gameData->cubeMesh, Transform(vec3(3.0f, -3.0f, -3.0f)));

    float t = (float)gameState->time.now;
    vec3 pos = vec3(sinf(t), 0, cosf(t)) * 5;
    draw(&colorShader, &gameData->cubeMesh, Transform(pos));
}

// BINDINGS FOR DLL LOADING
extern "C"
{
    DECLARE_EXPORT void gameAudio(GameState* state, byte* stream, int length) {}
    DECLARE_EXPORT void gameInit(GameState* state) 
    {
        initUniforms(state);
        gameInit();
    }

    DECLARE_EXPORT void gameTick(GameState* state) {
        initUniforms(state);
        gameTick();
    }

    DECLARE_EXPORT void gameShutdown(GameState* state) {
        initUniforms(state);
        gameShutdown();
    }

    DECLARE_EXPORT void gameBeforeReset(GameState* state) {
        initUniforms(state);
        gameBeforeReset();
    }

    DECLARE_EXPORT void gameAfterReset(GameState* state) {
        initUniforms(state);
        gameAfterReset();
    }

    DECLARE_EXPORT void gameLoadFunctionPtrs(void** functions) 
    {
        int i = 0;
        loggFunc loggFuncPtr = (loggFunc)functions[i++];
        invalid_pathFunc invalid_pathPtr = (invalid_pathFunc) functions[i++];
        setDebugFunctions(loggFuncPtr, invalid_pathPtr);
        //loggf("GameLoadFunctionPtrs\n");
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
        glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC) functions[i++];
        glActiveTexture = (PFNGLACTIVETEXTUREPROC) functions[i++];
        glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) functions[i++];
        glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) functions[i++];
        glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC) functions[i++];
        glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) functions[i++];
        glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC) functions[i++];
        glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC) functions[i++];
        glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC) functions[i++];
        glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC) functions[i++];
        glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC) functions[i++];
        glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC) functions[i++];
    }
}




#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#endif
