#ifndef __RENDER_STATE_HPP__
#define __RENDER_STATE_HPP__

// Includes
#include "openGLFunctions.hpp"

#define TEXTURE_UNIT_COUNT 16
struct RenderState
{
    GLuint vao;
    GLuint program;
    GLuint fbo;
    GLuint textureUnit[TEXTURE_UNIT_COUNT];
    int nextTextureUnit;
    int viewportWidth;
    int viewportHeight;
    int frameCounter;
};

RenderState renderState;
void initRenderer() 
{
    memset(&renderState, 0, sizeof(RenderState));
    glBindVertexArray(0);
    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    for (int i = 0; i < TEXTURE_UNIT_COUNT; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void setViewport(int width, int height) 
{
    if (renderState.viewportWidth != width || 
        renderState.viewportHeight != height) 
    {
        renderState.viewportWidth = width;
        renderState.viewportHeight = height;
        glViewport(0, 0, width, height);
    }
}

void bindVao(GLuint vao) {
    if (renderState.vao != vao) {
        glBindVertexArray(vao);
        renderState.vao = vao;
    }
}

void bindProgram(GLuint id) {
    if (renderState.program != id) {
        glUseProgram(id);
        renderState.program = id;
    }
}

void bindFbo(GLuint fbo) {
    if (renderState.fbo != fbo) {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        renderState.fbo = fbo;
        renderState.frameCounter++;
    }
}

// Returns bound texture unit
GLint bindTexture2D(GLuint id) 
{
    // Check if it is already bound
    for (int i = 0; i < TEXTURE_UNIT_COUNT; i++) {
        if (renderState.textureUnit[i] == id) {
            return i;
        }
    }

    // Bind to unit
    GLint unit = renderState.nextTextureUnit;
    renderState.nextTextureUnit = (renderState.nextTextureUnit + 1) % TEXTURE_UNIT_COUNT;
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, id);

    return unit;
}

#endif
