#ifndef CH_MESSAGE_H_
#define CH_MESSAGE_H_

#include "ch_config.h"

#include "hb_utils/hb_time.h"
#include "hb_utils/hb_ring.h"

#define CH_MESSAGE_TEXT_MIN 64
#define CH_MESSAGE_TEXT_MAX 4096

typedef struct ch_message_t
{
    HB_RING_DECLARE( base, ch_message_t );

    hb_time_t created_timestamp;
    hb_size_t capacity;

    char text[];
} ch_message_t;

#endif