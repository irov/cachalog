#include "cachalog_time.h"

#include <time.h>

void ch_time( ch_time_t * _time )
{
    time_t t = time( CH_NULLPTR );

    *_time = (ch_time_t)t;
}