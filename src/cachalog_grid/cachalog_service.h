#ifndef CACHALOG_SERVICE_H_
#define CACHALOG_SERVICE_H_

#include "cachalog_config/cachalog_config.h"
#include "cachalog_mutex/cachalog_mutex.h"

#include "cachalog_message.h"
#include "cachalog_record.h"

typedef struct ch_service_t
{    
    ch_time_t created_timestamp;

    uint32_t capacity;
    ch_time_t timemax;

    ch_mutex_handle_t * messages_mutex;
    ch_size_t messages_count[7];
    ch_size_t messages_max;
    ch_message_t * messages[7];
    ch_message_t * message_empty;

    ch_mutex_handle_t * records_mutex;
    uint64_t records_enumerator;
    uint64_t records_seed;
    ch_size_t records_count;
    ch_size_t records_max;
    ch_record_t * records;

    ch_mutex_handle_t * attributes_mutex;
    ch_size_t attributes_count;
    ch_size_t attributes_max;
    ch_attribute_t * attributes;

    ch_mutex_handle_t * tags_mutex;
    ch_size_t tags_count;
    ch_size_t tags_max;
    ch_tag_t * tags;
} ch_service_t;

ch_result_t ch_service_create( ch_service_t ** _service, uint32_t _capacity, ch_time_t _timemax );
ch_result_t ch_service_get_record( ch_service_t * _service, ch_time_t _timestamp, ch_record_t ** _record );
ch_result_t ch_service_get_message( ch_service_t * _service, ch_time_t _timestamp, ch_size_t _size, ch_message_t ** _message );
ch_result_t ch_service_get_message_empty( ch_service_t * _service, ch_time_t _timestamp, ch_message_t ** _message );
ch_result_t ch_service_get_attribute( ch_service_t * _service, ch_time_t _timestamp, ch_attribute_t ** _attribute );
ch_result_t ch_service_get_tag( ch_service_t * _service, ch_time_t _timestamp, ch_tag_t ** _tag );

typedef void(*ch_service_records_visitor_t)(uint64_t _index, const ch_record_t * _record, void * _ud);

ch_result_t ch_service_select_records( ch_service_t * _service, ch_time_t _timestamp, ch_time_t _timeoffset, ch_time_t _timelimit, ch_size_t _countlimit, ch_service_records_visitor_t _visitor, void * _ud );

#endif