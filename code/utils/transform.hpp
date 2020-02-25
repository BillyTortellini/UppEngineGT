#ifndef __TRANSFORM_HPP__
#define __TRANSFORM_HPP__

struct Transform
{
    Transform(){}
    Transform(const vec3& p) : pos(p) {}
    mat4 toModelMat() const {
        return translate(pos);
    }

    mat3 toNormalMat() const {
        return mat3(1);
    }

    vec3 pos;
};




#endif
