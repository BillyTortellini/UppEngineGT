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
        INVERSE_VIEW_MATRIX,
        NORMAL_MATRIX,
        PROJECTION_MATRIX,
        MVP_MATRIX,
        VP_MATRIX,
        CAMERA_POS,
        RESOLUTION,
        MOUSE_POS,
        TIME,

        COUNT // MUST STAY LAST
    };
}

const char* toStr(AutoUniformType::ENUM type)
{
    using namespace AutoUniformType;
    switch(type)
    {
        case MODEL_MATRIX:
            return "MODEL_MATRIX";
        case VIEW_MATRIX:
            return "VIEW_MATRIX";
        case INVERSE_VIEW_MATRIX:
            return "INVERSE_VIEW_MATRIX";
        case PROJECTION_MATRIX:
            return "PROJECTION_MATRIX";
        case MVP_MATRIX:
            return "MVP_MATRIX";
        case VP_MATRIX:
            return "VP_MATRIX";
        case NORMAL_MATRIX:
            return "NORMAL_MATRIX";
        case CAMERA_POS:
            return "CAMERA_POS";
        case TIME:
            return "TIME";
        case MOUSE_POS:
            return "MOUSE_POS";
        case RESOLUTION:
            return "RESOLUTION";
    }
    return "INVALID_AUTO_UNIFORM";
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

    SupportedAutoUniform("u_inverseview", AutoUniformType::INVERSE_VIEW_MATRIX, GL_FLOAT_MAT3, true), 
    SupportedAutoUniform("u_invview", AutoUniformType::INVERSE_VIEW_MATRIX, GL_FLOAT_MAT3, true), 
    SupportedAutoUniform("u_invviewmat", AutoUniformType::INVERSE_VIEW_MATRIX, GL_FLOAT_MAT3, true), 
    SupportedAutoUniform("u_invviewmatrix", AutoUniformType::INVERSE_VIEW_MATRIX, GL_FLOAT_MAT3, true), 

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

    SupportedAutoUniform("u_normal", AutoUniformType::NORMAL_MATRIX, GL_FLOAT_MAT3, false), 
    SupportedAutoUniform("u_normalmat", AutoUniformType::NORMAL_MATRIX, GL_FLOAT_MAT3, false), 
    SupportedAutoUniform("u_normalmatrix", AutoUniformType::NORMAL_MATRIX, GL_FLOAT_MAT3, false), 
    SupportedAutoUniform("u_nmat", AutoUniformType::NORMAL_MATRIX, GL_FLOAT_MAT3, false), 

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
    
    SupportedAutoUniform("u_resolution", AutoUniformType::RESOLUTION, GL_FLOAT_VEC2, true),
    SupportedAutoUniform("u_res", AutoUniformType::RESOLUTION, GL_FLOAT_VEC2, true),
    SupportedAutoUniform("u_viewport", AutoUniformType::RESOLUTION, GL_FLOAT_VEC2, true),
    SupportedAutoUniform("u_screenSize", AutoUniformType::RESOLUTION, GL_FLOAT_VEC2, true),
    SupportedAutoUniform("u_screen", AutoUniformType::RESOLUTION, GL_FLOAT_VEC2, true),
    SupportedAutoUniform("u_size", AutoUniformType::RESOLUTION, GL_FLOAT_VEC2, true),

    SupportedAutoUniform("u_mousepos", AutoUniformType::MOUSE_POS, GL_FLOAT_VEC2, true),
    SupportedAutoUniform("u_mouse", AutoUniformType::MOUSE_POS, GL_FLOAT_VEC2, true),
    SupportedAutoUniform("u_mpos", AutoUniformType::MOUSE_POS, GL_FLOAT_VEC2, true),
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

void print(AutoShaderProgram* p)
{
    loggf("AutoShaderProgram: \n");
    loggf("PerModel: (count %d)\n", p->perModel.size());
    int i = 0;
    for (AutoUniform& u : p->perModel) {
        loggf("  #%d\n", i);
        loggf("\t Type: %s, location=%d\n", toStr(u.type), u.location);
        i++;
    }
    loggf("PerFrame: (count %d)\n", p->perFrame.size());
    i = 0;
    for (AutoUniform& u : p->perFrame) {
        loggf("  #%d\n", i);
        loggf("\t Type: %s, location=%d\n", toStr(u.type), u.location);
        i++;
    }
    loggf("Attrib locations: (count %d)\n", p->attribLocs.size());
    i = 0;
    for (AttribLocation& loc : p->attribLocs) {
        loggf("  #%d\n", i);
        loggf("\t Attrib: %s, location=%d\n", toStr(loc.attrib), loc.location);
        i++;
    }
}

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
                //loggf("Detected uniform: %s\n", info.name);
                AutoUniform* uniform;
                if (sup.perFrame) {
                    uniform = &p->perFrame[p->perFrame.size()];
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
                //loggf("AttribName found: %s\n", autoAttrib.name);
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
    loggf("AUTO_SHADER_RELOAD\n");
    AutoShaderProgram* p = (AutoShaderProgram*) sp;
    p->perModel.reset();
    p->perFrame.reset();
    p->attribLocs.reset();
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
    p->attribLocs.init(alloc, 4);
    p->lastUpdateFrame = -1;
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

void updatePerFrameUniforms(AutoShaderProgram* program, Camera3D* cam, const vec2& mousePos, float time)
{
    if (program->program.id == 0) {
        return;
    }

    bind(program);
    if (program->lastUpdateFrame == renderState.frameCounter) 
        return;
    program->lastUpdateFrame = renderState.frameCounter;

    mat4 vp = cam->projection * cam->view;
    mat3 inverseView = mat3(vec3(cam->view.columns[0].x, cam->view.columns[0].y, cam->view.columns[0].z),
                            vec3(cam->view.columns[1].x, cam->view.columns[1].y, cam->view.columns[1].z),
                            vec3(cam->view.columns[2].x, cam->view.columns[2].y, cam->view.columns[2].z));
    inverseView = transpose(inverseView);
    for (AutoUniform& u : program->perFrame) 
    {
        switch (u.type)
        {
            case AutoUniformType::VIEW_MATRIX:
                glUniformMatrix4fv(u.location, 1, GL_FALSE, (GLfloat*) &cam->view);
                break;
            case AutoUniformType::INVERSE_VIEW_MATRIX:
                glUniformMatrix3fv(u.location, 1, GL_FALSE, (GLfloat*) &inverseView);
                break;
            case AutoUniformType::PROJECTION_MATRIX:
                glUniformMatrix4fv(u.location, 1, GL_FALSE, (GLfloat*) &cam->projection);
                break;
            case AutoUniformType::VP_MATRIX:
                glUniformMatrix4fv(u.location, 1, GL_FALSE, (GLfloat*) &vp);
                break;
            case AutoUniformType::CAMERA_POS:
                glUniform3fv(u.location, 1, (GLfloat*) &cam->pos);
                break;
            case AutoUniformType::TIME:
                glUniform1f(u.location, time);
                break;
            case AutoUniformType::RESOLUTION:
                {
                vec2 resolution = vec2(renderState.viewportWidth, renderState.viewportHeight);
                glUniform2fv(u.location, 1, (GLfloat*)&resolution);
                break;
                }
            case AutoUniformType::MOUSE_POS: 
                glUniform2fv(u.location, 1, (GLfloat*)&mousePos);
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
    mat4 mvp = cam->projection * cam->view * model;
    mat3 normalView = mat3(cam->view.getDataPtr()); // This is wrong, because it is wrong
    mat3 normal = normalView * transform.toNormalMat();
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
            case AutoUniformType::NORMAL_MATRIX:
                invalid_path("Normal matrix not yet defined\n");
                glUniformMatrix3fv(u.location, 1, GL_FALSE, (GLfloat*) &normal);
                break;
            default:
                invalid_path("ERROR");
                break;
        }
    }
}

void updateAutoUniforms(AutoShaderProgram* p, Camera3D* cam, const vec2& mousePos, float time, const Transform& transform)
{
    updatePerFrameUniforms(p, cam, mousePos, time);
    updatePerModelUniforms(p, cam, transform);
}

void setUniform(AutoShaderProgram* p, const char* name, const float& f) {
    setUniform(&p->program, name, f);
}
void setUniform(AutoShaderProgram* p, const char* name, const vec2& f) {
    setUniform(&p->program, name, f);
}
void setUniform(AutoShaderProgram* p, const char* name, const vec3& f) {
    setUniform(&p->program, name, f);
}
void setUniform(AutoShaderProgram* p, const char* name, const vec4& f) {
    setUniform(&p->program, name, f);
}
void setUniform(AutoShaderProgram* p, const char* name, const mat2& f) {
    setUniform(&p->program, name, f);
}
void setUniform(AutoShaderProgram* p, const char* name, const mat3& f) {
    setUniform(&p->program, name, f);
}
void setUniform(AutoShaderProgram* p, const char* name, const mat4& f) {
    setUniform(&p->program, name, f);
}
void setUniform(AutoShaderProgram* p, const char* name, const int& f) {
    setUniform(&p->program, name, f);
}
void setUniform(AutoShaderProgram* p, const char* name, const u32& f) {
    setUniform(&p->program, name, f);
}



#endif
