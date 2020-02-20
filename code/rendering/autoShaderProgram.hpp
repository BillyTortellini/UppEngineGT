#ifndef __AUTO_SHADER_PROGRAM_HPP__
#define __AUTO_SHADER_PROGRAM_HPP__

#include "../utils/camera.hpp"
#include "../utils/transform.hpp"

namespace AutoUniformType
{
    enum ENUM
    {
        MODEL_MATRIX = 0,
        VIEW_MATRIX,
        PROJECTION_MATRIX,
        MVP_MATRIX,
        VP_MATRIX,
        CAMERA_POS,
        TIME,

        COUNT // MUST STAY LAST
    };
}

struct AutoUniform
{
    GLint location;
    AutoUniformType::ENUM type;
};

struct SupportedAutoUniform
{
    SupportedAutoUniform() {};
    SupportedAutoUniform(const char* name,
            AutoUniformType::ENUM type, 
            GLenum glType, 
            bool perFrame)
        : name(name), type(type), glType(glType), perFrame(perFrame) {};
    const char* name;
    AutoUniformType::ENUM type;
    GLenum glType;
    bool perFrame;
};

SupportedAutoUniform supportedAutoUniforms[] = {
    SupportedAutoUniform("u_model", AutoUniformType::MODEL_MATRIX, GL_FLOAT_MAT4, false), 
    SupportedAutoUniform("u_modelmat", AutoUniformType::MODEL_MATRIX, GL_FLOAT_MAT4, false), 
    SupportedAutoUniform("u_modelmatrix", AutoUniformType::MODEL_MATRIX, GL_FLOAT_MAT4, false), 
    SupportedAutoUniform("u_model", AutoUniformType::MODEL_MATRIX, GL_FLOAT_MAT4, false), 

    SupportedAutoUniform("u_view", AutoUniformType::VIEW_MATRIX, GL_FLOAT_MAT4, true), 
    SupportedAutoUniform("u_v", AutoUniformType::VIEW_MATRIX, GL_FLOAT_MAT4, true), 
    SupportedAutoUniform("u_viewmat", AutoUniformType::VIEW_MATRIX, GL_FLOAT_MAT4, true), 
    SupportedAutoUniform("u_viewmatrix", AutoUniformType::VIEW_MATRIX, GL_FLOAT_MAT4, true), 

    SupportedAutoUniform("u_projection", AutoUniformType::PROJECTION_MATRIX, GL_FLOAT_MAT4, true), 
    SupportedAutoUniform("u_p", AutoUniformType::PROJECTION_MATRIX, GL_FLOAT_MAT4, true), 
    SupportedAutoUniform("u_projectionmat", AutoUniformType::PROJECTION_MATRIX, GL_FLOAT_MAT4, true), 
    SupportedAutoUniform("u_projectionmatrix", AutoUniformType::PROJECTION_MATRIX, GL_FLOAT_MAT4, true), 

    SupportedAutoUniform("u_mvp", AutoUniformType::MVP_MATRIX, GL_FLOAT_MAT4, false), 
    SupportedAutoUniform("u_mvpmat", AutoUniformType::MVP_MATRIX, GL_FLOAT_MAT4, false), 
    SupportedAutoUniform("u_mvpmatrix", AutoUniformType::MVP_MATRIX, GL_FLOAT_MAT4, false), 

    SupportedAutoUniform("u_vp", AutoUniformType::VP_MATRIX, GL_FLOAT_MAT4, true), 
    SupportedAutoUniform("u_vpmat", AutoUniformType::VP_MATRIX, GL_FLOAT_MAT4, true), 
    SupportedAutoUniform("u_vpmatrix", AutoUniformType::VP_MATRIX, GL_FLOAT_MAT4, true), 

    SupportedAutoUniform("u_campos", AutoUniformType::CAMERA_POS, GL_FLOAT_VEC3, true), 
    SupportedAutoUniform("u_cam", AutoUniformType::CAMERA_POS, GL_FLOAT_VEC3, true), 
    SupportedAutoUniform("u_camera", AutoUniformType::CAMERA_POS, GL_FLOAT_VEC3, true), 
    SupportedAutoUniform("u_camerapos", AutoUniformType::CAMERA_POS, GL_FLOAT_VEC3, true), 
    SupportedAutoUniform("u_cameraposition", AutoUniformType::CAMERA_POS, GL_FLOAT_VEC3, true), 
    SupportedAutoUniform("u_eye", AutoUniformType::CAMERA_POS, GL_FLOAT_VEC3, true), 
    SupportedAutoUniform("u_eyepos", AutoUniformType::CAMERA_POS, GL_FLOAT_VEC3, true), 

    SupportedAutoUniform("u_time", AutoUniformType::TIME, GL_FLOAT, true),
    SupportedAutoUniform("u_t", AutoUniformType::TIME, GL_FLOAT, true),
    SupportedAutoUniform("u_now", AutoUniformType::TIME, GL_FLOAT, true),
};

