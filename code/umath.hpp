#ifndef __UMATH_HPP__
#define __UMATH_HPP__

#include <cmath>

// Defines
#define PI 3.14159265359f
#define NORMALIZE_SAVE_MIN 0.000001f

// SCALARS
template<typename T>
inline T max(T a, T b) {
    return a > b ? a : b;
}

template<typename T>
T min(T a, T b) {
    return a < b ? a : b;
}


template<typename T>
T clamp(T x, T minimum, T maximum) {
    return min(max(x, minimum), maximum);
}

template<typename T>
T lerp(T t1, T t2, float a) {
    return t1 * (1.0f-a) + a * t2;
}

// Radians to degree
float r2d(float r) {
    return r / PI * 180.0f;
}
float d2r(float d) {
    return d / (180.0f) * PI;
}

// VEC 2
struct vec2
{
    vec2(){}
    explicit vec2(float s) : x(s), y(s) {}
    vec2(float x, float y) : x(x), y(y) {}
    vec2(int s) : vec2((float)s, (float)s){};
    vec2(int x, int y) : vec2((float)x, (float)y){}

    float x, y;

    vec2& operator+=(const vec2& v) {
        this->x += v.x;
        this->y += v.y;
        return *this;
    }
    vec2& operator-=(const vec2& v) {
        this->x -= v.x;
        this->y -= v.y;
        return *this;
    }
    vec2& operator*=(const vec2& v) {
        this->x *= v.x;
        this->y *= v.y;
        return *this;
    }
    vec2& operator/=(const vec2& v) {
        this->x /= v.x;
        this->y /= v.y;
        return *this;
    }
    vec2& operator+=(float s) {
        this->x += s;
        this->y += s;
        return *this;
    }
    vec2& operator-=(float s) {
        this->x -= s;
        this->y -= s;
        return *this;
    }
    vec2& operator*=(float s) {
        this->x *= s;
        this->y *= s;
        return *this;
    }
    vec2& operator/=(float s) {
        this->x /= s;
        this->y /= s;
        return *this;
    }
};

vec2 operator-(const vec2& v) {
    return vec2(-v.x, -v.y);
}
vec2 operator+(const vec2& v1, const vec2& v2) {
    return vec2(v1.x+v2.x, v1.y+v2.y);
}
vec2 operator-(const vec2& v1, const vec2& v2) {
    return vec2(v1.x-v2.x, v1.y-v2.y);
}
vec2 operator*(const vec2& v1, const vec2& v2) {
    return vec2(v1.x*v2.x, v1.y*v2.y);
}
vec2 operator/(const vec2& v1, const vec2& v2) {
    return vec2(v1.x/v2.x, v1.y/v2.y);
}

vec2 operator+(const vec2& v, float s) {
    return vec2(v.x+s, v.y+s);
}
vec2 operator-(const vec2& v, float s) {
    return vec2(v.x-s, v.y-s);
}
vec2 operator*(const vec2& v, float s) {
    return vec2(v.x*s, v.y*s);
}
vec2 operator/(const vec2& v, float s) {
    return vec2(v.x/s, v.y/s);
}

float length(const vec2& v) {
    return sqrtf(v.x*v.x + v.y*v.y);
}
float lengthSq(const vec2& v) {
    return v.x*v.x + v.y*v.y;
}
float dist(const vec2& v1, const vec2& v2) {
    return length(v1-v2);
}
float distSq(const vec2& v1, const vec2& v2) {
    return lengthSq(v1-v2);
}
vec2 normalize(const vec2& v) {
    return v / length(v);
}
vec2 normalizeSafe(const vec2& v) {
    float l = length(v);
    if (l < NORMALIZE_SAVE_MIN) {
        return vec2(1.0f, 0.0f); // TODO Check if this should be null vector
    }
    else {
        return v / l;
    }
}
float dot(const vec2& v1, const vec2& v2) {
    return v1.x*v2.x + v1.y*v2.y;
}
float cross(const vec2& v1, const vec2& v2) {
    return v1.x*v2.y - v2.x*v1.y;
}
vec2 rot90CW(const vec2& v) {
    return vec2(v.y, -v.x);
}
vec2 rot90CCW(const vec2& v) {
    return vec2(-v.y, v.x);
}

