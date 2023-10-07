#include "cachalog_date.h"
#include "cachalog_clock.h"

#include <time.h>

void ch_date( ch_date_t * _date )
{
    time_t t = time( CH_NULLPTR );
    struct tm * tm = localtime( &t );

    _date->year = tm->tm_year + 1900;
    _date->mon = tm->tm_mon + 1;
    _date->mday = tm->tm_mday;
    _date->hour = tm->tm_hour;
    _date->min = tm->tm_min;
    _date->sec = tm->tm_sec;

    ch_clock_t msec;
    ch_clock_msec( &msec );

    _date->msec = msec;
}