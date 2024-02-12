#ifndef CH_ATTRIBUTE_H_
#define CH_ATTRIBUTE_H_

#include "ch_config.h"

#include "hb_utils/hb_time.h"
#include "hb_utils/hb_ring.h"

#define CH_ATTRIBUTE_NAME_MAX 31
#define CH_ATTRIBUTE_VALUE_MAX 127

typedef enum ch_attribute_type_t
{
    CH_ATTRIBUTE_TYPE_BOOLEAN,
    CH_ATTRIBUTE_TYPE_INTEGER,
    CH_ATTRIBUTE_TYPE_STRING
} ch_attribute_type_t;

typedef struct ch_attribute_t
{
    HB_RING_DECLARE( ch_attribute_t );

    hb_time_t created_timestamp;

    char name[CH_ATTRIBUTE_NAME_MAX + 1];
    
    ch_attribute_type_t value_type;

    union 
    {
        hb_bool_t value_boolean;
        uint64_t value_integer;
        char value_string[CH_ATTRIBUTE_VALUE_MAX + 1];
    };
} ch_attribute_t;

#endif