struct SupportedAutoAttrib
{
    SupportedAutoAttrib() {}
    SupportedAutoAttrib(char* name, MeshAttrib::ENUM attrib, GLenum type)
        : name(name), attrib(attrib), type(type) {}
    const char* name;
    MeshAttrib::ENUM attrib;
    GLenum type;
};

// All lowercase because search is case insensitive
SupportedAutoAttrib supportedAutoAttribs[] =
{
    // Positions
    SupportedAutoAttrib("a_pos", MeshAttrib::POS2, GL_FLOAT_VEC2),
    SupportedAutoAttrib("a_pos", MeshAttrib::POS3, GL_FLOAT_VEC3),
    SupportedAutoAttrib("a_p", MeshAttrib::POS2, GL_FLOAT_VEC2),
    SupportedAutoAttrib("a_p", MeshAttrib::POS3, GL_FLOAT_VEC3),
    SupportedAutoAttrib("a_p2", MeshAttrib::POS2, GL_FLOAT_VEC2),
    SupportedAutoAttrib("a_p3", MeshAttrib::POS3, GL_FLOAT_VEC3),
    SupportedAutoAttrib("a_pos2", MeshAttrib::POS2, GL_FLOAT_VEC2),
    SupportedAutoAttrib("a_pos3", MeshAttrib::POS3, GL_FLOAT_VEC3),
    // Normals
    SupportedAutoAttrib("a_norm", MeshAttrib::NORMAL, GL_FLOAT_VEC3),
    SupportedAutoAttrib("a_normal", MeshAttrib::NORMAL, GL_FLOAT_VEC3),
    SupportedAutoAttrib("a_normals", MeshAttrib::NORMAL, GL_FLOAT_VEC3),
    SupportedAutoAttrib("a_n", MeshAttrib::NORMAL, GL_FLOAT_VEC3),
    // UVs
    SupportedAutoAttrib("a_uv", MeshAttrib::UV, GL_FLOAT_VEC2),
    SupportedAutoAttrib("a_uvs", MeshAttrib::UV, GL_FLOAT_VEC2),
    SupportedAutoAttrib("a_texcoord", MeshAttrib::UV, GL_FLOAT_VEC2),
    SupportedAutoAttrib("a_texcoords", MeshAttrib::UV, GL_FLOAT_VEC2),
    SupportedAutoAttrib("a_texturecoordinates", MeshAttrib::UV, GL_FLOAT_VEC2),
    SupportedAutoAttrib("a_coords", MeshAttrib::UV, GL_FLOAT_VEC2),
    // Colors
    SupportedAutoAttrib("a_colour", MeshAttrib::COLOR3, GL_FLOAT_VEC3),
    SupportedAutoAttrib("a_colour", MeshAttrib::COLOR4, GL_FLOAT_VEC4),
    SupportedAutoAttrib("a_color", MeshAttrib::COLOR3, GL_FLOAT_VEC3),
    SupportedAutoAttrib("a_color", MeshAttrib::COLOR4, GL_FLOAT_VEC4),
    SupportedAutoAttrib("a_col", MeshAttrib::COLOR3, GL_FLOAT_VEC3),
    SupportedAutoAttrib("a_col", MeshAttrib::COLOR4, GL_FLOAT_VEC4),
    SupportedAutoAttrib("a_c", MeshAttrib::COLOR3, GL_FLOAT_VEC3),
    SupportedAutoAttrib("a_c", MeshAttrib::COLOR4, GL_FLOAT_VEC4),
};

