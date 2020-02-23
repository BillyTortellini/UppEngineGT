#ifndef __MATERIAL_RENDERER_HPP__
#define __MATERIAL_RENDERER_HPP__

struct Material
{
    // Phong shading parameters
    vec3 albedo;
    float specular;
};

struct DirLight
{
    vec3 dir;
};

struct Lighting
{
    DirLight dirLight;
    vec3 ambientColor;
    float ambientStrength;
};

struct DrawRequest
{
    DrawRequest() {}
    DrawRequest(AutoMesh* m, const Transform& t) :
        mesh(m), transform(t) {}
    AutoMesh* mesh;
    Transform transform;
};

struct MaterialRenderer
{
    AutoShaderProgram materialShader;
    Camera3D* camera;
    DynArr<DrawRequest> drawRequests;
    Lighting lighting;
    Material defaultMaterial;
};

void init(MaterialRenderer* r, Camera3D* camera, Allocator* alloc)
{
    init(&r->materialShader, {"material/phong.vert", "material/phong.frag"}, alloc);
    r->drawRequests.init(alloc, 16);
    r->camera = camera;

    // Init lighting
    r->lighting.dirLight.dir = normalize(vec3(-0.2f, -0.8f, -0.4f));
    r->lighting.ambientColor = vec3(1);
    r->lighting.ambientStrength = 0.15f;
    // Init default material
    r->defaultMaterial.albedo = vec3(1, 0, 0);
    r->defaultMaterial.specular = 0.0f;
}

void shutdown(MaterialRenderer* r) {
    shutdown(&r->materialShader);
    r->drawRequests.shutdown();
}

void draw(MaterialRenderer* r, AutoMesh* m, vec3 pos) {
    r->drawRequests.push_back(DrawRequest(m, Transform(pos)));
}

void render(MaterialRenderer* r, GameState* gameState) 
{
    vec2 mousePos = vec2((float)gameState->input.mouseX/gameState->windowState.width, 
            (float)gameState->input.mouseY/gameState->windowState.height);

    bind(&r->materialShader);
    setUniform(&r->materialShader, "u_lightDir", r->lighting.dirLight.dir);
    setUniform(&r->materialShader, "u_ambient", r->lighting.ambientColor * r->lighting.ambientStrength);
    setUniform(&r->materialShader, "u_albedo", r->defaultMaterial.albedo);
    for (DrawRequest& request : r->drawRequests) {
        draw(request.mesh, &r->materialShader, r->camera, mousePos, (float)gameState->time.now, request.transform);
    }
    r->drawRequests.reset();
}










#endif
