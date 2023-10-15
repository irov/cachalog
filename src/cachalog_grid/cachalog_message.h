#ifndef CACHALOG_MESSAGE_H_
#define CACHALOG_MESSAGE_H_

#include "cachalog_config/cachalog_config.h"
#include "cachalog_utils/cachalog_ring.h"

#define CH_MESSAGE_TEXT_MIN 64
#define CH_MESSAGE_TEXT_MAX 4096

typedef struct ch_message_t
{
    CH_RING_DECLARE( ch_message_t );
    ch_time_t created_timestamp;
    ch_size_t capacity;

    char text[];
} ch_message_t;

#endif