struct AutoShaderProgram
{
    ShaderProgram program;
    // Auto uniform 
    DynArr<AutoUniform> perModel;
    DynArr<AutoUniform> perFrame;
    // Prepare stuff
    int lastUpdateFrame;
    // Automatic Attribs
    DynArr<AttribLocation> attribLocs; // Sorted by location
};

void detectAutoUniforms(AutoShaderProgram* p)
{
    if (p->program.id == 0) {
        return;
    }
    p->perFrame.reset();
    p->perModel.reset();

    // Loop over all uniforms
    for (UniformInfo& info : p->program.uniformInfos) 
    {
        // Skip array uniforms (As of yet)
        if (info.size != 1) {
            continue;
        }

        // Set uniform name to all lower charcters
        TmpStr lowerName = info.name;
        toLower((char*) lowerName);

        // Loop over all supported uniforms and check if they match
        for(const SupportedAutoUniform& sup : supportedAutoUniforms)
        { 
            if (strcmp(sup.name, lowerName.c_str()) == 0 && 
                    sup.glType == info.type)
            {
                loggf("Detected uniform: %s\n", info.name);
                AutoUniform* uniform;
                if (sup.perFrame) {
                    uniform = &p->perFrame[p->perModel.size()];
                }
                else {
                    uniform = &p->perModel[p->perModel.size()];
                }

                uniform->type = sup.type;
                uniform->location = info.location;
                break;
            }
        }
    }
}

void detectAutoAttribs(AutoShaderProgram* p)
{
    if (p->program.id == 0) {
        return;
    }

    // Loop over all shader attributes
    for(AttribInfo& info : p->program.attribInfos)
    {
        TmpStr attribName = info.name;
        toLower((char*)attribName);
        // Loop over all supported auto attributes
        for (SupportedAutoAttrib& autoAttrib : supportedAutoAttribs)
        {
            if (strcmp(autoAttrib.name, attribName.c_str()) == 0 &&
                       autoAttrib.type == info.type)
            {
                loggf("AttribName found: %s\n", autoAttrib.name);
                p->attribLocs.push_back(AttribLocation(autoAttrib.attrib, info.location));
                break;
            }
        }
    }

    auto attribLocationCmp = [](AttribLocation* a, AttribLocation* b) {
        if (a->location < b->location) return -1;
        return 1;
    };

    // Sort attribs
    p->attribLocs.sort(attribLocationCmp);
}

void onAutoShaderReload(ShaderProgram* sp)
{
    AutoShaderProgram* p = (AutoShaderProgram*) sp;
    if (p->program.id == 0) {
        return;
    }

    detectAutoUniforms(p);
    detectAutoAttribs(p);
}

void init(AutoShaderProgram* p, 
        std::initializer_list<const char*> shaderFiles, 
        Allocator* alloc)
{
    init(&p->program, shaderFiles, alloc);
    p->perModel.init(alloc, 4);
    p->perFrame.init(alloc, 4);
    p->lastUpdateFrame = -1;
    p->attribLocs.init(alloc, 4);
    p->program.reloadCallbacks.push_back(&onAutoShaderReload);

    detectAutoUniforms(p);
    detectAutoAttribs(p);
}

void shutdown(AutoShaderProgram* p)
{
    shutdown(&p->program);
    p->perModel.shutdown();
    p->perFrame.shutdown();
    p->attribLocs.shutdown();
}

void bind(AutoShaderProgram* p) {
    bind(&p->program);
}

