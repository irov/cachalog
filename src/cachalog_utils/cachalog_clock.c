#include "cachalog_clock.h"

#include <time.h>

void ch_clock_msec( ch_clock_t * _clock )
{
    clock_t c = clock();

    clock_t msec = c / (CLOCKS_PER_SEC / 1000);

    *_clock = msec % 1000;
}