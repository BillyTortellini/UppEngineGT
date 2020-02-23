#ifndef __FRAMEBUFFER_HPP__
#define __FRAMEBUFFER_HPP__

// -------------------
// --- FRAMEBUFFER ---
// -------------------
struct Framebuffer
{
    GLuint fbo;
    Texture depthTexture;
    Texture colorTexture;
    bool hasDepth;
    bool hasColor;
    bool autoResize;
    GLenum colorFormat;
    int width, height;
};

void init(Framebuffer* f, int width, int height, bool withColor, bool withDepth, bool autoResize, GLenum colorFormat) 
{
    // Set members
    f->hasDepth = withDepth;
    f->hasColor = withColor;
    f->autoResize = autoResize;
    f->colorFormat = colorFormat;
    f->width = width;
    f->height = height;

    // Check if called correctly
    assert(!(!withDepth && !withColor), "Both withDepth and withColor was false!\n");

    // Generate framebuffer
    glGenFramebuffers(1, &f->fbo);
    assert(f->fbo != 0, "glGenFramebuffer failed!\n");
    bindFbo(f->fbo);

    // Init textures
    if (withDepth) {
        init(&f->depthTexture, width, height, GL_DEPTH_COMPONENT, 
                TextureFilterMode(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE));
    }
    if (withColor) {
        init(&f->colorTexture, width, height, colorFormat, 
                TextureFilterMode(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE));
    }

    // Attach depth and color texture
    if (withColor) {
        bind(&f->colorTexture);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, f->colorTexture.id, 0);
    }
    if (withDepth) {
        bind(&f->depthTexture);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, f->depthTexture.id, 0);
    }

    // Check if finished
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch (status) 
    {
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            invalid_path("Status was GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            invalid_path("Status was GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            invalid_path("Status was GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER\n");
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            invalid_path("Status was GL_FRAMEBUFFER_UNSUPPORTED\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
            invalid_path("Status was GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS\n");
            break;
    }
    assert(status == GL_FRAMEBUFFER_COMPLETE, "Framebuffer not complete\n");

    // Unbind framebuffer so nothing can mess with the data
    bindFbo(0);
}

Texture* getDepthTexture(Framebuffer* f) {
    assert(f->hasDepth, "GetDepthTexture called on framebuffer with no depth\n");
    return &f->depthTexture;
}

Texture* getColorTexture(Framebuffer* f) {
    assert(f->hasColor, "GetDepthTexture called on framebuffer with no depth\n");
    return &f->colorTexture;
}

void shutdown(Framebuffer* f) 
{
    bindFbo(0); // Make sure this framebuffer is not bound
    if (f->hasColor) {
        shutdown(&f->colorTexture);
    }
    if (f->hasDepth) {
        shutdown(&f->depthTexture);
    }
    glDeleteFramebuffers(1, &f->fbo);
}

void bind(Framebuffer* f, int width, int height) 
{
    if (f->autoResize) {
        if (f->width != width ||
            f->height != height) {
            f->width = width;
            f->height = height;

            if (f->hasDepth) {
                resize(&f->depthTexture, f->width, f->height);
            }
            if (f->hasColor) {
                resize(&f->colorTexture, f->width, f->height);
            }
        }
    }
    setViewport(f->width, f->height);
    bindFbo(f->fbo);
}

void bindDefaultFramebuffer(int width, int height) 
{
    setViewport(width, height);
    bindFbo(0);
}





#endif
