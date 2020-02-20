#ifndef __WIN32_FILELISTENER_CPP__
#define __WIN32_FILELISTENER_CPP__

// How does this shit work again on Windows?
// -----------------------------------------
// 1. Works on directory level, not on file level
// 2. Call CreateFile with directory name
// 3. Loop:
//     - ReadDirectoryChangesW to request a asynchronous watching of one directory
//     - Use GetOverlappedResults to check if files have changed

// INCLUDES
#include "../fileListener.hpp"
#include "../utils/tmpAlloc.hpp"
#include "../fileIO.hpp"
#include <cstdlib>

// MAKROS
#define MAX_FILE_LISTENERS 64
#define NOTIFYBUFFER_SIZE 2048

// STRUCTS
struct FileListener
{
    ListenerToken token;
    listenerCallbackFunc callback;
    HANDLE file; 
    void* userData;
    char* dir;
    char* filename;
    OVERLAPPED* overlap;
    DWORD* notifyBuffer;
    Blk dirBlk, filenameBlk;
    Blk notifyBufferBlk;
    Blk overlapBlk;
};

struct FileListenerTracker
{
    int count;
    FileListener listeners[MAX_FILE_LISTENERS];
    ListenerToken tokenCounter;
};

// GLOBALS
FileListenerTracker tracker;
Allocator* listenerAlloc;

void printFileListeners() 
{
    loggf("FileListenerCount: %d\n", tracker.count);
    for (int i = 0; i < tracker.count; i++) {
        loggf("\t#%d: %s, path: %s\n", i, tracker.listeners[i].filename, tracker.listeners[i].dir);
    }
}

void initFileListener(Allocator* alloc)
{
    listenerAlloc = alloc;
    tracker.count = 0;
    tracker.tokenCounter = 0;
}

void requestDirectoryChanges(FileListener* listener) 
{
    *(listener->overlap) = {};
    bool success = ReadDirectoryChangesW(
            listener->file,
            listener->notifyBuffer,
            NOTIFYBUFFER_SIZE,
            FALSE,
            FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION,
            //FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
            NULL,
            listener->overlap,
            NULL);

    if (!success) {
        invalid_path("ReadDirectoryChanges failed!");
    }
}

ListenerToken createFileListener(
        const char* path, 
        listenerCallbackFunc callback, 
        void* userData)
{
    // Check if max listeners is reached
    if (tracker.count+1 == MAX_FILE_LISTENERS) {
        loggf("Create file listener max listener reached!\n");
        return INVALID_TOKEN; 
    }

    // Check if file exists
    assert(file_exists(path), 
            "CreateListener called with non existing file %s\n", path);

    // Extract directory and filename from path 
    char filename[512];
    char directory[512];
    {
        int len = (int)strlen(path);
        assert(len < 512, "Create file listener was bigger than 512 byte\n");

        int lastSlash = -15;
        for (int i = len-1; i >= 0; i--) {
            if (path[i] == '/' || path[i] == '\\') {
                lastSlash = i;
                break;
            }
        }

        if (lastSlash == -15) { // If no slash was found dir = ./
            strcpy(filename, path);
            strcpy(directory, (char*) ".\\");
        }
        else {
            strcpy(filename, &path[lastSlash+1]);
            strcpy(directory, path);
            directory[lastSlash] = '\0';
        }
    }

    // Get FileListener from tracker
    FileListener* listener;
    {
        if (tracker.count + 1 >= MAX_FILE_LISTENERS) {
            return INVALID_TOKEN;
        }

        listener = &(tracker.listeners[tracker.count]);
        tracker.count++;
    }

    // Set FileListener values
    {
        listener->token = tracker.tokenCounter++;
        listener->callback = callback;
        listener->userData = userData;
        listener->overlapBlk = listenerAlloc->alloc(sizeof(OVERLAPPED));
        listener->overlap = (OVERLAPPED*) listener->overlapBlk;
        listener->notifyBufferBlk = listenerAlloc->alloc(NOTIFYBUFFER_SIZE);
        listener->notifyBuffer = (DWORD*) listener->notifyBufferBlk.data;
        // Copy filename and dir
        listener->filenameBlk = listenerAlloc->alloc(strlen(filename)+1);
        listener->dirBlk = listenerAlloc->alloc(strlen(directory)+1);
        listener->dir = (char*) listener->dirBlk.data;
        listener->filename = (char*) listener->filenameBlk.data;
        strcpy(listener->dir, directory);
        strcpy(listener->filename, filename);
    }

    // Create file handle
    {
        listener->file = CreateFile(
                listener->dir, 
                GENERIC_READ, 
                FILE_SHARE_READ, 
                NULL, 
                OPEN_EXISTING, 
                FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, 
                NULL); 

        if (listener->file == INVALID_HANDLE_VALUE) {
            invalid_path("CreateFile failed\n");
        }
    }

    // Request changes for this filelistener
    requestDirectoryChanges(listener);

    return listener->token;
}