void updatePerFrameUniforms(AutoShaderProgram* program, 
        Camera3D* cam, int frame, float time)
{
    if (program->program.id == 0) {
        return;
    }

    bind(program);
    if (program->lastUpdateFrame == frame) 
        return;
    program->lastUpdateFrame = frame;

    // Prepare camera
    update(cam, frame);

    for (AutoUniform& u : program->perFrame) 
    {
        switch (u.type)
        {
            case AutoUniformType::VIEW_MATRIX:
                glUniformMatrix4fv(u.location, 1, GL_FALSE, (GLfloat*) &cam->view);
                break;
            case AutoUniformType::PROJECTION_MATRIX:
                glUniformMatrix4fv(u.location, 1, GL_FALSE, (GLfloat*) &cam->projection);
                break;
            case AutoUniformType::VP_MATRIX:
                glUniformMatrix4fv(u.location, 1, GL_FALSE, (GLfloat*) &cam->vp);
                break;
            case AutoUniformType::CAMERA_POS:
                glUniform3fv(u.location, 3, (GLfloat*) &cam->pos);
                break;
            case AutoUniformType::TIME:
                glUniform1f(u.location, time);
                break;
            default:
                invalid_path("ERROR");
                break;
        }
    }
}

void updatePerModelUniforms(AutoShaderProgram* program, Camera3D* cam, const Transform& transform)
{
    if (program->program.id == 0) {
        return;
    }
    bind(program);

    mat4 model = transform.toModelMat();
    mat4 mvp = cam->vp * model;
    for (AutoUniform& u : program->perModel) 
    {
        switch (u.type)
        {
            case AutoUniformType::MODEL_MATRIX:
                glUniformMatrix4fv(u.location, 1, GL_FALSE, (GLfloat*) &model);
                break;
            case AutoUniformType::MVP_MATRIX:
                glUniformMatrix4fv(u.location, 1, GL_FALSE, (GLfloat*) &mvp);
                break;
            default:
                invalid_path("ERROR");
                break;
        }
    }
}



// Auto mesh
struct AutoMesh
{
    MeshGPUBuffer buffer;
    DynArr<MeshVao> meshVaos;
};

void init(AutoMesh* mesh, MeshData* meshData, Allocator* alloc)
{
    init(&mesh->buffer, meshData, alloc); 
    mesh->meshVaos.init(alloc, 4);
}

void shutdown(AutoMesh* mesh)
{
    for (MeshVao& m : mesh->meshVaos) {
        shutdown(&m);
    }
    mesh->meshVaos.shutdown();
    shutdown(&mesh->buffer);
}

bool isCompatible(MeshVao* meshVao, AutoShaderProgram* p)
{
    // Check if vao has enough attribs
    if (meshVao->attribLocs.size() < p->attribLocs.size())
        return false;

    // Loop over all shader attributes
    int meshIndex = 0;
    int shaderIndex = 0;
    bool quit = false;
    while (!quit) 
    {
        AttribLocation* meshLoc = &meshVao->attribLocs[meshIndex];
        AttribLocation* shaderLoc = &p->attribLocs[shaderIndex];
        // If it fits, advance both indices
        if (meshLoc->location == shaderLoc->location &&
                meshLoc->attrib == shaderLoc->attrib) {
            meshIndex++;
            shaderIndex++;
        }
        else { 
            // Advance mesh index, because meshes can 
            // have more data then the shader needs
            meshIndex++; 
        }
        if (meshIndex == meshVao->attribLocs.size() ||
                shaderIndex == p->attribLocs.size()) {
            quit = true;
        }
    }

    if (shaderIndex == p->attribLocs.size()) {
        return true;
    }
    return false;
}

void draw(AutoMesh* mesh, AutoShaderProgram* p)
{
    if (p->program.id == 0) {
        return;
    }
    bind(p);

    // Loop through vaos if one fits
    for (MeshVao& meshVao : mesh->meshVaos)
    {
        if (isCompatible(&meshVao, p)) {
            draw(&meshVao, mesh->buffer.indexBuffer.indexCount);            
            return;
        }
    }

    // Else create new vao
    MeshVao meshVao;
    int attribLocCount = p->attribLocs.size();
    AttribLocation* attribLocs = (AttribLocation*)p->attribLocs.data.data;
    init(&meshVao, &mesh->buffer, attribLocCount, attribLocs, p->program.alloc);
    mesh->meshVaos.push_back(meshVao);

    draw(&meshVao, mesh->buffer.indexBuffer.indexCount);
}








#endif
