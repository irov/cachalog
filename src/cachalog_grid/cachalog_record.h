#ifndef CACHALOG_RECORD_H_
#define CACHALOG_RECORD_H_

#include "cachalog_config/cachalog_config.h"

#include "cachalog_message.h"
#include "cachalog_attribute.h"

typedef struct ch_record_t
{
    struct ch_record_t * next;

    ch_time_t created_timestamp;

    char service[16];
    char id[64];
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

    const ch_attribute_t * attributes[20];
} ch_record_t;

#endif