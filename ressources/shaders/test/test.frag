#version 430 core

in vec2 f_uv;
out vec4 o_color;

uniform vec3 u_eye;
uniform mat3 u_invview;
uniform vec2 u_resolution;
uniform float u_time;

float sdfPoint(vec3 p, vec3 pos) {
    return distance(p, pos);
}

float sdfSphere(vec3 p, vec3 pos, float r) {
    return sdfPoint(p, pos) - r;
}

float sdfPlane(vec3 p, vec4 plane) {
    return dot(vec4(p, 1.), plane);
}

float sdfCube(vec3 p, vec3 r) {
    vec3 b = abs(p)-r;
    return length(max(b, 0.)) + min(max(b.x, max(b.y, b.z)), 0.);
}

float sceneSdf(vec3 p) {
    float d = 1000.;
    float halfSphere = max(sdfSphere(p, vec3(0.), 1.3), sdfPlane(p, vec4(0., 1., 0., 0.0)));
    d = min(d, halfSphere);
    d = min(d, sdfSphere(p, vec3(sin(u_time*26.)*.05, .5+sin(u_time*14.23)*.03, sin(u_time*17.32)*.06), .5+sin(u_time)*.05));
    float c = abs(sdfCube(p-vec3(0., .53, 0.), vec3(.5))) -.03;
    float s = sdfSphere(p, vec3(0., .5, -0.0), .65);
    c = max(c, -s);
    //d = min(d, s);
    d = min(d, c);

    // max is intersection, min is union
    // Difference is what? 

    return d;
}

#define CENTRAL_DIFF_VAL 0.005
vec3 sceneNormal(vec3 p) {
    float d = sceneSdf(p);
    vec3 n = vec3(sceneSdf(p+vec3(CENTRAL_DIFF_VAL, 0., 0.))-d,
                  sceneSdf(p+vec3(0., CENTRAL_DIFF_VAL, 0.))-d,
                  sceneSdf(p+vec3(0., 0., CENTRAL_DIFF_VAL))-d);
    n = normalize(n);
    return n;
}


// Returns length of raymarch
#define MAX_STEPS 250
#define MIN_DIST 0.001
#define MAX_DIST 50.
float rayMarch(vec3 ro, vec3 rd) 
{
    float t = 0.;
    float d = sceneSdf(ro);
    if (d < MIN_DIST || d > MAX_DIST) {
        return t;
    }

    for (int i = 0; i < MAX_STEPS; i++) 
    {
        d = sceneSdf(ro + t*rd);
        if (d < MIN_DIST || d > MAX_DIST) {
            break;
        }
        t += d;
    }

    return t;
}

vec3 dirLight(vec3 p, vec3 n, vec3 lDir, vec3 lCol) {
    vec3 lookDir = normalize(p - u_eye);

    // Diffuse
    float diff = max(0., dot(n, -lDir));
    // Specular 
    float spec = pow(max(0., dot(n, normalize(-lookDir + -lDir))), 32.) * .3 * smoothstep(0., 0.1, diff);
    //spec = 0.;
    float shadow = step(MAX_DIST-10., rayMarch(p+n*.01, -lDir));

    return (diff+spec)* lCol * shadow;
}

vec3 pointLight(vec3 p, vec3 n, vec3 lPos, vec3 lCol, vec3 attenuation) 
{
    vec3 lDir = normalize(p - lPos);
    vec3 lookDir = normalize(p - u_eye);
    float ld = distance(lPos, p); // Light distance
    float ls = 1./dot(attenuation, vec3(1., ld, ld*ld)); // Light strength

    vec3 l = vec3(0.); // Light value
    // Diffuse
    l += max(0., dot(-n, lDir)) * lCol;
    // Specular (Blinn-phong)
    float sPow = 1.; // specular power
    float sStr = 0.5;
    // Safety check
    float safe = smoothstep(0., 0.1, dot(n, -lDir));
    l += pow(max(0., dot(n, normalize(-lookDir+-lDir))), sPow) * lCol * sStr * safe;

    // Check if in shadow
    float t = rayMarch(p+n*.15, -lDir);
    ls = mix(ls, 0., step(t,ld-2.));
    // Add light strength
    l = l * ls;

    return l;
}

vec3 shade(vec3 p, vec3 n) {
    vec3 lightPos = vec3(sin(u_time*.5)*5., 3., cos(u_time*.5)*5.);

    vec3 col = vec3(0.);
    col += pointLight(p, n, lightPos, vec3(.5, 1., 1.), vec3(.5, .25, .1));
    col += dirLight(p, n, normalize(vec3(0.32, -0.5, 0.32)), vec3(0.15, 0.082, 0.06));

    return col;
}

vec3 renderScene(vec2 r_uv) 
{
    float a = u_resolution.x/u_resolution.y;
    vec2 uv = r_uv * vec2(max(1., a), min(1., 1./a));

    vec3 ro = u_eye;
    vec3 rd = u_invview * normalize(vec3(uv, -1.));

    float t = rayMarch(ro, rd);
    vec3 p = ro + t*rd;
    vec3 n = sceneNormal(p);

    vec3 col = vec3(.5);
    col = shade(p, n);

    // Gamma correction
    col = pow(col, vec3(1./2.2));
    return col;
}

void main()
{
    vec3 col = vec3(0.);
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            vec2 off = (vec2(float(i), float(j))*.25)/u_resolution*2.;
            col += renderScene(f_uv+off)/4.;
        }
    }
    //col = renderScene(f_uv);
    o_color = vec4(col, 1.);
}
