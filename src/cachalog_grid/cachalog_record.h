#ifndef CACHALOG_RECORD_H_
#define CACHALOG_RECORD_H_

#include "cachalog_config/cachalog_config.h"
#include "cachalog_utils/cachalog_ring.h"

#include "cachalog_message.h"
#include "cachalog_attribute.h"

#define CH_RECORD_ATTRIBUTES_MAX 20

typedef struct ch_record_t
{
    CH_RING_DECLARE( ch_record_t );

    ch_time_t created_timestamp;
    uint64_t id;
    uint64_t rnd;

    char service[16];
    char user_id[64];
    const ch_message_t * message;
    char category[16];
    uint32_t level;
    uint64_t timestamp;
    uint64_t live;
    char build_environment[8];
    ch_bool_t build_release;
    char build_version[16];
    uint64_t build_number;
    char device_model[32];
    char os_family[16];
    char os_version[16];

    const ch_attribute_t * attributes[CH_RECORD_ATTRIBUTES_MAX];
} ch_record_t;

#endif