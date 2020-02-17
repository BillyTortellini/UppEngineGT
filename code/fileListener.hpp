#ifndef __FILELISTENER_HPP__
#define __FILELISTENER_HPP__

// Typedefs
#define INVALID_TOKEN -1
typedef int ListenerToken;
typedef void (*listenerCallbackFunc)(const char* filename, void* userData);

// Prototypes
void initFileListener(Allocator* alloc);
void checkFilesChanged();
ListenerToken createFileListener(const char* path, listenerCallbackFunc callback, void* userData);
void deleteFileListener(ListenerToken token);

#endif
