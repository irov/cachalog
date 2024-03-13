#include "ch_grid_request.h"

#include "ch_service.h"

#include "hb_json/hb_json.h"
#include "hb_utils/hb_time.h"

#include "hb_log/hb_log.h"

#include <string.h>
#include <stdio.h>
#include <inttypes.h>

//////////////////////////////////////////////////////////////////////////
typedef struct json_foreach_ud_t
{
    ch_service_t * service;
    ch_record_t * record;

    hb_time_t timestamp;

    char * reason;
} json_foreach_ud_t;
//////////////////////////////////////////////////////////////////////////
static hb_result_t __ch_grid_json_attributes_visitor( hb_size_t _index, const hb_json_handle_t * _key, const hb_json_handle_t * _value, void * _ud )
{
    json_foreach_ud_t * ud = (json_foreach_ud_t *)_ud;

    if( _index >= CH_RECORD_ATTRIBUTES_MAX )
    {
        snprintf( ud->reason, CH_GRID_REASON_MAX_SIZE, "invalid get attributes count %zu >= %zu", _index, CH_RECORD_ATTRIBUTES_MAX );

        return HB_FAILURE;
    }

    ch_attribute_t * attribute;
    if( ch_service_get_attribute( ud->service, ud->timestamp, &attribute ) == HB_FAILURE )
    {
        snprintf( ud->reason, CH_GRID_REASON_MAX_SIZE, "invalid get attribute" );

        return HB_FAILURE;
    }

    if( hb_json_copy_string( _key, attribute->name, sizeof( attribute->name ), HB_NULLPTR ) == HB_FAILURE )
    {
        snprintf( ud->reason, CH_GRID_REASON_MAX_SIZE, "invalid get attribute name copy" );

        return HB_FAILURE;
    }

    hb_json_type_e value_type = hb_json_get_type( _value );

    switch( value_type )
    {
    case e_hb_json_false:
        {
            attribute->value_type = CH_ATTRIBUTE_TYPE_BOOLEAN;
            attribute->value_boolean = HB_FALSE;
        } break;
    case e_hb_json_true:
        {
            attribute->value_type = CH_ATTRIBUTE_TYPE_BOOLEAN;
            attribute->value_boolean = HB_TRUE;
        } break;
    case e_hb_json_integer:
        {
            attribute->value_type = CH_ATTRIBUTE_TYPE_INTEGER;

            if( hb_json_to_uint64( _value, &attribute->value_integer ) == HB_FAILURE )
            {
                snprintf( ud->reason, CH_GRID_REASON_MAX_SIZE, "invalid get attribute '%s' value integer"
                    , attribute->name
                );

                return HB_FAILURE;
            }
        } break;
    case e_hb_json_string:
        {
            attribute->value_type = CH_ATTRIBUTE_TYPE_STRING;

            if( hb_json_copy_string( _value, attribute->value_string, sizeof( attribute->value_string ), HB_NULLPTR ) == HB_FAILURE )
            {
                snprintf( ud->reason, CH_GRID_REASON_MAX_SIZE, "invalid get attribute '%s' value string"
                    , attribute->name
                );

                return HB_FAILURE;
            }
        } break;
    default:
        {
            snprintf( ud->reason, CH_GRID_REASON_MAX_SIZE, "invalid get attribute '%s' value type %d"
                , attribute->name
                , value_type
            );

            return HB_FAILURE;
        } break;
    }

    if( ud->record->attributes == HB_NULLPTR )
    {
        HB_RING_INIT( record, attribute );

        ud->record->attributes = attribute;
    }
    else
    {
        HB_RING_PUSH_BACK( record, ud->record->attributes, attribute );
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static hb_result_t __ch_grid_json_tags_visitor( hb_size_t _index, const hb_json_handle_t * _value, void * _ud )
{
    json_foreach_ud_t * ud = (json_foreach_ud_t *)_ud;

    if( _index >= CH_RECORD_TAGS_MAX )
    {
        snprintf( ud->reason, CH_GRID_REASON_MAX_SIZE, "invalid get tags count %zu >= %zu", _index, CH_RECORD_TAGS_MAX );

        return HB_FAILURE;
    }

    ch_tag_t * tag;
    if( ch_service_get_tag( ud->service, ud->timestamp, &tag ) == HB_FAILURE )
    {
        snprintf( ud->reason, CH_GRID_REASON_MAX_SIZE, "invalid get tag" );

        return HB_FAILURE;
    }

    if( hb_json_copy_string( _value, tag->value, sizeof( tag->value ), HB_NULLPTR ) == HB_FAILURE )
    {
        snprintf( ud->reason, CH_GRID_REASON_MAX_SIZE, "invalid get tag value copy" );

        return HB_FAILURE;
    }

    if( ud->record->tags == HB_NULLPTR )
    {
        HB_RING_INIT( record, tag );

        ud->record->tags = tag;
    }
    else
    {
        HB_RING_PUSH_BACK( record, ud->record->tags, tag );
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static hb_bool_t __record_attribute_boolean( uint32_t _processId, uint64_t _requestId, ch_record_t * _record, ch_record_attributes_flag_e _flag, const hb_json_handle_t * _json, const char * _name, hb_bool_t * _value )
{
    const hb_json_handle_t * json_field;
    if( hb_json_get_field( _json, _name, &json_field ) == HB_FAILURE )
    {
        return HB_FALSE;
    }

    if( hb_json_to_boolean( json_field, _value ) == HB_FAILURE )
    {
        return HB_FALSE;
    }

    HB_LOG_MESSAGE_DEBUG( "grid", "[%u:%" PRIu64 "] set record attribute '%s' value: %" PRIu32 ""
        , _processId
        , _requestId
        , _name
        , *_value
    );

    _record->flags |= 1LL << _flag;

    return HB_TRUE;
}
//////////////////////////////////////////////////////////////////////////
static hb_bool_t __record_attribute_uint32( uint32_t _processId, uint64_t _requestId, ch_record_t * _record, ch_record_attributes_flag_e _flag, const hb_json_handle_t * _json, const char * _name, uint32_t * _value )
{
    const hb_json_handle_t * json_field;
    if( hb_json_get_field( _json, _name, &json_field ) == HB_FAILURE )
    {
        return HB_FALSE;
    }

    if( hb_json_to_uint32( json_field, _value ) == HB_FAILURE )
    {
        return HB_FALSE;
    }

    HB_LOG_MESSAGE_DEBUG( "grid", "[%u:%" PRIu64 "] set record attribute '%s' value: %" PRIu32 ""
        , _processId
        , _requestId
        , _name
        , *_value
    );

    _record->flags |= 1LL << _flag;

    return HB_TRUE;
}
//////////////////////////////////////////////////////////////////////////
static hb_bool_t __record_attribute_uint64( uint32_t _processId, uint64_t _requestId, ch_record_t * _record, ch_record_attributes_flag_e _flag, const hb_json_handle_t * _json, const char * _name, uint64_t * _value )
{
    const hb_json_handle_t * json_field;
    if( hb_json_get_field( _json, _name, &json_field ) == HB_FAILURE )
    {
        return HB_FALSE;
    }

    if( hb_json_to_uint64( json_field, _value ) == HB_FAILURE )
    {
        return HB_FALSE;
    }

    HB_LOG_MESSAGE_DEBUG( "grid", "[%u:%" PRIu64 "] set record attribute '%s' value: %" PRIu64 ""
        , _processId
        , _requestId
        , _name
        , *_value
    );

    _record->flags |= 1LL << _flag;

    return HB_TRUE;
}
//////////////////////////////////////////////////////////////////////////
static hb_bool_t __record_attribute_string( uint32_t _processId, uint64_t _requestId, ch_record_t * _record, ch_record_attributes_flag_e _flag, const hb_json_handle_t * _json, const char * _name, char * _value, hb_size_t _capacity )
{
    const hb_json_handle_t * json_field;
    if( hb_json_get_field( _json, _name, &json_field ) == HB_FAILURE )
    {
        return HB_FALSE;
    }

    if( hb_json_copy_string( json_field, _value, _capacity, HB_NULLPTR ) == HB_FAILURE )
    {
        return HB_FALSE;
    }

    HB_LOG_MESSAGE_DEBUG( "grid", "[%u:%" PRIu64 "] set record attribute '%s' value: %s"
        , _processId
        , _requestId
        , _name
        , _value
    );

    _record->flags |= 1LL << _flag;

    return HB_TRUE;
}
//////////////////////////////////////////////////////////////////////////
static ch_http_code_t __record_insert( const hb_json_handle_t * _json, uint32_t _processId, uint64_t _requestId, ch_service_t * _service, const char * _project, char * const _reason )
{
    hb_time_t timestamp;
    hb_time( &timestamp );

    HB_LOG_MESSAGE_DEBUG( "grid", "[%u:%" PRIu64 "] get record timestamp: %llu"
        , _processId
        , _requestId
        , timestamp
    );

    ch_record_t * record;
    if( ch_service_get_record( _service, timestamp, &record ) == HB_FAILURE )
    {
        snprintf( _reason, CH_GRID_REASON_MAX_SIZE, "invalid get record" );

        return CH_HTTP_INTERNAL;
    }

    HB_LOG_MESSAGE_DEBUG( "grid", "[%u:%" PRIu64 "] set project: %s"
        , _processId
        , _requestId
        , _project
    );

    strncpy( record->project, _project, sizeof( record->project ) );

    HB_LOG_MESSAGE_DEBUG( "grid", "[%u:%" PRIu64 "] fill record attributes"
        , _processId
        , _requestId
    );

    if( __record_attribute_string( _processId, _requestId, record, CH_RECORD_ATTRIBUTE_USER_ID, _json, "user.id", record->user_id, sizeof( record->user_id ) ) == HB_FALSE )
    {
        snprintf( _reason, CH_GRID_REASON_MAX_SIZE, "invalid get required user.id" );

        return CH_HTTP_BADREQUEST;
    }

    if( record->user_id[0] == '\0' )
    {
        snprintf( _reason, CH_GRID_REASON_MAX_SIZE, "invalid get required user.id empty" );

        return CH_HTTP_BADREQUEST;
    }

    if( __record_attribute_uint32( _processId, _requestId, record, CH_RECORD_ATTRIBUTE_LEVEL, _json, "level", &record->level ) == HB_FALSE )
    {
        snprintf( _reason, CH_GRID_REASON_MAX_SIZE, "invalid get required level" );

        return CH_HTTP_BADREQUEST;
    }

    __record_attribute_string( _processId, _requestId, record, CH_RECORD_ATTRIBUTE_SERVICE, _json, "service", record->service, sizeof( record->service ) );
    __record_attribute_string( _processId, _requestId, record, CH_RECORD_ATTRIBUTE_THREAD, _json, "thread", record->thread, sizeof( record->thread ) );

    hb_json_string_t message_string;
    if( hb_json_get_field_string( _json, "message", &message_string ) == HB_SUCCESSFUL )
    {
        if( message_string.size >= CH_MESSAGE_TEXT_MAX )
        {
            snprintf( _reason, CH_GRID_REASON_MAX_SIZE, "invalid get message length %zu >= %zu", message_string.size, CH_MESSAGE_TEXT_MAX );

            return CH_HTTP_BADREQUEST;
        }

        ch_message_t * message;
        if( ch_service_get_message( _service, timestamp, message_string.value, message_string.size, &message ) == HB_FAILURE )
        {
            snprintf( _reason, CH_GRID_REASON_MAX_SIZE, "invalid get message" );

            return CH_HTTP_INTERNAL;
        }

        record->flags |= 1LL << CH_RECORD_ATTRIBUTE_MESSAGE;

        record->message = message;
    }
    else
    {
        snprintf( _reason, CH_GRID_REASON_MAX_SIZE, "invalid get required message" );

        return CH_HTTP_BADREQUEST;
    }

    __record_attribute_uint64( _processId, _requestId, record, CH_RECORD_ATTRIBUTE_TIMESTAMP, _json, "timestamp", &record->timestamp );
    __record_attribute_uint64( _processId, _requestId, record, CH_RECORD_ATTRIBUTE_LIVE, _json, "live", &record->live );

    __record_attribute_string( _processId, _requestId, record, CH_RECORD_ATTRIBUTE_BUILD_ENVIRONMENT, _json, "build.environment", record->build_environment, sizeof( record->build_environment ) );
    __record_attribute_boolean( _processId, _requestId, record, CH_RECORD_ATTRIBUTE_BUILD_RELEASE, _json, "build.release", &record->build_release );
    __record_attribute_string( _processId, _requestId, record, CH_RECORD_ATTRIBUTE_BUILD_VERSION, _json, "build.version", record->build_version, sizeof( record->build_version ) );
    __record_attribute_uint64( _processId, _requestId, record, CH_RECORD_ATTRIBUTE_BUILD_NUMBER, _json, "build.number", &record->build_number );

    __record_attribute_string( _processId, _requestId, record, CH_RECORD_ATTRIBUTE_DEVICE_MODEL, _json, "device.model", record->device_model, sizeof( record->device_model ) );

    __record_attribute_string( _processId, _requestId, record, CH_RECORD_ATTRIBUTE_OS_FAMILY, _json, "os.family", record->os_family, sizeof( record->os_family ) );
    __record_attribute_string( _processId, _requestId, record, CH_RECORD_ATTRIBUTE_OS_VERSION, _json, "os.version", record->os_version, sizeof( record->os_version ) );

    const hb_json_handle_t * json_attributes;
    if( hb_json_get_field( _json, "attributes", &json_attributes ) == HB_SUCCESSFUL )
    {
        if( hb_json_is_object( json_attributes ) == HB_FALSE )
        {
            snprintf( _reason, CH_GRID_REASON_MAX_SIZE, "invalid attributes is not object" );

            return CH_HTTP_BADREQUEST;
        }

        json_foreach_ud_t ud;
        ud.service = _service;
        ud.record = record;
        ud.timestamp = timestamp;
        ud.reason = _reason;

        if( hb_json_visit_object( json_attributes, &__ch_grid_json_attributes_visitor, &ud ) == HB_FAILURE )
        {
            return CH_HTTP_BADREQUEST;
        }

        if( hb_json_get_object_size( json_attributes ) != 0 )
        {
            record->flags |= 1LL << CH_RECORD_ATTRIBUTE_ATTRIBUTES;
        }
    }

    const hb_json_handle_t * json_tags;
    if( hb_json_get_field( _json, "tags", &json_tags ) == HB_SUCCESSFUL )
    {
        if( hb_json_is_array( json_tags ) == HB_FALSE )
        {
            snprintf( _reason, CH_GRID_REASON_MAX_SIZE, "invalid tags is not array" );

            return CH_HTTP_BADREQUEST;
        }

        json_foreach_ud_t ud;
        ud.service = _service;
        ud.record = record;
        ud.timestamp = timestamp;
        ud.reason = _reason;

        if( hb_json_visit_array( json_tags, &__ch_grid_json_tags_visitor, &ud ) == HB_FAILURE )
        {
            return CH_HTTP_BADREQUEST;
        }

        if( hb_json_get_array_size( json_tags ) != 0 )
        {
            record->flags |= 1LL << CH_RECORD_ATTRIBUTE_TAGS;
        }
    }

    return CH_HTTP_OK;
}
//////////////////////////////////////////////////////////////////////////
typedef struct __ch_records_visitor_t
{
    uint32_t process_id;
    uint64_t request_id;

    ch_service_t * service;
    const char * project;

    ch_http_code_t http_code;
    char * http_reason;
} __ch_records_visitor_t;
//////////////////////////////////////////////////////////////////////////
static hb_result_t __records_visitor( hb_size_t _index, const hb_json_handle_t * _value, void * _ud )
{
    HB_UNUSED( _index );

    __ch_records_visitor_t * ud = (__ch_records_visitor_t *)_ud;

    if( hb_log_check_verbose_level( HB_LOG_DEBUG ) == HB_TRUE )
    {
        char value_dumps[HB_DATA_MAX_SIZE] = {'\0'};
        if( hb_json_dump( _value, value_dumps, sizeof( value_dumps ), HB_NULLPTR ) == HB_SUCCESSFUL )
        {
            HB_LOG_MESSAGE_DEBUG( "grid", "[%u:%" PRIu64 "] visit record index: %zu data: %s"
                , ud->process_id
                , ud->request_id
                , _index
                , value_dumps
            );
        }
    }

    ch_http_code_t http_code = __record_insert( _value, ud->process_id, ud->request_id, ud->service, ud->project, ud->http_reason );

    if( http_code != CH_HTTP_OK )
    {
        ud->http_code = http_code;

        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_http_code_t ch_grid_request_insert( const ch_grid_request_args_t * _args )
{
    const hb_json_handle_t * json_records;
    if( hb_json_get_field( _args->json, "records", &json_records ) == HB_FAILURE )
    {
        snprintf( _args->reason, CH_GRID_REASON_MAX_SIZE, "invalid get records" );

        return CH_HTTP_BADREQUEST;
    }

    if( hb_json_is_array( json_records ) == HB_FALSE )
    {
        snprintf( _args->reason, CH_GRID_REASON_MAX_SIZE, "invalid records is not array" );

        return CH_HTTP_BADREQUEST;
    }

    HB_LOG_MESSAGE_DEBUG( "grid", "[%u:%" PRIu64 "] insert visit records count: %zu"
        , _args->process_id
        , _args->request_id
        , hb_json_get_array_size( json_records )
    );

    __ch_records_visitor_t ud;
    ud.process_id = _args->process_id;
    ud.request_id = _args->request_id;
    ud.service = _args->service;
    ud.project = _args->project;
    ud.http_code = CH_HTTP_OK;
    ud.http_reason = _args->reason;

    if( hb_json_visit_array( json_records, &__records_visitor, &ud ) == HB_FAILURE )
    {
        return CH_HTTP_BADREQUEST;
    }

    *_args->response_size += (hb_size_t)snprintf( _args->response, CH_GRID_RESPONSE_DATA_MAX_SIZE, "{}" );

    return CH_HTTP_OK;
}
//////////////////////////////////////////////////////////////////////////