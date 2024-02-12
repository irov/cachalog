#ifndef CH_RECORD_H_
#define CH_RECORD_H_

#include "ch_config.h"

#include "ch_message.h"
#include "ch_attribute.h"
#include "ch_tag.h"

#include "hb_utils/hb_ring.h"

#define CH_TOKEN_SIZE 32
#define CH_PROJECT_MINLEN 3
#define CH_PROJECT_MAXLEN 7
#define CH_CMD_MAXLEN 16


#define CH_RECORD_USER_ID_MAX 63
#define CH_RECORD_SERVICE_MAX 15
#define CH_RECORD_THREAD_MAX 15
#define CH_RECORD_BUILD_ENVIRONMENT_MAX 15
#define CH_RECORD_BUILD_VERSION_MAX 31
#define CH_RECORD_DEVICE_MODEL_MAX 31
#define CH_RECORD_OS_FAMILY_MAX 15
#define CH_RECORD_OS_VERSION_MAX 15

#define CH_RECORD_ATTRIBUTES_MAX 20
#define CH_RECORD_TAGS_MAX 16

typedef enum ch_record_attributes_flag_e
{
    CH_RECORD_ATTRIBUTE_NONE,
    CH_RECORD_ATTRIBUTE_USER_ID,
    CH_RECORD_ATTRIBUTE_LEVEL,
    CH_RECORD_ATTRIBUTE_SERVICE,
    CH_RECORD_ATTRIBUTE_THREAD,
    CH_RECORD_ATTRIBUTE_MESSAGE,
    CH_RECORD_ATTRIBUTE_TIMESTAMP,
    CH_RECORD_ATTRIBUTE_LIVE,
    CH_RECORD_ATTRIBUTE_BUILD_ENVIRONMENT,
    CH_RECORD_ATTRIBUTE_BUILD_RELEASE,
    CH_RECORD_ATTRIBUTE_BUILD_VERSION,
    CH_RECORD_ATTRIBUTE_BUILD_NUMBER,
    CH_RECORD_ATTRIBUTE_DEVICE_MODEL,
    CH_RECORD_ATTRIBUTE_OS_FAMILY,
    CH_RECORD_ATTRIBUTE_OS_VERSION,
    CH_RECORD_ATTRIBUTE_ATTRIBUTES,
    CH_RECORD_ATTRIBUTE_TAGS,
} ch_record_attributes_flag_e;

#define CH_GET_RECORD_FLAG(TAG) (1LL << TAG)
#define CH_HAS_RECORD_FLAG(FLAG, TAG) (FLAG & CH_GET_RECORD_FLAG(TAG))

typedef struct ch_record_t
{
    HB_RING_DECLARE( ch_record_t );

    hb_time_t created_timestamp;
    uint64_t id;

    uint64_t flags;

    char project[CH_PROJECT_MAXLEN + 1];
    char user_id[CH_RECORD_USER_ID_MAX + 1];
    uint32_t level;

    char service[CH_RECORD_SERVICE_MAX + 1];
    char thread[CH_RECORD_THREAD_MAX + 1];

    const ch_message_t * message;

    uint64_t timestamp;
    uint64_t live;

    char build_environment[CH_RECORD_BUILD_ENVIRONMENT_MAX + 1];
    hb_bool_t build_release;
    char build_version[CH_RECORD_BUILD_VERSION_MAX + 1];
    uint64_t build_number;

    char device_model[CH_RECORD_DEVICE_MODEL_MAX + 1];

    char os_family[CH_RECORD_OS_FAMILY_MAX + 1];
    char os_version[CH_RECORD_OS_VERSION_MAX + 1];

    const ch_attribute_t * attributes[CH_RECORD_ATTRIBUTES_MAX];
    const ch_tag_t * tags[CH_RECORD_TAGS_MAX];
} ch_record_t;

#endif