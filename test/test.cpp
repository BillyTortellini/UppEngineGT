#include <iostream>

using std::cout;
using std::endl;
using std::cin;

#define COMB2(x) 
#define COMB(x) NEXT(TWO(x))
#define TWO(x) (x+x)
#define NEXT(x) (x+1)
#define PRINT(x) cout << #x " is: " << x << endl


#include <functional>
class ScopeExit
{
public:
    ScopeExit(std::function<void()> f) {
        this->f = f;
    }
    ~ScopeExit() {
        f();
    }
    std::function<void()> f;
};

#define CONCAT_ARGS_(x, y) x ## y
#define CONCAT_ARGS(x, y) CONCAT_ARGS_(x, y)
#define SCOPE_EXIT(code) ScopeExit CONCAT_ARGS(__scope_exit_var_, __LINE__)([&](){code;});

struct Blk
{
    Blk(){}
    Blk(void* data, int size): data(data), size(size) {}
    void* data;
    int size;
    template<typename T>
    explicit operator T*(){return (T*)data;}
};

namespace MeshAttrib
{
    enum ENUM
    {
        SS,
        AA,
        DD,
        FF
    };
};

int main(int argc, char** argv)
{
    {
        //SCOPE_EXIT(cout << "Hello" << endl;);
        //SCOPE_EXIT(cout << "something" << endl;);
        //cout << "Before scope exit" << endl;
    }

    {
        using namespace tst;
    }
    MeshAttrib::POS3

    cout << "worked" << endl;

    //Blk b(new char[256], 256);
    //char* test = (char*)b;
    //cout << "b.data:\t" << b.data << endl << "test:\t" << (void*)test << endl;

    cin.ignore();



    return 0;
}
