#ifndef __TEXTURE_HPP__
#define __TEXTURE_HPP__

#include "renderer.hpp"
#include "stb_image.h"


struct Image
{
    int width;
    int height;
    int numChannels;
    byte* data;    
    Blk blk;
    Allocator* alloc;
};

void init(Image* img, const char* filename, Allocator* alloc) 
{
    img->alloc = alloc;
    TmpStr filepath = "ressources/textures/";
    filepath = filepath + filename;

    // Load image
    stbi_set_flip_vertically_on_load(true);
    byte* data = stbi_load((char*)filepath, &img->width, &img->height, &img->numChannels, 0);
    assert(data != nullptr, "stbi_load failed\n");
    SCOPE_EXIT(stbi_image_free(data););

    // Copy data to renderAlloc
    u64 size = img->numChannels * img->width * img->height;
    img->blk = alloc->alloc(img->numChannels * img->width * img->height);
    memcpy(img->blk.data, data, size);
    img->data = (byte*) img->blk.data;
}

void shutdown(Image* img) {
    img->alloc->dealloc(img->blk);
}

GLuint createTexture(Image* img) 
{
    GLuint id;
    glGenTextures(1, &id);
    bindTexture2D(id);

    // Set image data
    GLenum format = GL_RED;
    GLint internalFormat = GL_RED;
    switch(img->numChannels) 
    {
        case 1:
            format = GL_RED;
            internalFormat = GL_RED;
            break;
        case 2:
            format = GL_RG;
            internalFormat = GL_RG;
            break;
        case 3:
            format = GL_RGB;
            internalFormat = GL_RGB;
            break;
        case 4:
            format = GL_RGBA;
            internalFormat = GL_RGBA;
            break;
        default:
            invalid_path("Num channels not valid!\n");
            break;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, 
            internalFormat,
            img->width, img->height, 
            0,// Always 0
            format, 
            GL_UNSIGNED_BYTE, 
            img->data);

    return id;
}

struct TextureFilterMode
{
    TextureFilterMode(){};
    TextureFilterMode(GLint min, GLint mag, GLint u, GLint v) 
        : minFilter(min), magFilter(mag), wrapU(u), wrapV(v) {}

    GLint minFilter;
    GLint magFilter;
    GLint wrapU;
    GLint wrapV;
};

const TextureFilterMode bestFilter =
    TextureFilterMode(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT);

struct Texture
{
    GLuint id;
    int width; 
    int height;
    int numChannels;
    TextureFilterMode filterMode;
    bool mipmapped;
};

GLint bind(Texture* texture) {
    return bindTexture2D(texture->id); 
}

void setFilterMode(Texture* tex, const TextureFilterMode& mode)
{
    bind(tex); 
    tex->filterMode = mode;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mode.minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mode.magFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode.wrapU);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode.wrapV);
    if (!tex->mipmapped && 
            (mode.minFilter == GL_LINEAR_MIPMAP_LINEAR || 
             mode.minFilter == GL_NEAREST_MIPMAP_LINEAR ||
             mode.minFilter == GL_LINEAR_MIPMAP_NEAREST || 
             mode.minFilter == GL_NEAREST_MIPMAP_NEAREST)) {
        glGenerateMipmap(GL_TEXTURE_2D);
        tex->mipmapped = true;
    }
}

void init(Texture* tex, Image* img, const TextureFilterMode& filterMode) 
{
    tex->width= img->width;
    tex->height= img->height;
    tex->numChannels = img->numChannels;
    tex->mipmapped = false;

    tex->id = createTexture(img);
    setFilterMode(tex, filterMode); 
}

void init(Texture* tex, Image* img) {
    init(tex, img, bestFilter);
}

void init(Texture* texture, const char* name, const TextureFilterMode& filterMode, Allocator* alloc) 
{
    // Load image from filepath
    Image tmpImg;
    init(&tmpImg, name, alloc);
    SCOPE_EXIT(shutdown(&tmpImg););

    // Create texture
    init(texture, &tmpImg, filterMode);
}

void init(Texture* texture, const char* name, Allocator* alloc) {
    init(texture, name, bestFilter, alloc); 
}

void shutdown(Texture* tex) {
    glDeleteTextures(1, &tex->id);
}

void setUniform(ShaderProgram* p, const char* name, Texture* t)
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
    glUniform1i(info->location, bind(t)); 
}









#endif
