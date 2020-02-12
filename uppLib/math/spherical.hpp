#ifndef __SPHERICAL_HPP__
#define __SPHERICAL_HPP__

// -----------------------------
// --- SPHERICAL COORDINATES ---
// -----------------------------
// Contains functions for transfering spherical to euclidean coordinates

vec3 spherical2euclidean(const vec2& s)
{
    // The s vector is made up of (azimuth angle, polar angle (measured from zenith pointing up))
    float y = sinf(s.y);
    float len_xz = cosf(s.y);
    float z = -cosf(s.x) * len_xz;
    float x = -sinf(s.x) * len_xz;
    return vec3(x, y, z);
}

vec3 spherical2euclidean(const vec3& s)
{
    // The s vector is made up of (azimuth angle, polar angle, radius)
    return spherical2euclidean(vec2(s.x, s.y)) * s.z;
}

#define sp2eu(x) spherical2euclidean((x))

// Puts the azimuth in [-PI, PI) range and the polar angle in [PI/2, PI/2]
vec2 sphericalNorm(const vec2& s) 
{
    vec2 r;
    r.x = modInt(s.x, -PI, PI);
    r.y = clamp(s.y, -PI/2.0f + 0.001f, PI/2.0f - 0.001f);
    return r;
}












#endif

