#ifndef __UMATH_HPP__
#define __UMATH_HPP__

#include <cmath>

// Defines
#define PI 3.14159265359

// SCALARS
double max(double a, double b) {
    return a > b ? a : b;
}

double min(double a, double b) {
    return a < b ? a : b;
}

double clamp(double x, double minimum, double maximum) {
    return min(max(x, minimum), maximum);
}

// VEC 2
struct vec2
{
    vec2(){}
    vec2(float s) : x(s), y(s) {}
    vec2(float x, float y) : x(x), y(y) {}

    float x, y;

    vec2& operator+=(const vec2& v) {
        this->x += v.x;
        this->y += v.y;
        return *this;
    }
};


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
float dist(const vec2& v1, const vec2& v2) {
    return length(v1-v2);
}
vec2 normalize(const vec2& v) {
    return v / length(v);
}
vec2 normalizeSafe(const vec2& v) {
    float l = length(v);
    if (l < 0.000001f) {
        return vec2(0.0f);
    }
    else {
        return v / l;
    }
}
float dot(const vec2& v1, const vec2& v2) {
    return v1.x*v2.x + v1.y*v2.y;
}

#endif
