#ifndef CH_SERVICE_H_
#define CH_SERVICE_H_

#include "ch_config.h"

#include "ch_message.h"
#include "ch_record.h"

#include "hb_mutex/hb_mutex.h"

typedef struct ch_service_t
{    
    hb_time_t created_timestamp;

    uint32_t capacity;
    hb_time_t timemax;

    hb_mutex_handle_t * messages_mutex;
    hb_size_t messages_count[7];
    hb_size_t messages_max;
    ch_message_t * messages[7];
    ch_message_t * message_empty;

    hb_mutex_handle_t * records_mutex;
    uint64_t records_enumerator;
    uint64_t records_seed;
    hb_size_t records_count;
    hb_size_t records_max;
    ch_record_t * records;

    hb_mutex_handle_t * attributes_mutex;
    hb_size_t attributes_count;
    hb_size_t attributes_max;
    ch_attribute_t * attributes;

    hb_mutex_handle_t * tags_mutex;
    hb_size_t tags_count;
    hb_size_t tags_max;
    ch_tag_t * tags;
} ch_service_t;

hb_result_t ch_service_create( ch_service_t ** _service, uint32_t _capacity, hb_time_t _timemax );
hb_result_t ch_service_get_record( ch_service_t * _service, hb_time_t _timestamp, ch_record_t ** _record );
hb_result_t ch_service_get_message( ch_service_t * _service, hb_time_t _timestamp, const char * _text, hb_size_t _size, ch_message_t ** _message );
hb_result_t ch_service_get_message_empty( ch_service_t * _service, hb_time_t _timestamp, ch_message_t ** _message );
hb_result_t ch_service_get_attribute( ch_service_t * _service, hb_time_t _timestamp, ch_attribute_t ** _attribute );
hb_result_t ch_service_get_tag( ch_service_t * _service, hb_time_t _timestamp, ch_tag_t ** _tag );

typedef void(*ch_service_records_visitor_t)(uint64_t _index, const ch_record_t * _record, void * _ud);

hb_result_t ch_service_select_records( ch_service_t * _service, hb_time_t _timestamp, hb_time_t _timeoffset, hb_time_t _timelimit, hb_size_t _countlimit, ch_service_records_visitor_t _visitor, void * _ud );

#endif