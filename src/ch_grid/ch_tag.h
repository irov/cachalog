#ifndef CH_TAG_H_
#define CH_TAG_H_

#include "ch_config.h"

#include "hb_utils/hb_time.h"
#include "hb_utils/hb_ring.h"

#define CH_TAG_VALUE_MAX 31

typedef struct ch_tag_t
{
    HB_RING_DECLARE( base, ch_tag_t );
    HB_RING_DECLARE( record, ch_tag_t );

    hb_time_t created_timestamp;

    char value[CH_TAG_VALUE_MAX + 1];

} ch_tag_t;

#endif