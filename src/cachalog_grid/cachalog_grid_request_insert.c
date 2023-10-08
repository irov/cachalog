#include "cachalog_config/cachalog_config.h"
#include "cachalog_json/cachalog_json.h"
#include "cachalog_utils/cachalog_time.h"

#include "cachalog_log/cachalog_log.h"

#include "cachalog_service.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////
typedef struct json_attributes_foreach_ud_t
{
    ch_service_t * service;
    ch_record_t * record;
    ch_time_t timestamp;
} json_attributes_foreach_ud_t;
//////////////////////////////////////////////////////////////////////////
static ch_result_t __ch_grid_json_attributes_visitor( ch_size_t _index, const ch_json_handle_t * _key, const ch_json_handle_t * _value, void * _ud )
{
    json_attributes_foreach_ud_t * ud = (json_attributes_foreach_ud_t *)_ud;

    ch_attribute_t * attribute;
    if( ch_service_get_attribute( ud->service, ud->timestamp, &attribute ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    attribute->record = ud->record;

    ch_json_copy_string( _key, attribute->name, sizeof( attribute->name ), CH_NULLPTR );
    ch_json_copy_string( _value, attribute->value, sizeof( attribute->value ), CH_NULLPTR );

    ud->record->attributes[_index] = attribute;

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_http_code_t ch_grid_request_insert( const ch_json_handle_t * _json, ch_service_t * _service, char * _response, ch_size_t * _size )
{
    CH_UNUSED( _response );
    CH_UNUSED( _size );

    ch_time_t timestamp;
    ch_time( &timestamp );

    ch_record_t * record;
    if( ch_service_get_record( _service, timestamp, &record ) == CH_FAILURE )
    {
        return CH_HTTP_INTERNAL;
    }    

    ch_bool_t required = CH_TRUE;

    ch_json_copy_field_string_required( _json, "service", record->service, sizeof( record->service ), CH_NULLPTR, &required );
    ch_json_copy_field_string_required( _json, "user.id", record->user_id, sizeof( record->user_id ), CH_NULLPTR, &required );
    ch_json_copy_field_string_required( _json, "category", record->category, sizeof( record->category ), CH_NULLPTR, &required );
    ch_json_get_field_uint32_required( _json, "level", &record->level, &required );
    ch_json_get_field_uint64_required( _json, "timestamp", &record->timestamp, &required );
    ch_json_get_field_uint64_required( _json, "live", &record->live, &required );
    ch_json_copy_field_string_required( _json, "build.environment", record->build_environment, sizeof( record->build_environment ), CH_NULLPTR, &required );
    ch_json_get_field_bool_required( _json, "build.release", &record->build_release, &required );
    ch_json_copy_field_string_required( _json, "build.version", record->build_version, sizeof( record->build_version ), CH_NULLPTR, &required );
    ch_json_get_field_uint64_required( _json, "build.number", &record->build_number, &required );
    ch_json_copy_field_string_required( _json, "device.model", record->device_model, sizeof( record->device_model ), CH_NULLPTR, &required );
    ch_json_copy_field_string_required( _json, "os.family", record->os_family, sizeof( record->os_family ), CH_NULLPTR, &required );
    ch_json_copy_field_string_required( _json, "os.version", record->os_version, sizeof( record->os_version ), CH_NULLPTR, &required );

    ch_message_t * message;
    if( ch_service_get_message( _service, timestamp, &message ) == CH_FAILURE )
    {
        return CH_HTTP_INTERNAL;
    }

    message->record = record;

    ch_json_copy_field_string_required( _json, "message", message->text, sizeof( message->text ), CH_NULLPTR, &required );

    record->message = message;

    json_attributes_foreach_ud_t ud;
    ud.service = _service;
    ud.record = record;
    ud.timestamp = timestamp;

    ch_result_t result_attributes = ch_json_foreach_field_object( _json, "attributes", &__ch_grid_json_attributes_visitor, &ud );

    if( result_attributes == CH_FAILURE )
    {
        return CH_HTTP_INTERNAL;
    }

    CH_LOG_MESSAGE_INFO( "log", "record id: %llu timestamp: %llu message: %s"
        , record->id
        , record->created_timestamp
        , message->text
    );

    return CH_HTTP_OK;
}
//////////////////////////////////////////////////////////////////////////