// VEC 3
struct vec3
{
    vec3(){};
    explicit vec3(float s) : x(s), y(s), z(s) {}
    vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    vec3(const vec2& v, float s) : x(v.x), y(v.y), z(s) {}
    vec3(float s, const vec2& v) : x(s), y(v.x), z(v.y) {}

    float x,y,z;

    // Shorcut += -= *= /= functions
    vec3& operator+=(const vec3& v) {
        this->x += v.x;
        this->y += v.y;
        this->z += v.z;
        return *this;
    }
    vec3& operator-=(const vec3& v) {
        this->x -= v.x;
        this->y -= v.y;
        this->z -= v.z;
        return *this;
    }
    vec3& operator*=(const vec3& v) {
        this->x *= v.x;
        this->y *= v.y;
        this->z *= v.z;
        return *this;
    }
    vec3& operator/=(const vec3& v) {
        this->x /= v.x;
        this->y /= v.y;
        this->z /= v.z;
        return *this;
    }
    vec3& operator+=(float s) {
        this->x += s;
        this->y += s;
        this->z += s;
        return *this;
    }
    vec3& operator-=(float s) {
        this->x -= s;
        this->y -= s;
        this->z -= s;
        return *this;
    }
    vec3& operator*=(float s) {
        this->x *= s;
        this->y *= s;
        this->z *= s;
        return *this;
    }
    vec3& operator/=(float s) {
        this->x /= s;
        this->y /= s;
        this->z /= s;
        return *this;
    }
};

vec3 operator-(const vec3& v) {
    return vec3(-v.x, -v.y, -v.z);
}
// Regular arithmetic operations
vec3 operator+(const vec3& v1, const vec3& v2) {
    return vec3(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z);
}
vec3 operator-(const vec3& v1, const vec3& v2) {
    return vec3(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z);
}
vec3 operator*(const vec3& v1, const vec3& v2) {
    return vec3(v1.x*v2.x, v1.y*v2.y, v1.z*v2.z);
}
vec3 operator/(const vec3& v1, const vec3& v2) {
    return vec3(v1.x/v2.x, v1.y/v2.y, v1.z/v2.z);
}

vec3 operator+(const vec3& v, float s) {
    return vec3(v.x+s, v.y+s, v.z+s);
}
vec3 operator-(const vec3& v, float s) {
    return vec3(v.x-s, v.y-s, v.z-s);
}
vec3 operator*(const vec3& v, float s) {
    return vec3(v.x*s, v.y*s, v.z*s);
}
vec3 operator/(const vec3& v, float s) {
    return vec3(v.x/s, v.y/s, v.z/s);
}

// Special vector operations
float length(const vec3& v) {
    return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
}
float lengthSq(const vec3& v) {
    return v.x*v.x + v.y*v.y + v.z*v.z;
}
float dist(const vec3& v1, const vec3& v2) {
    return length(v1-v2);
}
float distSq(const vec3& v1, const vec3& v2) {
    return lengthSq(v1-v2);
}
vec3 normalize(const vec3& v) {
    return v / length(v);
}
vec3 normalizeSafe(const vec3& v) {
    float l = length(v);
    if (l < NORMALIZE_SAVE_MIN) {
        return vec3(1.0f, 0.0f, 0.0f); // TODO Check if this should be null vector
    }
    else {
        return v / l;
    }
}
float dot(const vec3& v1, const vec3& v2) {
    return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}
vec3 cross(const vec3& v1, const vec3& v2) {
    return vec3(v1.y*v2.z - v1.z*v2.y, 
            v1.z*v2.x - v1.x*v2.z,
            v1.x*v2.y - v1.y*v2.x);
}
vec3 homogenize(const vec3& v) {
    return vec3(v.x/v.z, v.y/v.z, 1.0f);
}


// VEC 4
struct vec4
{
    vec4(){};
    explicit vec4(float s) : x(s), y(s), z(s), w(s) {}
    vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    vec4(const vec2& v, float z, float w) : x(v.x), y(v.y), z(z), w(w) {}
    vec4(float x, const vec2& v, float w) : x(x), y(v.x), z(v.y), w(w) {}
    vec4(float x, float y, const vec2& v) : x(x), y(y), z(v.x), w(v.y) {}
    vec4(const vec2& v1, const vec2& v2) : x(v1.x), y(v1.y), z(v2.x), w(v2.y) {}
    vec4(const vec3& v, float w) : x(v.x), y(v.y), z(v.z), w(w) {}
    vec4(float x, const vec3& v) : x(x), y(v.x), z(v.y), w(v.z) {}