void deleteFileListener(ListenerToken token) 
{
    // Find index in listener array
    int index = -1;
    for (int i = 0; i < tracker.count; i++) {
        if (tracker.listeners[i].token == token) {
            index = i;
            break;
        }
    }
    assert(index != -1, "DeleteFileListener called with invalid value\n");

    // Delete file request
    CancelIo(tracker.listeners[index].file);

    // Special case
    if (tracker.count == 1) {
        tracker.count = 0;
        return;
    }

    int from = tracker.count-1;
    tracker.listeners[index] = tracker.listeners[from];
    tracker.count = tracker.count - 1;
}

bool checkFileChanged(FileListener* listener)
{
    // Check if changes happened
    {
        DWORD bytesReturned;
        bool result = GetOverlappedResult(
                listener->file, 
                listener->overlap, 
                &bytesReturned, 
                false);

        if (!result) 
        {
            DWORD error = GetLastError();
            if (error == ERROR_IO_INCOMPLETE) {
                return false;
            }
            invalid_path("GetOverlappedResult returned invalid error\n");
        }
        assert(bytesReturned != 0, "GetoverlappedResult returned 0\n");
    }

    bool quit = false;
    bool found = false;
    int nextEntryOffset = 0;
    // Loop over all notifications
    while(!quit)
    {
        // Get Info structure from buffer
        _FILE_NOTIFY_INFORMATION& info = *((_FILE_NOTIFY_INFORMATION*)
                ((byte*)listener->notifyBuffer + nextEntryOffset));

        assert(info.FileNameLength != 0, "Filenamelength = 0\n");

        // Convert wide to utf8
        char filename[256];
        int res = WideCharToMultiByte(
                CP_UTF8, NULL, 
                info.FileName, 
                info.FileNameLength, 
                filename, 
                256, NULL, NULL);
        assert(res != 0, "WideCharToMultibyte failed\n");

        // Add null terminator
        filename[info.FileNameLength/sizeof(wchar_t)] = '\0';
        //char* changeType = "";
        //#define testf(x) case x: changeType = #x;break;
        //switch(info.Action)
        //{
        //    testf(FILE_ACTION_ADDED);
        //    testf(FILE_ACTION_REMOVED);
        //    testf(FILE_ACTION_MODIFIED);
        //    testf(FILE_ACTION_RENAMED_OLD_NAME);
        //    testf(FILE_ACTION_RENAMED_NEW_NAME);
        //}
        //loggf("Changes %s happened to file: %s\n", changeType, filename);

        if (strcmp(filename, listener->filename) == 0) {
            found = true;
            break;
        }

        nextEntryOffset += info.NextEntryOffset;
        if (info.NextEntryOffset == 0) break;
    }
    requestDirectoryChanges(listener);

    return found;
}

void checkFilesChanged()
{
    // Loop over all Listeners
    for (int i = 0; i < tracker.count; i++)
    {
        FileListener* listener = &(tracker.listeners[i]);
        if (checkFileChanged(listener)) {
            loggf("Calling callback of file: %s\n", listener->filename);
            listener->callback(listener->filename, listener->userData);
        }
    }
}















#endif
