#ifndef __VECTORS_HPP__
#define __VECTORS_HPP__

// ---------------
// --- VECTORS ---
// ---------------
// Up to 4 dimensional vectors and typical operations
// of these vectors are defined here.
// Vectors are immutable


#define NORMALIZE_SAVE_MIN 0.000001f

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
        return vec4(1.0f, 0.0f, 0.0f, 0.0f); // TODO Consider if this should be the nullvector
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





#endif