    float x,y,z,w;

    // Shorcut += -= *= /= functions
    vec4& operator+=(const vec4& v) {
        this->x += v.x;
        this->y += v.y;
        this->z += v.z;
        this->w += v.w;
        return *this;
    }
    vec4& operator-=(const vec4& v) {
        this->x -= v.x;
        this->y -= v.y;
        this->z -= v.z;
        this->w -= v.w;
        return *this;
    }
    vec4& operator*=(const vec4& v) {
        this->x *= v.x;
        this->y *= v.y;
        this->z *= v.z;
        this->w *= v.w;
        return *this;
    }
    vec4& operator/=(const vec4& v) {
        this->x /= v.x;
        this->y /= v.y;
        this->z /= v.z;
        this->w /= v.w;
        return *this;
    }
    vec4& operator+=(float s) {
        this->x += s;
        this->y += s;
        this->z += s;
        this->w += s;
        return *this;
    }
    vec4& operator-=(float s) {
        this->x -= s;
        this->y -= s;
        this->z -= s;
        this->w -= s;
        return *this;
    }
    vec4& operator*=(float s) {
        this->x *= s;
        this->y *= s;
        this->z *= s;
        this->w *= s;
        return *this;
    }
    vec4& operator/=(float s) {
        this->x /= s;
        this->y /= s;
        this->z /= s;
        this->w /= s;
        return *this;
    }
};

vec4 operator-(const vec4& v) {
    return vec4(-v.x, -v.y, -v.z, -v.w);
}
// Regular arithmetic operations
vec4 operator+(const vec4& v1, const vec4& v2) {
    return vec4(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z, v1.w+v2.w);
}
vec4 operator-(const vec4& v1, const vec4& v2) {
    return vec4(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z, v1.w-v2.w);
}
vec4 operator*(const vec4& v1, const vec4& v2) {
    return vec4(v1.x*v2.x, v1.y*v2.y, v1.z*v2.z, v1.w*v2.w);
}
vec4 operator/(const vec4& v1, const vec4& v2) {
    return vec4(v1.x/v2.x, v1.y/v2.y, v1.z/v2.z, v1.w/v2.w);
}

vec4 operator+(const vec4& v, float s) {
    return vec4(v.x+s, v.y+s, v.z+s, v.w+s);
}
vec4 operator-(const vec4& v, float s) {
    return vec4(v.x-s, v.y-s, v.z-s, v.w-s);
}
vec4 operator*(const vec4& v, float s) {
    return vec4(v.x*s, v.y*s, v.z*s, v.w*s);
}
vec4 operator/(const vec4& v, float s) {
    return vec4(v.x/s, v.y/s, v.z/s, v.w/s);
}

// Special vector operations
float length(const vec4& v) {
    return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w);
}
float lengthSq(const vec4& v) {
    return v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w;
}
float dist(const vec4& v1, const vec4& v2) {
    return length(v1-v2);
}
float distSq(const vec4& v1, const vec4& v2) {
    return lengthSq(v1-v2);
}
vec4 normalize(const vec4& v) {
    return v / length(v);
}
vec4 normalizeSafe(const vec4& v) {
    float l = length(v);
    if (l < NORMALIZE_SAVE_MIN) {
        return vec4(1.0f, 0.0f, 0.0f, 0.0f); // TODO Check if this should be null vector
    }
    else {
        return v / l;
    }
}
float dot(const vec4& v1, const vec4& v2) {
    return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z + v1.w*v2.w; 
}
vec4 homogenize(const vec4& v) {
    return vec4(v.x/v.w, v.y/v.w, v.z/v.w, 1.0f);
}

// -------------------------------
// ----- MATRIX CALCULATIONS -----
// -------------------------------
struct mat2
{
    mat2() {}
    mat2(float s) { // Diagonal matrix with entries s
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
    mat3(float s) {
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

// 
struct mat4
{
    mat4() {}
    mat4(float s) {
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

#endif
