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
    //int arr[] = {1, 2, 4, 5, 6};    
    //for (int& i : arr) {
    //    cout << i << endl;
    //    i = 5;
    //}
    //cout << endl << "NEW: " << endl;
    //for (int& i : arr) {
    //    cout << i << endl;
    //}

    cin.ignore();
    return 0;
}
