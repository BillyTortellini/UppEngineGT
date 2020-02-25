#ifndef __MATRICES_HPP__
#define __MATRICES_HPP__

// -------------------------------
// ----- MATRIX CALCULATIONS -----
// -------------------------------
// Structs and functions of symetrical matricies
// up to 4x4 are contained in this file.

struct mat2
{
    mat2() {}
    explicit mat2(float s) { // Diagonal matrix with entries s
        columns[0] = vec2(s, 0.0f);
        columns[1] = vec2(0.0f, s);
    } 
    mat2(const vec2& v1, const vec2& v2) { // Diagonal matrix with entries s
        columns[0] = v1;
        columns[1] = v2;
    } 

    vec2 columns[2];
};

vec2 operator*(const mat2& m, const vec2& v) {
    return m.columns[0]*v.x + m.columns[1]*v.y;
}

mat2 operator*(const mat2& m1, const mat2& m2) {
    return mat2(m1*m2.columns[0], m1*m2.columns[1]);    
}

mat2 transpose(const mat2& m) {
    return mat2(vec2(m.columns[0].x, m.columns[1].x),
            vec2(m.columns[0].y, m.columns[1].y));
}

mat2 rotate(float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    return mat2(vec2(c, s), vec2(-s, c));
}

mat2 scale(const vec2& s) {
    return mat2(vec2(s.x, 0.0f), vec2(0.0f, s.y));
}

// Mat 3
struct mat3
{
    mat3(){}
    explicit mat3(float s) {
        columns[0] = vec3(s, 0.0f, 0.0f);
        columns[1] = vec3(0.0f, s, 0.0f);
        columns[2] = vec3(0.0f, 0.0f, s);
    }
    mat3(const vec3& v1, const vec3& v2, const vec3& v3) {
        columns[0] = v1;
        columns[1] = v2;
        columns[2] = v3;
    }
    mat3(const mat2& m) {
        columns[0] = vec3(m.columns[0], 0.0f);
        columns[1] = vec3(m.columns[1], 0.0f);
        columns[2] = vec3(0.0f, 0.0f, 1.0f);
    }
    mat3(float* data) {
        columns[0] = vec3(data[0], data[1], data[2]);
        columns[1] = vec3(data[3], data[4], data[5]);
        columns[2] = vec3(data[6], data[7], data[8]);
    }

    vec3 columns[3];
};

vec3 operator*(const mat3& m, const vec3& v) {
    return m.columns[0]*v.x + m.columns[1]*v.y + m.columns[2]*v.z;
}

vec2 operator*(const mat3& m, const vec2& v) {
    vec3 r = m * vec3(v, 1.0f);
    return vec2(r.x, r.y);
}

mat3 operator*(const mat3& m1, const mat3& m2) {
    return mat3(m1*m2.columns[0], m1*m2.columns[1], m1*m2.columns[2]);    
}

mat3 rotate(float yaw, float pitch, float roll) {
    return mat3(0.0f);
}

mat3 scale(const vec3& s) {
    return mat3(vec3(s.x, 0.0f, 0.0f),
            vec3(0.0f, s.y, 0.0f),
            vec3(0.0f, 0.0f, s.z));
}

mat3 translate(const vec2& t) {
    return mat3(vec3(1.0f, 0.0f, 0.0f),
            vec3(0.0f, 1.0f, 0.0f),
            vec3(t.x, t.y, 1.0f));
}

mat3 transpose(const mat3& m) {
    return mat3(vec3(m.columns[0].x, m.columns[1].x, m.columns[2].x),
            vec3(m.columns[0].y, m.columns[1].y, m.columns[2].y),
            vec3(m.columns[0].z, m.columns[1].z, m.columns[2].z));
}

