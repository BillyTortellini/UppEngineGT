#ifndef __OPENGL_STATE_HPP__
#define __OPENGL_STATE_HPP__

// Includes
#include "openGLFunctions.hpp"

#define TEXTURE_UNIT_COUNT 16
struct OpenGLState
{
    GLuint vao;
    GLuint program;
    GLuint fbo;
    GLuint textureUnit[TEXTURE_UNIT_COUNT];
    int nextTextureUnit;
};

OpenGLState glState;
void initOpenGLState() 
{
    memset(&glState, 0, sizeof(OpenGLState));
    glBindVertexArray(0);
    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    for (int i = 0; i < TEXTURE_UNIT_COUNT; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void bindVao(GLuint vao) {
    if (glState.vao != vao) {
        glBindVertexArray(vao);
        glState.vao = vao;
    }
}

void bindProgram(GLuint id) {
    if (glState.program != id) {
        glUseProgram(id);
        glState.program = id;
    }
}

void bindFbo(GLuint fbo) {
    if (glState.fbo != fbo) {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glState.fbo = fbo;
    }
}

// Returns bound texture unit
GLint bindTexture2D(GLuint id) 
{
    // Check if it is already bound
    for (int i = 0; i < TEXTURE_UNIT_COUNT; i++) {
        if (glState.textureUnit[i] == id) {
            return i;
        }
    }

    // Bind to unit
    GLint unit = glState.nextTextureUnit;
    glState.nextTextureUnit = (glState.nextTextureUnit + 1) % TEXTURE_UNIT_COUNT;
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, id);

    return unit;
}

#endif
