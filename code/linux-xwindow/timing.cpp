#include <sys/prctl.h>
#include <time.h>
#include <errno.h>

// Sets timerslack
void init_timing()
{
    prctl(PR_SET_TIMERSLACK, 1000);
}

// Sleeps in microseconds
void microsleep(int us) {
    timespec req, rem;
    req.tv_sec = (us / 1000000);
    req.tv_nsec = (us % 1000000)*1000;
    int res = nanosleep(&req, &rem);
    while (res < 0) {
        if (errno != EINTR) {
            return;
        }
        req = rem;
        nanosleep(&req, &rem);
    }
}

// Time in microseconds
uint64 getTime() {
    timespec t;
    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    uint64 res = 1000000 * t.tv_sec + t.tv_nsec / 1000;
    return res;
}

// Tick in cpu ticks
uint64 getTick() {
    return 0;
}
