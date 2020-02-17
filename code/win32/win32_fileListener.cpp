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
#include "..\fileListener.hpp"
#include "../tmpAlloc.hpp"
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
    bool deleteFlag;
    bool overlapActive;
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
    int deleteCount = 0;
    int deleteIndices[MAX_FILE_LISTENERS];
    ListenerToken tokenCounter;
};

// GLOBALS
FileListenerTracker tracker;
Allocator* listenerAlloc;

void initFileListener(Allocator* alloc)
{
    listenerAlloc = alloc;
    tracker.count = 0;
    tracker.tokenCounter = 0;
}

void requestDirectoryChanges(FileListener* listener) 
{
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

    listener->overlapActive = true;
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
        *(listener->overlap) = {}; // TODO: CHekc if works
        listener->notifyBufferBlk = listenerAlloc->alloc(NOTIFYBUFFER_SIZE);
        listener->notifyBuffer = (DWORD*) listener->notifyBufferBlk.data;
        listener->deleteFlag = false;
        listener->overlapActive = false;
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

    tracker.listeners[index].deleteFlag = true;
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
        listener->overlapActive = false;
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

    *(listener->overlap) = {};
    if (!listener->deleteFlag) {
        requestDirectoryChanges(listener);
    }

    return found;
}

int compareIntDesc(const void* ap, const void* bp)
{
    int a = *((int*)ap);
    int b = *((int*)bp);
    if (a == b) return 0;
    if (a > b) return -1;
    if (a < b) return 1;
    return 0;
}

void checkFilesChanged()
{
    // Loop over all Listeners
    for (int i = 0; i < tracker.count; i++)
    {
        FileListener* listener = &(tracker.listeners[i]);
        if (checkFileChanged(listener) && !listener->deleteFlag) {
            listener->callback(listener->filename, listener->userData);
        }

        if (listener->deleteFlag && !listener->overlapActive)
        {
            tracker.deleteIndices[tracker.deleteCount] = i;
            tracker.deleteCount++;
        }
    }

    // Shutdown all flagged listeners
    for (int i = 0; i < tracker.deleteCount; i++)
    {
        int index = tracker.deleteIndices[i];
        FileListener* listener = &(tracker.listeners[index]);
        listenerAlloc->dealloc(listener->dirBlk);
        listenerAlloc->dealloc(listener->filenameBlk);
        listenerAlloc->dealloc(listener->notifyBufferBlk);
        listenerAlloc->dealloc(listener->overlapBlk);
    }

    // Move all flagged listeners to the back
    // Sort indices first, so that swapping does not do something bad
    assert(tracker.deleteCount < tracker.count, "Deletecount wrong\n");
    qsort(tracker.deleteIndices, tracker.deleteCount, sizeof(int), compareIntDesc);
    for (int i = 0; i < tracker.deleteCount; i++) {
        int index = tracker.deleteIndices[i];
        tracker.listeners[index] = tracker.listeners[tracker.count - 1];
        tracker.count--;
    }
    tracker.deleteCount = 0;
}















#endif
