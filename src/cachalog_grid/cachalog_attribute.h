#ifndef CACHALOG_ATTRIBUTE_H_
#define CACHALOG_ATTRIBUTE_H_

#include "cachalog_config/cachalog_config.h"
#include "cachalog_utils/cachalog_ring.h"

#define CH_ATTRIBUTE_NAME_MAX 32
#define CH_ATTRIBUTE_VALUE_MAX 128

typedef struct ch_attribute_t
{
    CH_RING_DECLARE( ch_attribute_t );

    struct ch_record_t * record;

    ch_time_t created_timestamp;

    char name[CH_ATTRIBUTE_NAME_MAX];
    char value[CH_ATTRIBUTE_VALUE_MAX];

} ch_attribute_t;

#endif