#ifndef CH_ATTRIBUTE_H_
#define CH_ATTRIBUTE_H_

#include "ch_config.h"

#include "hb_utils/hb_time.h"
#include "hb_utils/hb_ring.h"

#define CH_ATTRIBUTE_NAME_MAX 32
#define CH_ATTRIBUTE_VALUE_MAX 128

typedef struct ch_attribute_t
{
    HB_RING_DECLARE( ch_attribute_t );

    hb_time_t created_timestamp;

    char name[CH_ATTRIBUTE_NAME_MAX];
    char value[CH_ATTRIBUTE_VALUE_MAX];

} ch_attribute_t;

#endif