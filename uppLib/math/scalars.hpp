#ifndef __SCALARS_HPP__
#define __SCALARS_HPP__

// ------------------------
// --- SCALAR FUNCTIONS ---
// ------------------------
// Take one or multiple scalars and produce another scalar

// Basic functions
template<typename T>
T max(T a, T b) {
    return a > b ? a : b;
}

template<typename T>
T min(T a, T b) {
    return a < b ? a : b;
}

template<typename T>
T abs(T a) {
    return max(-a, a);
}

template<typename T>
T clamp(T x, T minimum, T maximum) {
    return min(max(x, minimum), maximum);
}

template<typename T>
T lerp(T t1, T t2, float a) {
    return t1 * (1.0f-a) + a * t2;
}

// Rounding functions

// Rounds down to next lower multiple
u64 floor(u64 x, u64 m) { 
    return x - x % m;
}

// Rounds up to next higher multiple
u64 ceil(u64 x, u64 m) {
    if (x % m == 0) {
        return x;
    }
    return x + (m - x % m);
}



// Intervals
template<typename T>
struct Interval
{
    Interval(){};
    Interval(T min, T max) : min(min), max(max) {}
    T min, max;
};

template<typename T>
bool inIntervalOpen(T x, const Interval<T>& i) {
    return x > i.min && x < i.max;
}

template<typename T>
bool inIntervalClosed(T x, const Interval<T>& i) {
    return x >= i.min && x <= i.max;
}

template<typename T>
bool noOverlap(const Interval<T>& a, const Interval<T>& b) {
    return a.max < b.min || b.max < a.min;
}

template<typename T>
bool overlap(Interval<T> a, Interval<T> b) {
    return !noOverlap(a, b);
}

template<typename T>
bool inside(const Interval<T> x, Interval<T> surrounding) {
    return x.min >= surrounding.min && x.max <= surrounding.max;
}



// MODULO FUNCTIONS
// These are used because the c++ % operator is actually the remainder, not modulo

int mod(int x, int m) {
    return (x % m + x) % m;
}

float mod(float x, float m) {
    return x - m*floor(x/m);
}

// Modulo in an interval
float modInt(float x, float min, float max) {
    return mod(x - min, max - min) + min;
}


// TRIGONOMETRIC FUNCTIONS
// Are defined in cmath, these are just some helper functions

// Radians to degree
float radians2degree(float r) {
    return r / PI * 180.0f;
}
// degreeToRadians
float degree2radians(float d) {
    return d / (180.0f) * PI;
}

#define r2d(x) radians2degree((x))
#define d2r(x) degree2radians((x))


#endif
