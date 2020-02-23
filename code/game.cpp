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
#include "materialRenderer/materialRenderer.hpp"

// TODO:
// -----
//  More Autouniforms, u_resolution, u_res, u_viewport, u_screenSize, u_screen, u_size,
//         u_mousePos, u_mPos, u_mouse
//  
//  - 3D Transform with Quaternions (Or just from rotation vector) 
//  - rayTracing Shader
//  - Shaderprogram autoadd #define primitives
//  - MaterialRenderer(phong shading, directional lighting)
//  - Textures from Shaderfiles (Maybe animated)
//  - Improved MeshGenerator
//    * More primitives (Spheres, Torus, Cylinder, Pill)
//    * generateNormals(float angleThreshhold)
//    * boolean operations(Union...)
//    * UV projections (sphere-projection, orthogonal, perspective, cylinder-projection)
//    * Marching cubes
//    * Procedural terrain
//    * Metaballs
//  - Rendering Stuff:
//    * HDR framebuffers
//    * Blur 
//    * Tone mapping
//    * Deferred shading
//    * Ambient occlusion
//    * Baked light maps
//    * Add Gamma correction
//    * Shadow mapping
//    * Normal mapping
//    * Multisampling 
//    * Computeshaders

struct GameData
{
    Camera3D camera;
    ArcBallController controller;
    AutoMesh cubeMesh;
    AutoMesh planeMesh;
    AutoMesh quadMesh;
};

void draw(AutoMesh* m, AutoShaderProgram* p, const vec3& pos) {
    vec2 mousePos = vec2((float)gameState->input.mouseX/gameState->windowState.width, 
            (float)gameState->input.mouseY/gameState->windowState.height);
    draw(m, p, &gameData->camera, mousePos, (float)gameState->time.now, Transform(pos));
}

void updateAutoUniforms(AutoShaderProgram* p) {
    vec2 mousePos = vec2((float)gameState->input.mouseX/gameState->windowState.width, 
            (float)gameState->input.mouseY/gameState->windowState.height);
    updatePerFrameUniforms(p, &gameData->camera, mousePos, (float)gameState->time.now);
}

AutoShaderProgram colorShader;
AutoShaderProgram imageShader;
AutoShaderProgram skyShader;
AutoShaderProgram postProcessShader;
AutoShaderProgram testShader;
Framebuffer postProcessFramebuffer;
Texture testTexture;
MaterialRenderer materialRenderer;
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
    gameState->windowState.fullscreen = true;

    // Init framebuffers
    init(&postProcessFramebuffer, 
            gameState->windowState.width, 
            gameState->windowState.height,
            true, true, true, GL_RGBA);

    // Init renderers
    init(&materialRenderer, &gameData->camera, gameAlloc);
    
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
    init(&postProcessShader, {"postProcess.vert", "postProcess.frag"}, gameAlloc);
    init(&testShader, {"test/test.vert", "test/test.frag"}, gameAlloc);
}

void gameBeforeReload() 
{
    shutdown(&testTexture);
    shutdown(&imageShader);
    shutdown(&colorShader);
    shutdown(&skyShader);
    shutdown(&postProcessShader);
    shutdown(&postProcessFramebuffer);
    shutdown(&materialRenderer);
    shutdown(&testShader);
}

void renderScene() 
{
#define Resolution gameState->windowState.width, gameState->windowState.height
    bind(&postProcessFramebuffer, Resolution);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    // Draw sky
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    draw(&gameData->cubeMesh, &skyShader, vec3(0.0f));

    // Draw meshes
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    setUniform(&imageShader.program, "image", &testTexture);
    draw(&gameData->planeMesh, &imageShader, vec3(0.0f));

    //draw(&gameData->cubeMesh, &imageShader, vec3(3.0f));
    //draw(&gameData->cubeMesh, &colorShader, vec3(-3.0f));
    //draw(&gameData->cubeMesh, &colorShader, vec3(-3.0f, -3.0f, 3.0f));
    //draw(&gameData->cubeMesh, &colorShader, vec3(3.0f, -3.0f, -3.0f));
    //draw(&gameData->cubeMesh, &colorShader, vec3(3.0f, -3.0f, -3.0f));
    draw(&materialRenderer, &gameData->cubeMesh, vec3(0));
    render(&materialRenderer, gameState);

    bindDefaultFramebuffer(Resolution);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    setUniform(&postProcessShader, "frame", getColorTexture(&postProcessFramebuffer));
    setUniform(&postProcessShader, "depthMap", getDepthTexture(&postProcessFramebuffer));
    updateAutoUniforms(&postProcessShader);
    draw(&gameData->quadMesh, &postProcessShader);

    // Test shader
    glClear(GL_DEPTH_BUFFER_BIT);
    updateAutoUniforms(&testShader);
    draw(&gameData->quadMesh, &testShader);
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
    //gameState->windowState.hideCursor = gameState->windowState.inFocus;
    if (gameState->windowState.wasResized) {
        loggf("Width: %d, height: %d\n", gameState->windowState.width, gameState->windowState.height);
    }

    // Handle resize
    if (gameState->windowState.wasResized) {
        setViewport(gameState->windowState.width, gameState->windowState.height);
        gameData->camera.projection = projection(0.01f, 100.0f, d2r(90), 
                (float)gameState->windowState.width/gameState->windowState.height);
    }

    // Update camera
    update(&gameData->controller, gameState);

    renderScene();
}

void gameInit() 
{
    // Create basic meshes 
    createCubeMesh(&gameData->cubeMesh, gameAlloc);
    createPlaneMesh(&gameData->planeMesh, gameAlloc);
    createQuadMesh(&gameData->quadMesh, gameAlloc);

    // Create/set camera and controller
    init(&gameData->camera, gameState->windowState.width, gameState->windowState.height);
    init(&gameData->controller, &gameData->camera, 0.005f, 0.2f);
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
