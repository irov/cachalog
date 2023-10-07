#include "cachalog_service.h"

#include "cachalog_memory/cachalog_memory.h"

//////////////////////////////////////////////////////////////////////////
static ch_result_t __ch_service_create_messages( ch_service_t * _service )
{
    ch_mutex_handle_t * mutex;
    if( ch_mutex_create( &mutex ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    _service->mutex_messages = mutex;
    _service->messages = CH_NULLPTR;

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static ch_result_t __ch_service_create_records( ch_service_t * _service )
{
    ch_mutex_handle_t * mutex;
    if( ch_mutex_create( &mutex ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    _service->mutex_records = mutex;
    _service->records = CH_NULLPTR;

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static ch_result_t __ch_service_create_attributes( ch_service_t * _service )
{
    ch_mutex_handle_t * mutex;
    if( ch_mutex_create( &mutex ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    _service->mutex_attributes = mutex;
    _service->records = CH_NULLPTR;

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_service_create( ch_service_t ** _service, uint32_t _capacity )
{
    ch_service_t * service = CH_NEW( ch_service_t );

    if( service == CH_NULLPTR )
    {
        return CH_FAILURE;
    }

    service->capacity = _capacity;

    ch_mutex_handle_t * mutex_messages;
    if( ch_mutex_create( &mutex_messages ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    service->mutex_messages = mutex_messages;

    if( __ch_service_create_messages( service ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    if( __ch_service_create_records( service ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    if( __ch_service_create_attributes( service ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    *_service = service;

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_service_get_record( ch_service_t * _service, ch_record_t ** _record )
{
    ch_mutex_lock( _service->mutex_records );

    ch_record_t * record = _service->records;
    _service->records = record->next;

    ch_mutex_unlock( _service->mutex_records );

    *_record = record;

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_service_get_message( ch_service_t * _service, ch_message_t ** _message )
{
    if( _service->capacity == 0 )
    {
        *_message = CH_NULLPTR;

        return CH_SUCCESSFUL;
    }

    ch_mutex_lock( _service->mutex_messages );

    if( _service->messages == CH_NULLPTR )
    {

    }

    ch_message_t * message = _service->messages;
    _service->messages = message->next;

    ch_mutex_unlock( _service->mutex_messages );

    *_message = message;

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_service_get_attribute( ch_service_t * _service, ch_attribute_t ** _attribute )
{
    ch_mutex_lock( _service->mutex_attributes );

    ch_attribute_t * attribute = _service->attributes;
    _service->attributes = attribute->next;

    ch_mutex_unlock( _service->mutex_attributes );

    *_attribute = attribute;

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
