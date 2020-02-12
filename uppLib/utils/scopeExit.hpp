#ifndef __SCOPED_EXIT__
#define __SCOPED_EXIT__

#include <functional>

template <typename F>
struct ScopeExit {
    ScopeExit(F f) : f(f) {}
    ~ScopeExit() { f(); }
    F f;
};

template <typename F>
ScopeExit<F> makeScopeExit(F f) {
    return ScopeExit<F>(f);
}

#define STRING_JOIN2(arg1, arg2) DO_STRING_JOIN2(arg1, arg2)
#define DO_STRING_JOIN2(arg1, arg2) arg1 ## arg2
#define SCOPE_EXIT(code) \
    auto STRING_JOIN2(_scope_exit_, __LINE__) = makeScopeExit([&](){code;})


#endif
