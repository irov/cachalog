#ifndef CACHALOG_SERVICE_H_
#define CACHALOG_SERVICE_H_

#include "cachalog_config/cachalog_config.h"
#include "cachalog_mutex/cachalog_mutex.h"

#include "cachalog_message.h"
#include "cachalog_record.h"

typedef struct ch_service_t
{    
    uint32_t capacity;

    ch_mutex_handle_t * mutex_messages;
    ch_message_t * messages;

    ch_mutex_handle_t * mutex_records;
    ch_record_t * records;

    ch_mutex_handle_t * mutex_attributes;
    ch_attribute_t * attributes;
} ch_service_t;

ch_result_t ch_service_create( ch_service_t ** _service, uint32_t _capacity );
ch_result_t ch_service_get_record( ch_service_t * _service, ch_record_t ** _record );
ch_result_t ch_service_get_message( ch_service_t * _service, ch_message_t ** _message );
ch_result_t ch_service_get_attribute( ch_service_t * _service, ch_attribute_t ** _attribute );

#endif