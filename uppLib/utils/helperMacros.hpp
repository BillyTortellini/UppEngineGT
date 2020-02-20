#ifndef __HELPER_MACROS_HPP__
#define __HELPER_MACROS_HPP__

#define NO_COPY_MOVE(type) \
    type(const type& other) = delete; \
    type(type&& other) = delete; \
    type& operator=(const type& other) = delete; \
    type& operator=(type&& other) = delete;






#endif
