#ifndef CACHALOG_ATTRIBUTE_H_
#define CACHALOG_ATTRIBUTE_H_

#include "cachalog_config/cachalog_config.h"

typedef struct ch_attribute_t
{
    struct ch_attribute_t * next;

    char name[32];
    char value[128];

} ch_attribute_t;

#endif