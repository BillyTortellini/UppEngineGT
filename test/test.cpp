#include <iostream>

using std::cout;
using std::endl;
using std::cin;

struct alignas(16) vec4
{
    float x, y, z, w;
};

void printVec(vec4* v) {
    cout << "X/Y/Z/W:" << v->x << " " <<v->y << " " <<v->y << " " <<v->z << endl;
}

int main(int argc, char** argv)
{
    cout << "Hello there" << endl; 

    cin.ignore();



    return 0;
}
