#include "ch_service.h"

#include "hb_memory/hb_memory.h"
#include "hb_utils/hb_time.h"
#include "hb_utils/hb_rand.h"
#include "hb_utils/hb_math.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////
static hb_result_t __ch_service_create_messages( ch_service_t * _service )
{
    hb_mutex_handle_t * mutex;
    if( hb_mutex_create( &mutex ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    _service->messages_mutex = mutex;
    _service->messages_max = _service->capacity;

    for( uint32_t index = 0; index != 7; ++index )
    {
        _service->messages_count[index] = 0;
        _service->messages[index] = HB_NULLPTR;
    }

    ch_message_t * message_empty = HB_NEWE( ch_message_t, 1 );
    HB_RING_INIT( base, message_empty );
    message_empty->created_timestamp = _service->created_timestamp;
    message_empty->capacity = 0;
    message_empty->text[0] = '\0';

    _service->message_empty = message_empty;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static hb_result_t __ch_service_create_records( ch_service_t * _service )
{
    hb_mutex_handle_t * mutex;
    if( hb_mutex_create( &mutex ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    _service->records_mutex = mutex;
    _service->records_enumerator = 0;
    _service->records_count = 0;
    _service->records_max = _service->capacity;

    _service->records = HB_NULLPTR;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static hb_result_t __ch_service_create_attributes( ch_service_t * _service )
{
    hb_mutex_handle_t * mutex;
    if( hb_mutex_create( &mutex ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    _service->attributes_mutex = mutex;
    _service->attributes_count = 0;
    _service->attributes_max = _service->capacity * CH_RECORD_ATTRIBUTES_MAX;

    _service->attributes = HB_NULLPTR;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static hb_result_t __ch_service_create_tags( ch_service_t * _service )
{
    hb_mutex_handle_t * mutex;
    if( hb_mutex_create( &mutex ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    _service->tags_mutex = mutex;
    _service->tags_count = 0;
    _service->tags_max = _service->capacity * CH_RECORD_TAGS_MAX;

    _service->tags = HB_NULLPTR;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t ch_service_create( ch_service_t ** _service, uint32_t _capacity, hb_time_t _timemax )
{
    ch_service_t * service = HB_NEW( ch_service_t );

    if( service == HB_NULLPTR )
    {
        return HB_FAILURE;
    }

    service->capacity = _capacity;
    service->timemax = _timemax;

    hb_time_t timestamp;
    hb_time( &timestamp );

    service->created_timestamp = timestamp;

    if( __ch_service_create_messages( service ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( __ch_service_create_records( service ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( __ch_service_create_attributes( service ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( __ch_service_create_tags( service ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    *_service = service;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static void __ch_service_init_record( ch_record_t * _record, hb_time_t _timestamp, uint64_t _id )
{
    hb_time( &_record->created_timestamp );
    _record->created_timestamp = _timestamp;
    _record->id = _id;
    _record->flags = CH_RECORD_ATTRIBUTE_NONE;
    _record->project[0] = '\0';
    _record->user_id[0] = '\0';
    _record->level = 0;
    _record->service[0] = '\0';
    _record->thread[0] = '\0';
    _record->message = HB_NULLPTR;
    _record->timestamp = 0;
    _record->live = 0;
    _record->build_environment[0] = '\0';
    _record->build_release = HB_FALSE;
    _record->build_bundle[0] = '\0';
    _record->build_version[0] = '\0';
    _record->build_number = 0;
    _record->device_model[0] = '\0';
    _record->os_family[0] = '\0';
    _record->os_version[0] = '\0';

    _record->attributes = HB_NULLPTR;
    _record->tags = HB_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
static hb_result_t __ch_service_unmutex_get_record( ch_service_t * _service, hb_time_t _timestamp, ch_record_t ** _record )
{
    if( _service->records == HB_NULLPTR )
    {
        ch_record_t * record = HB_NEW( ch_record_t );

        if( record == HB_NULLPTR )
        {
            return HB_FAILURE;
        }

        HB_RING_INIT( base, record );

        _service->records = record;

        ++_service->records_count;

        *_record = record;

        return HB_SUCCESSFUL;
    }

    if( _timestamp - HB_RING_GET_PREV( base, _service->records )->created_timestamp >= _service->timemax )
    {
        while( _timestamp - HB_RING_GET_PREV_PREV( base, _service->records )->created_timestamp >= _service->timemax )
        {
            if( _service->records_count == 1 )
            {
                break;
            }
            else
            {
                --_service->records_count;

                ch_record_t * old_record = HB_RING_GET_PREV( base, _service->records );

                HB_RING_REMOVE( base, old_record );

                HB_FREE( old_record );
            }
        }

        ch_record_t * record = HB_RING_GET_PREV( base, _service->records );
        _service->records = record;

        *_record = record;

        return HB_SUCCESSFUL;
    }

    if( _service->records_count >= _service->records_max )
    {
        ch_record_t * record = HB_RING_GET_PREV( base, _service->records );
        _service->records = record;

        *_record = record;

        return HB_SUCCESSFUL;
    }

    ch_record_t * record = HB_NEW( ch_record_t );

    if( record == HB_NULLPTR )
    {
        return HB_FAILURE;
    }

    HB_RING_PUSH_BACK( base, _service->records, record );
    _service->records = record;

    ++_service->records_count;

    *_record = record;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t ch_service_get_record( ch_service_t * _service, hb_time_t _timestamp, ch_record_t ** _record )
{
    hb_mutex_lock( _service->records_mutex );

    hb_result_t result = __ch_service_unmutex_get_record( _service, _timestamp, _record );

    if( result == HB_SUCCESSFUL )
    {
        uint64_t id = ++_service->records_enumerator;

        __ch_service_init_record( *_record, _timestamp, id );
    }

    hb_mutex_unlock( _service->records_mutex );

    return result;
}
//////////////////////////////////////////////////////////////////////////
static void __ch_service_init_message( ch_message_t * _message, hb_time_t _timestamp, const char * _text, hb_size_t _size )
{
    _message->created_timestamp = _timestamp;

    memcpy( _message->text, _text, _size );
    _message->text[_size] = '\0';
}
//////////////////////////////////////////////////////////////////////////
static hb_result_t __ch_service_unmutex_get_message( ch_service_t * _service, hb_time_t _timestamp, hb_size_t _size, ch_message_t ** _message )
{
    hb_size_t clamp_size = hb_clampz( CH_MESSAGE_TEXT_MIN, CH_MESSAGE_TEXT_MAX, _size );

    uint32_t nearest_pow2_size = hb_nearest_pow2( clamp_size );
    uint32_t l2_size = hb_log2( nearest_pow2_size );
    uint32_t capacity = hb_pow2( l2_size );

    uint32_t index = l2_size - hb_log2( CH_MESSAGE_TEXT_MIN );

    if( _service->messages[index] == HB_NULLPTR )
    {
        ch_message_t * message = HB_NEWE( ch_message_t, capacity );

        if( message == HB_NULLPTR )
        {
            return HB_FAILURE;
        }

        message->capacity = capacity;

        HB_RING_INIT( base, message );

        _service->messages[index] = message;

        ++_service->messages_count[index];

        *_message = message;

        return HB_SUCCESSFUL;
    }

    ch_message_t * messages = _service->messages[index];

    if( _timestamp - HB_RING_GET_PREV( base, messages )->created_timestamp >= _service->timemax )
    {
        while( _timestamp - HB_RING_GET_PREV_PREV( base, messages )->created_timestamp >= _service->timemax )
        {
            if( _service->messages_count[index] == 1 )
            {
                break;
            }
            else
            {
                --_service->messages_count[index];

                ch_message_t * old_message = HB_RING_GET_PREV( base, messages );

                HB_RING_REMOVE( base, old_message );

                HB_FREE( old_message );
            }
        }

        ch_message_t * message = HB_RING_GET_PREV( base, messages );
        _service->messages[index] = message;

        *_message = message;

        return HB_SUCCESSFUL;
    }

    if( _service->messages_count[index] >= _service->messages_max )
    {
        ch_message_t * message = HB_RING_GET_PREV( base, messages );
        _service->messages[index] = message;

        *_message = message;

        return HB_SUCCESSFUL;
    }

    ch_message_t * message = HB_NEWE( ch_message_t, capacity );

    if( message == HB_NULLPTR )
    {
        return HB_FAILURE;
    }

    message->capacity = capacity;

    HB_RING_PUSH_BACK( base, _service->messages[index], message );
    _service->messages[index] = message;

    ++_service->messages_count[index];

    *_message = message;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t ch_service_get_message( ch_service_t * _service, hb_time_t _timestamp, const char * _text, hb_size_t _size, ch_message_t ** _message )
{
    hb_mutex_lock( _service->messages_mutex );

    hb_result_t result = __ch_service_unmutex_get_message( _service, _timestamp, _size, _message );

    if( result == HB_SUCCESSFUL )
    {
        __ch_service_init_message( *_message, _timestamp, _text, _size );
    }

    hb_mutex_unlock( _service->messages_mutex );

    return result;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t ch_service_get_message_empty( ch_service_t * _service, hb_time_t _timestamp, ch_message_t ** _message )
{
    HB_UNUSED( _timestamp );

    *_message = _service->message_empty;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static void __ch_service_init_attribute( ch_attribute_t * _attribute, hb_time_t _timestamp )
{
    _attribute->created_timestamp = _timestamp;

    _attribute->name[0] = '\0';
    _attribute->value_boolean = HB_FALSE;
    _attribute->value_integer = 0UL;
    _attribute->value_string[0] = '\0';
}
//////////////////////////////////////////////////////////////////////////
static hb_result_t __ch_service_unmutex_get_attribute( ch_service_t * _service, hb_time_t _timestamp, ch_attribute_t ** _attribute )
{
    if( _service->attributes == HB_NULLPTR )
    {
        ch_attribute_t * attribute = HB_NEW( ch_attribute_t );

        if( attribute == HB_NULLPTR )
        {
            return HB_FAILURE;
        }

        HB_RING_INIT( base, attribute );

        _service->attributes = attribute;

        ++_service->attributes_count;

        *_attribute = attribute;

        return HB_SUCCESSFUL;
    }

    if( _timestamp - HB_RING_GET_PREV( base, _service->attributes )->created_timestamp >= _service->timemax )
    {
        while( _timestamp - HB_RING_GET_PREV_PREV( base, _service->attributes )->created_timestamp >= _service->timemax )
        {
            if( _service->attributes_count == 1 )
            {
                break;
            }
            else
            {
                --_service->attributes_count;

                ch_attribute_t * old_attribute = HB_RING_GET_PREV( base, _service->attributes );

                HB_RING_REMOVE( base, old_attribute );

                HB_FREE( old_attribute );
            }
        }

        ch_attribute_t * attribute = HB_RING_GET_PREV( base, _service->attributes );
        _service->attributes = attribute;

        *_attribute = attribute;

        return HB_SUCCESSFUL;
    }

    if( _service->attributes_count >= _service->attributes_max )
    {
        ch_attribute_t * attribute = HB_RING_GET_PREV( base, _service->attributes );
        _service->attributes = attribute;

        *_attribute = attribute;

        return HB_SUCCESSFUL;
    }

    ch_attribute_t * attribute = HB_NEW( ch_attribute_t );

    if( attribute == HB_NULLPTR )
    {
        return HB_FAILURE;
    }

    HB_RING_PUSH_BACK( base, _service->attributes, attribute );
    _service->attributes = attribute;

    ++_service->attributes_count;

    *_attribute = attribute;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t ch_service_get_attribute( ch_service_t * _service, hb_time_t _timestamp, ch_attribute_t ** _attribute )
{
    hb_mutex_lock( _service->attributes_mutex );

    hb_result_t result = __ch_service_unmutex_get_attribute( _service, _timestamp, _attribute );

    if( result == HB_SUCCESSFUL )
    {
        __ch_service_init_attribute( *_attribute, _timestamp );
    }

    hb_mutex_unlock( _service->attributes_mutex );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static void __ch_service_init_tag( ch_tag_t * _tag, hb_time_t _timestamp )
{
    _tag->created_timestamp = _timestamp;

    _tag->value[0] = '\0';
}
//////////////////////////////////////////////////////////////////////////
static hb_result_t __ch_service_unmutex_get_tag( ch_service_t * _service, hb_time_t _timestamp, ch_tag_t ** _tag )
{
    if( _service->tags == HB_NULLPTR )
    {
        ch_tag_t * tag = HB_NEW( ch_tag_t );

        if( tag == HB_NULLPTR )
        {
            return HB_FAILURE;
        }

        HB_RING_INIT( base, tag );

        _service->tags = tag;

        ++_service->tags_count;

        *_tag = tag;

        return HB_SUCCESSFUL;
    }

    if( _timestamp - HB_RING_GET_PREV( base, _service->tags )->created_timestamp >= _service->timemax )
    {
        while( _timestamp - HB_RING_GET_PREV_PREV( base, _service->tags )->created_timestamp >= _service->timemax )
        {
            if( _service->tags_count == 1 )
            {
                break;
            }
            else
            {
                --_service->tags_count;

                ch_tag_t * old_tag = HB_RING_GET_PREV( base, _service->tags );

                HB_RING_REMOVE( base, old_tag );

                HB_FREE( old_tag );
            }
        }

        ch_tag_t * tag = HB_RING_GET_PREV( base, _service->tags );
        _service->tags = tag;

        *_tag = tag;

        return HB_SUCCESSFUL;
    }

    if( _service->tags_count >= _service->tags_max )
    {
        ch_tag_t * tag = HB_RING_GET_PREV( base, _service->tags );
        _service->tags = tag;

        *_tag = tag;

        return HB_SUCCESSFUL;
    }

    ch_tag_t * tag = HB_NEW( ch_tag_t );

    if( tag == HB_NULLPTR )
    {
        return HB_FAILURE;
    }

    HB_RING_PUSH_BACK( base, _service->tags, tag );
    _service->tags = tag;

    ++_service->tags_count;

    *_tag = tag;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t ch_service_get_tag( ch_service_t * _service, hb_time_t _timestamp, ch_tag_t ** _tag )
{
    hb_mutex_lock( _service->tags_mutex );

    hb_result_t result = __ch_service_unmutex_get_tag( _service, _timestamp, _tag );

    if( result == HB_SUCCESSFUL )
    {
        __ch_service_init_tag( *_tag, _timestamp );
    }

    hb_mutex_unlock( _service->tags_mutex );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void ch_service_select_records( ch_service_t * _service, const char * _project, hb_time_t _timestamp, hb_time_t _timeoffset, hb_time_t _timelimit, hb_size_t _countlimit, ch_service_records_visitor_t _visitor, void * _ud )
{
    if( _countlimit == 0 )
    {
        return;
    }

    hb_mutex_lock( _service->records_mutex );

    if( _service->records == HB_NULLPTR )
    {
        hb_mutex_unlock( _service->records_mutex );

        return;
    }

    uint64_t index = 0;

    HB_RING_FOREACH( base, ch_record_t, _service->records, record )
    {
        if( _timestamp - record->created_timestamp >= _service->timemax )
        {
            break;
        }

        if( _timestamp - record->created_timestamp >= _timelimit )
        {
            break;
        }

        if( _timestamp - record->created_timestamp < _timeoffset )
        {
            continue;
        }

        if( strncmp( record->project, _project, CH_PROJECT_MAXLEN ) != 0 )
        {
            continue;
        }

        _visitor( index, record, _ud );

        if( --_countlimit == 0 )
        {
            break;
        }

        ++index;
    }

    hb_mutex_unlock( _service->records_mutex );
}
//////////////////////////////////////////////////////////////////////////