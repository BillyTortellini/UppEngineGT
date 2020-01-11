#include <sys/inotify.h>
#include <cstdio>

// Returns a token, with which to watch the file
// If return code is negative, some error occured;
int init_filewatcher(const char* filename)
{
    int fd = inotify_init1(IN_NONBLOCK);
    if (fd == -1) {
        printf("inotify init didnt work\n");
        return -1;
    }
    
    int watchdescriptor = inotify_add_watch(
            fd, 
            filename,
            IN_MODIFY);

    if (watchdescriptor < 0) {
        printf("Inotify add watch failed\n");
        return -1;
    }

    return fd;
}

// Reads from a nonblocking file descriptor data into
// buffer buf. Returns true if data was available,
// false if the read would have blocked.
bool readNonBlocking(int fd, byte* buf, int size)
{
    int pos = 0;
    while (pos != size) 
    {
        int len = read(fd, buf + pos, size - pos);
        if (len == 0) { //EOF
            printf("End of file occured, should not happen\n"); 
            return false;
        }
        else if (len < 0) { // Error 
            if (errno == EINTR) {
                continue; // Just try again
            }
            else if (errno == EAGAIN) {
                // There are no events to process
                return false;
            }
            else {
                char* errorstr = strerror(errno);
                printf("Error with read: %s\n", errorstr);
                return false;
            }
        }

        pos += len;
    }

    return true;
}

// Returns true if an event happened in the meantime, 
// otherwise false.
bool hasFileChanged(int token) 
{
    if(token < 0) {
        return false;
    }

    byte eventBuffer[sizeof(inotify_event) + 256];
    inotify_event* event = (inotify_event*) &eventBuffer;

    // Read inotify_event struct
    if (!readNonBlocking(token, 
                eventBuffer,
                sizeof(inotify_event))) {
        return false;
    }

    // Read possibly available string
    if (event->len != 0) 
    {
        if (!readNonBlocking(token,
                    eventBuffer,
                    event->len)) {
            return false;
        }
    }

    return true;
}

