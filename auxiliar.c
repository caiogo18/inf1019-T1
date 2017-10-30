#include "auxiliar.h"

#include <unistd.h>
#include <time.h>

void ms_sleep(long milis){
    struct timespec ts;
    ts.tv_sec = milis / 1000;
    ts.tv_nsec = (milis % 1000) * 1000000;
    nanosleep(&ts, NULL);
}
