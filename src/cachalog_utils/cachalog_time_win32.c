#include "cachalog_time.h"

#include <time.h>

void ch_time( ch_time_t * _time )
{
    __time64_t t = _time64( CH_NULLPTR );

    *_time = (ch_time_t)t;
}