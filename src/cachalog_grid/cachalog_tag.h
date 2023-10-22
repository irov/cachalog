#ifndef CACHALOG_TAG_H_
#define CACHALOG_TAG_H_

#include "cachalog_config/cachalog_config.h"
#include "cachalog_utils/cachalog_ring.h"

#define CH_TAG_VALUE_MAX 32

typedef struct ch_tag_t
{
    CH_RING_DECLARE( ch_tag_t );

    ch_time_t created_timestamp;

    char value[CH_TAG_VALUE_MAX];

} ch_tag_t;

#endif