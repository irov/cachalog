#ifndef CACHALOG_DATE_H_
#define CACHALOG_DATE_H_

#include "cachalog_config/cachalog_config.h"

typedef struct ch_date_t
{
    uint32_t year;
    uint32_t mon;
    uint32_t mday;
    uint32_t hour;
    uint32_t min;
    uint32_t sec;
    uint32_t msec;
} ch_date_t;

void ch_date( ch_date_t * _date );

#endif
