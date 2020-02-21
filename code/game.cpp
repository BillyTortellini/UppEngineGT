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
#include <functional>

// TODO:
// -----
//  - 3D Transform with Quaternions (Or just from rotation vector) 
//  - Phong shading
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

struct GameData
{
    Camera3D camera;
    ArcBallController controller;
    AutoMesh cubeMesh;
    AutoMesh planeMesh;
    AutoMesh quadMesh;
    int frameCount;
};

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

// What does a framebuffer need?
//  - width height
//  - resizable
//  - Color attachment (Either RGBA or just RGB or stuff
//  - Depth attachment
//  - Depth-Stencil attachment
//  - 
//
// // How i would like it to work
// -- render into (e.g. deferred shading)
// Framebuffer f;
// init(&f, 512, 512, {GL_RGBA, GL_DEPTH_COMPONENT, GL_STENCIL_INDEX, GL_DEPTH_STENCIL}
//
// Either rendering into
//  - width, h
struct Framebuffer
{
    GLuint fbo;
    GLuint colorTexture;
    GLuint depthTexture;
    int width, height;
};

void init(Framebuffer* f, int width, int height) 
{
    f->width = width;
    f->height = height;

    glGenTextures(1, &f->colorTexture);
    glGenTextures(1, &f->depthTexture);
    assert(f->colorTexture != 0 && f->depthTexture != 0, "glGenTextures failed\n");

    // Generate gpu storage
    bindTexture2D(f->colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, 
            GL_RGBA, GL_UNSIGNED_BYTE, nullptr); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    bindTexture2D(f->depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, 
            GL_DEPTH_COMPONENT, GL_FLOAT, nullptr); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Generate framebuffer
    glGenFramebuffers(1, &f->fbo);
    assert(f->fbo != 0, "glGenFramebuffer failed!\n");
    bindFbo(f->fbo);

    // Attach depth and color texture
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, f->colorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, f->depthTexture, 0);

    // Check if finished
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer not complete\n");

    // Unbind framebuffer so nothing can mess with the data
    bindFbo(0);
}

void shutdown(Framebuffer* f) 
{
    bindFbo(0); // Make sure this framebuffer is not bound
    glDeleteTextures(1, &f->colorTexture);
    glDeleteTextures(1, &f->depthTexture);
    glDeleteFramebuffers(1, &f->fbo);
}

void bind(Framebuffer* f) {
    bindFbo(f->fbo);
}

void setUniform(ShaderProgram* p, const char* name, Framebuffer* t)
{
    bindProgram(p->id); 
    UniformInfo* info = getUniformInfo(p, name); 
    if (info == nullptr) { 
        loggf("Uniform \"%s\" not in shaderprogram\n", name); 
        return; 
    } 
    if (info->type != GL_SAMPLER_2D) { 
        loggf("Uniform \"%s\" type did not match\n", name); 
        return; 
    } 
    glUniform1i(info->location, bindTexture2D(t->colorTexture)); 
}

AutoShaderProgram colorShader;
AutoShaderProgram imageShader;
AutoShaderProgram skyShader;
Framebuffer testFramebuffer;
Texture testTexture;
void gameAfterReload() 
{
    // Set game options
    gameState->renderOptions.vsync = true;
    gameState->renderOptions.continuousDraw = true;
    gameState->renderOptions.fps = 60;

    // Set to second monitor
    gameState->windowState.x = -1000;
    gameState->windowState.y = 50;
    gameState->windowState.width = 800;
    gameState->windowState.height = 600;
    gameState->windowState.fullscreen = false;

    // Init framecount
    gameData->frameCount = 0;

    // Init renderer
    initOpenGLState();

    // Init framebuffers
    init(&testFramebuffer, 1024, 1024);
    
    // Set default options
    glClearColor(0, 0, 0, 0);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    // Init textures
    init(&testTexture, "test.bmp", gameAlloc);

    // Init shaders
    init(&imageShader, {"image.vert", "image.frag"}, gameAlloc);
    init(&colorShader, {"color.vert", "color.frag"}, gameAlloc);
    init(&skyShader, {"sky.vert", "sky.frag"}, gameAlloc);
}

void gameBeforeReload() 
{
    shutdown(&testTexture);
    shutdown(&imageShader);
    shutdown(&colorShader);
    shutdown(&skyShader);
    shutdown(&testFramebuffer);
}

void renderScene() 
{
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    gameData->frameCount++;

    // Draw sky
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    draw(&skyShader, &gameData->cubeMesh, vec3(0.0f));

    // Draw meshes
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    draw(&colorShader, &gameData->cubeMesh, vec3(3.0f));
    setUniform(&imageShader.program, "image", &testTexture);
    draw(&imageShader, &gameData->planeMesh, vec3(0.0f));
    

    //prepare(&imageShader, &gameData->planeMesh, Transform(vec3(0)));
    //GLint loc = bind(&testTexture);
    ////setUniform(&imageShader, "image", loc);
    //draw(&gameData->planeMesh);

    draw(&colorShader, &gameData->cubeMesh, vec3(-3.0f));
    draw(&colorShader, &gameData->cubeMesh, vec3(-3.0f, -3.0f, 3.0f));
    draw(&colorShader, &gameData->cubeMesh, vec3(3.0f, -3.0f, -3.0f));
    draw(&colorShader, &gameData->cubeMesh, vec3(3.0f, -3.0f, -3.0f));
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
    if (input->keyPressed[KEY_Y]) {
        loggf("FPS: 60,  VSYNC = FALSE\n");
        gameState->renderOptions.fps = 60;
        gameState->renderOptions.vsync = false;
    }
    if (input->keyPressed[KEY_X]) {
        loggf("FPS: 120, VSYNC = FALSE\n");
        gameState->renderOptions.fps = 120;
        gameState->renderOptions.vsync = false;
    }
    if (input->keyPressed[KEY_C]) {
        loggf("FPS: 60,  VSYNC = TRUE\n");
        gameState->renderOptions.fps = 60;
        gameState->renderOptions.vsync = true;
    }
    if (input->keyPressed[KEY_F11]) {
        gameState->windowState.fullscreen = !gameState->windowState.fullscreen;
    }

    // Handle resize
    if (gameState->windowState.wasResized) {
        glViewport(0, 0, gameState->windowState.width, gameState->windowState.height);
        gameData->camera.projection = projection(0.01f, 100.0f, d2r(90), 
                (float)gameState->windowState.width/gameState->windowState.height);
    }

    update(&gameData->controller, gameState);

    // Render to framebuffer
    bind(&testFramebuffer);
    glViewport(0, 0, testFramebuffer.width, testFramebuffer.height);
    renderScene();
    bindFbo(0);

    // Render normally
    gameData->frameCount++;
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, gameState->windowState.width, gameState->windowState.height);
    
    setUniform(&imageShader.program, "image", &testFramebuffer);
    draw(&imageShader, &gameData->planeMesh, vec3(0.0f));
    draw(&colorShader, &gameData->cubeMesh, vec3(0, 2, 0));

    //float t = (float)gameState->time.now;
    //vec3 pos = vec3(sinf(t), 0, cosf(t)) * 5;
    //draw(&colorShader, &gameData->cubeMesh, Transform(pos));
}

void gameInit() 
{
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

// Stub for game sound
void gameAudioTick(int size, byte* data){};

#endif
