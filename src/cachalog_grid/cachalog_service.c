#include "cachalog_service.h"

#include "cachalog_memory/cachalog_memory.h"
#include "cachalog_utils/cachalog_time.h"
#include "cachalog_utils/cachalog_rand.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////
static ch_result_t __ch_service_create_messages( ch_service_t * _service )
{
    ch_mutex_handle_t * mutex;
    if( ch_mutex_create( &mutex ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    _service->messages_mutex = mutex;
    _service->messages_count = 0;
    _service->messages_max = _service->capacity;

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

    _service->records_mutex = mutex;
    _service->records_enumerator = 0;
    _service->records_seed = _service->created_timestamp;
    _service->records_count = 0;
    _service->records_max = _service->capacity;

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

    _service->attributes_mutex = mutex;
    _service->attributes_count = 0;
    _service->attributes_max = _service->capacity * CH_RECORD_ATTRIBUTES_MAX;

    _service->attributes = CH_NULLPTR;

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_service_create( ch_service_t ** _service, uint32_t _capacity, ch_time_t _timemax )
{
    ch_service_t * service = CH_NEW( ch_service_t );

    if( service == CH_NULLPTR )
    {
        return CH_FAILURE;
    }

    service->capacity = _capacity;
    service->timemax = _timemax;

    ch_time_t timestamp;
    ch_time( &timestamp );
    
    service->created_timestamp = timestamp;

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
static void __ch_service_init_record( ch_record_t * _record, ch_time_t _timestamp, uint64_t _id, uint64_t _rnd )
{
    ch_time( &_record->created_timestamp );
    _record->created_timestamp = _timestamp;
    _record->id = _id;
    _record->rnd = _rnd;
    _record->service[0] = '\0';
    _record->user_id[0] = '\0';
    _record->message = CH_NULLPTR;
    _record->category[0] = '\0';
    _record->level = 0;
    _record->timestamp = 0;
    _record->live = 0;
    _record->build_environment[0] = '\0';
    _record->build_release = CH_FALSE;
    _record->build_version[0] = '\0';
    _record->build_number = 0;
    _record->device_model[0] = '\0';
    _record->os_family[0] = '\0';
    _record->os_version[0] = '\0';

    for( ch_size_t index = 0; index != CH_RECORD_ATTRIBUTES_MAX; ++index )
    {
        _record->attributes[index] = CH_NULLPTR;
    }
}
//////////////////////////////////////////////////////////////////////////
static ch_result_t __ch_service_unmutex_get_record( ch_service_t * _service, ch_time_t _timestamp, ch_record_t ** _record )
{
    if( _service->records == CH_NULLPTR )
    {
        ch_record_t * record = CH_NEW( ch_record_t );

        if( record == CH_NULLPTR )
        {
            return CH_FAILURE;
        }

        CH_RING_INIT( record );

        _service->records = record;

        ++_service->records_count;

        *_record = record;

        return CH_SUCCESSFUL;
    }

    if( _timestamp - _service->records->prev->created_timestamp >= _service->timemax )
    {
        while( _timestamp - _service->records->prev->prev->created_timestamp >= _service->timemax )
        {
            --_service->records_count;

            if( _service->records_count == 0 )
            {
                _service->records_count = 1;

                break;
            }
            else if( _service->records_count == 1 )
            {
                ch_record_t * old_record = _service->records->prev;

                CH_RING_REMOVE( old_record );

                CH_FREE( old_record );

                break;
            }
            else
            {
                ch_record_t * old_record = _service->records->prev->prev;

                CH_RING_REMOVE( old_record );

                CH_FREE( old_record );
            }
        }

        ch_record_t * record = _service->records;
        _service->records = record->prev;

        *_record = record;

        return CH_SUCCESSFUL;
    }

    if( _service->records_count >= _service->records_max )
    {
        ch_record_t * record = _service->records;
        _service->records = record->prev;

        *_record = record;

        return CH_SUCCESSFUL;
    }

    ch_record_t * record = CH_NEW( ch_record_t );

    if( record == CH_NULLPTR )
    {
        return CH_FAILURE;
    }

    CH_RING_PUSH_FRONT( _service->records, record );

    ++_service->records_count;

    *_record = record;

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_service_get_record( ch_service_t * _service, ch_time_t _timestamp, ch_record_t ** _record )
{
    ch_mutex_lock( _service->records_mutex );

    ch_result_t result = __ch_service_unmutex_get_record( _service, _timestamp, _record );

    if( result == CH_SUCCESSFUL )
    {
        uint64_t id = ++_service->records_enumerator;
        uint64_t rnd = ch_rand64( &_service->records_seed );

        __ch_service_init_record( *_record, _timestamp, id, rnd );
    }

    ch_mutex_unlock( _service->records_mutex );

    return result;
}
//////////////////////////////////////////////////////////////////////////
static void __ch_service_init_message( ch_message_t * _message, ch_time_t _timestamp )
{
    _message->created_timestamp = _timestamp;
    _message->text[0] = '\0';
}
//////////////////////////////////////////////////////////////////////////
static ch_result_t __ch_service_unmutex_get_message( ch_service_t * _service, ch_time_t _timestamp, ch_message_t ** _message )
{
    if( _service->messages == CH_NULLPTR )
    {
        ch_message_t * message = CH_NEW( ch_message_t );

        if( message == CH_NULLPTR )
        {
            return CH_FAILURE;
        }

        CH_RING_INIT( message );

        _service->messages = message;
        
        ++_service->messages_count;

        *_message = message;

        return CH_SUCCESSFUL;
    }

    if( _timestamp - _service->messages->prev->created_timestamp >= _service->timemax )
    {
        while( _timestamp - _service->messages->prev->prev->created_timestamp >= _service->timemax )
        {
            --_service->messages_count;

            if( _service->messages_count == 0 )
            {
                _service->messages_count = 1;

                break;
            }
            else if( _service->messages_count == 1 )
            {
                ch_message_t * old_message = _service->messages->prev;

                CH_RING_REMOVE( old_message );

                CH_FREE( old_message );

                break;
            }
            else
            {
                ch_message_t * old_message = _service->messages->prev->prev;

                CH_RING_REMOVE( old_message );

                CH_FREE( old_message );
            }
        }

        ch_message_t * message = _service->messages;
        _service->messages = message->prev;

        *_message = message;

        return CH_SUCCESSFUL;
    }

    if( _service->messages_count >= _service->messages_max )
    {
        ch_message_t * message = _service->messages;
        _service->messages = message->prev;

        *_message = message;

        return CH_SUCCESSFUL;
    }

    ch_message_t * message = CH_NEW( ch_message_t );

    if( message == CH_NULLPTR )
    {
        return CH_FAILURE;
    }

    CH_RING_PUSH_FRONT( _service->messages, message );

    ++_service->messages_count;

    *_message = message;

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_service_get_message( ch_service_t * _service, ch_time_t _timestamp, ch_message_t ** _message )
{
    ch_mutex_lock( _service->messages_mutex );

    ch_result_t result = __ch_service_unmutex_get_message( _service, _timestamp, _message );

    if( result == CH_SUCCESSFUL )
    {
        __ch_service_init_message( *_message, _timestamp );
    }

    ch_mutex_unlock( _service->messages_mutex );

    return result;
}
//////////////////////////////////////////////////////////////////////////
static void __ch_service_init_attribute( ch_attribute_t * _attribute, ch_time_t _timestamp )
{
    _attribute->created_timestamp = _timestamp;

    _attribute->name[0] = '\0';
    _attribute->value[0] = '\0';
}
//////////////////////////////////////////////////////////////////////////
static ch_result_t __ch_service_unmutex_get_attribute( ch_service_t * _service, ch_time_t _timestamp, ch_attribute_t ** _attribute )
{
    if( _service->attributes == CH_NULLPTR )
    {
        ch_attribute_t * attribute = CH_NEW( ch_attribute_t );

        if( attribute == CH_NULLPTR )
        {
            return CH_FAILURE;
        }

        CH_RING_INIT( attribute );

        _service->attributes = attribute;

        ++_service->attributes_count;

        *_attribute = attribute;

        return CH_SUCCESSFUL;
    }

    if( _timestamp - _service->attributes->prev->created_timestamp >= _service->timemax )
    {
        while( _timestamp - _service->attributes->prev->prev->created_timestamp >= _service->timemax )
        {
            --_service->attributes_count;

            if( _service->attributes_count == 0 )
            {
                _service->attributes_count = 1;

                break;
            }
            else if( _service->attributes_count == 1 )
            {
                ch_attribute_t * old_attribute = _service->attributes->prev;

                CH_RING_REMOVE( old_attribute );

                CH_FREE( old_attribute );

                break;
            }
            else
            {
                ch_attribute_t * old_attribute = _service->attributes->prev->prev;

                CH_RING_REMOVE( old_attribute );

                CH_FREE( old_attribute );
            }
        }

        ch_attribute_t * attribute = _service->attributes;
        _service->attributes = attribute->prev;

        *_attribute = attribute;

        return CH_SUCCESSFUL;
    }

    if( _service->attributes_count >= _service->attributes_max )
    {
        ch_attribute_t * attribute = _service->attributes;
        _service->attributes = attribute->prev;

        *_attribute = attribute;

        return CH_SUCCESSFUL;
    }

    ch_attribute_t * attribute = CH_NEW( ch_attribute_t );

    if( attribute == CH_NULLPTR )
    {
        return CH_FAILURE;
    }

    CH_RING_PUSH_FRONT( _service->attributes, attribute );

    ++_service->attributes_count;

    *_attribute = attribute;

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_service_get_attribute( ch_service_t * _service, ch_time_t _timestamp, ch_attribute_t ** _attribute )
{
    ch_mutex_lock( _service->attributes_mutex );

    ch_result_t result = __ch_service_unmutex_get_attribute( _service, _timestamp, _attribute );

    if( result == CH_SUCCESSFUL )
    {
        __ch_service_init_attribute( *_attribute, _timestamp );
    }

    ch_mutex_unlock( _service->attributes_mutex );

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_service_select_records( ch_service_t * _service, ch_time_t _timestamp, ch_time_t _timeoffset, ch_time_t _timelimit, ch_size_t _countlimit, ch_service_records_visitor_t _visitor, void * _ud )
{
    ch_mutex_lock( _service->records_mutex );

    ch_record_t * root_record = _service->records;    

    if( root_record != CH_NULLPTR )
    {
        uint64_t index = 0;

        ch_record_t * record = _service->records;

        do
        {
            if( _timestamp - record->created_timestamp > _service->timemax )
            {
                break;
            }

            if( _timestamp - record->created_timestamp > _timelimit )
            {
                break;
            }

            if( _timestamp - record->created_timestamp < _timeoffset )
            {
                record = record->next;

                continue;
            }

            _visitor( index, record, _ud );

            if( --_countlimit == 0 )
            {
                break;
            }

            ++index;

            record = record->next;
        } while( record != root_record );
    }

    ch_mutex_unlock( _service->records_mutex );

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////