// MATRIX 4x4
struct mat4
{
    mat4() {}
    explicit mat4(float s) {
        columns[0] = vec4(s, 0.0f, 0.0f, 0.0f);
        columns[1] = vec4(0.0f, s, 0.0f, 0.0f);
        columns[2] = vec4(0.0f, 0.0f, s, 0.0f);
        columns[3] = vec4(0.0f, 0.0f, 0.0f, s);
    }
    mat4(const vec4& v1, const vec4& v2, const vec4& v3, const vec4& v4) {
        columns[0] = v1;
        columns[1] = v2;
        columns[2] = v3;
        columns[3] = v4;
    }
    mat4(const mat3& m) {
        columns[0] = vec4(m.columns[0], 0.0f);
        columns[1] = vec4(m.columns[1], 0.0f);
        columns[2] = vec4(m.columns[2], 0.0f);
        columns[3] = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    }
    float* getDataPtr() {
        return (float*)columns;
    }

    vec4 columns[4];
};

vec4 operator*(const mat4& m, const vec4& v) {
    return m.columns[0]*v.x + m.columns[1]*v.y + m.columns[2]*v.z + m.columns[3]*v.w;
}

vec3 operator*(const mat4& m, const vec3& v) {
    vec4 r = m * vec4(v, 1.0f);
    return vec3(r.x, r.y, r.z);
}

mat4 operator*(const mat4& m1, const mat4& m2) {
    return mat4(m1*m2.columns[0], m1*m2.columns[1], m1*m2.columns[2], m1*m2.columns[3]);    
}

mat4 translate(const vec3& t) {
    return mat4(vec4(1.0f, 0.0f, 0.0f, 0.0f),
            vec4(0.0f, 1.0f, 0.0f, 0.0f),
            vec4(0.0f, 0.0f, 1.0f, 0.0f),
            vec4(t.x, t.y, t.z, 1.0f));
}

mat4 transpose(const mat4& m) {
    return mat4(vec4(m.columns[0].x, m.columns[1].x, m.columns[2].x, m.columns[3].x),
            vec4(m.columns[0].y, m.columns[1].y, m.columns[2].y, m.columns[3].y),
            vec4(m.columns[0].z, m.columns[1].z, m.columns[2].z, m.columns[3].z),
            vec4(m.columns[0].w, m.columns[1].w, m.columns[2].w, m.columns[3].w));
}

mat4 projection(float near, float far, float fovX, float aspectRatio) 
{
    mat4 projection;

    float fovY;
    if (aspectRatio > 1.0f) {
        fovY = fovX / aspectRatio;
    }
    else {
        fovY = fovX;
        fovX = fovX * aspectRatio;
    }
    float sx = 1.0f / tanf(fovX/2.0f);
    float sy = 1.0f / tanf(fovY/2.0f);
    projection.columns[0] = vec4(sx, 0.0f, 0.0f, 0.0f);
    projection.columns[1] = vec4(0.0f, sy, 0.0f, 0.0f);
    projection.columns[2] = vec4(0.0f, 0.0f, -(far+near)/(far-near), -1.0f);
    projection.columns[3] = vec4(0.0f, 0.0f, (-2.0f*near*far)/(far-near), 0.0f);

    return projection;
}

mat4 lookInDir(const vec3& pos, const vec3& dir, const vec3& up) 
{
    mat4 view;

    vec3 d = normalizeSafe(-dir);
    vec3 u = normalizeSafe(up);
    vec3 r = normalizeSafe(cross(u, d));
    u = cross(d, r);

    view = mat4(transpose(mat3(r, u, d)));
    view = view * translate(-pos);

    return view;
}

mat4 lookInDir(const vec3& pos, const vec3& dir) 
{
    return lookInDir(pos, dir, vec3(0.0f, 1.0f, 0.0f));
}

mat4 lookAt(const vec3& pos, const vec3& at, const vec3& up) {
    return lookInDir(pos, at - pos, up);
}

mat4 lookAt(const vec3& pos, const vec3& at) {
    return lookInDir(pos, at - pos, vec3(0.0f, 1.0f, 0.0f));
}





#endif __MATRICES_HPP__
