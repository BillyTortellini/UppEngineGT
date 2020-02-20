#ifndef __GAME_CPP__
#define __GAME_CPP__

// Own file includes
#include "uppLib.hpp"
#include "platform.hpp"
#include "utils/tmpAlloc.hpp"
#include "rendering/openGLFunctions.hpp"
#include "rendering/renderer.hpp"
#include "utils/meshGenerators.hpp"
#include "utils/camera.hpp"
#include "utils/arcBallController.hpp"

// TODO:
// -----
//  - Break up renderer into -> OpenGLState, ShaderProgram, Mesh
//  - AutoShaderProgram instead of ShaderProgram --> Try to get extern GameState state out of renderer!
//  - 3D Transform with Quaternions
//  - Phong shading
//  - If OpenGL render layer is finished, maybe move from game compile to normal compile
//  - Textures + Framebuffer rendering
//  - Textures from Shaderfiles
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
    Camera3D camera;
    ArcBallController controller;
    AutoMesh cubeMesh;
    AutoMesh planeMesh;
    AutoMesh quadMesh;
    int frameCount;
};

// Global uniforms (DONT CHANGE, these are set in gameHook.cpp)
GameData* gameData;
GameState* gameState;
Allocator* gameAlloc;

void draw(AutoShaderProgram* p, AutoMesh* m, const vec3& pos)
{
    updatePerFrameUniforms(p, &gameData->camera, gameData->frameCount, (float)gameState->time.now);
    updatePerModelUniforms(p, &gameData->camera, Transform(pos));
    draw(m, p);
}

void draw(AutoShaderProgram* p, Mesh* m, const vec3& pos)
{
    updatePerFrameUniforms(p, &gameData->camera, gameData->frameCount, (float)gameState->time.now);
    updatePerModelUniforms(p, &gameData->camera, Transform(pos));
    bind(p);
    draw(m);
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

AutoShaderProgram colorShader;
Mesh testMesh;
MeshData testMeshData;
ShaderProgram testShader;
void gameAfterReset() 
{
    // Init renderer
    initOpenGLState();
    
    // Set default options
    glClearColor(0, 0, 0, 0);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    // Init shaders
    //init(&imageShader, {"image.vert", "image.frag"}, gameAlloc, gameState);
    //init(&skyShader, {"sky.vert", "sky.frag"}, gameAlloc, gameState);
    init(&testShader, {"testShader.vert", "testShader.frag"}, gameAlloc);

    createCubeMeshData(&testMeshData, gameAlloc);
    init(&testMesh, &testMeshData, {
            AttribLocation(MeshAttrib::POS3, 0)},
            gameAlloc);

    //init(&colorShader, {"color.vert", "color.frag"}, gameAlloc, gameState);
    loggf("\n----------------\n");
    print(&testShader);
    loggf("\n----------------\n");
}

void gameBeforeReset() 
{
    shutdown(&colorShader);
    //shutdown(&imageShader);
    //shutdown(&skyShader);
    
    shutdown(&testShader);
    shutdown(&testMeshData);
    shutdown(&testMesh);
}

void gameInit() 
{
    // Set game options
    gameState->renderOptions.continuousDraw = true;
    gameState->renderOptions.fps = 60;

    // Set to second monitor
    gameState->windowState.x = -1000;
    gameState->windowState.y = 50;
    gameState->windowState.width = 800;
    gameState->windowState.height = 600;

    // Init framecount
    gameData->frameCount = 0;

    // Create mesh
    createCubeMesh(&gameData->cubeMesh, gameAlloc);
    createPlaneMesh(&gameData->planeMesh, gameAlloc);
    createQuadMesh(&gameData->quadMesh, gameAlloc);

    // Create/set camera and controller
    init(&gameData->camera, gameState);
    init(&gameData->controller, &gameData->camera, 0.005f, 0.2f);
    gameData->controller.pos = vec3(0, 0, 0);
    gameData->controller.distToCenter = 5;
}

void gameShutdown() 
{
    shutdown(&gameData->cubeMesh);
    shutdown(&gameData->planeMesh);
    shutdown(&gameData->quadMesh);
}

void gameTick() 
{
    gameData->frameCount++;

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
        loggf("width: %d, height: %d\n", gameState->windowState.width, gameState->windowState.height);
        gameData->camera.projection = projection(0.01f, 100.0f, d2r(90), 
                (float)gameState->windowState.width/gameState->windowState.height);
    }

    update(&gameData->controller, gameState);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    bind(&testShader);
    bind(&testMesh);
    update(&gameData->camera, gameData->frameCount);
    setUniform(&testShader, "u_MVP", gameData->camera.vp);
    draw(&testMesh);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    //draw(&skyShader, &testMesh, vec3(0.0f));
    //draw(&colorShader, &gameData->cubeMesh, vec3(0.0f));

    //prepare(&imageShader, &gameData->planeMesh, Transform(vec3(0)));
    //GLint loc = bind(&testTexture);
    ////setUniform(&imageShader, "image", loc);
    //draw(&gameData->planeMesh);

    //draw(&colorShader, &gameData->cubeMesh, vec3(-3.0f));
    //draw(&colorShader, &gameData->cubeMesh, vec3(-3.0f, -3.0f, 3.0f));
    //draw(&colorShader, &gameData->cubeMesh, vec3(3.0f, -3.0f, -3.0f));
    //draw(&colorShader, &gameData->cubeMesh, vec3(3.0f, -3.0f, -3.0f));

    //float t = (float)gameState->time.now;
    //vec3 pos = vec3(sinf(t), 0, cosf(t)) * 5;
    //draw(&colorShader, &gameData->cubeMesh, Transform(pos));
}

#include "gameHook.cpp"

#endif
