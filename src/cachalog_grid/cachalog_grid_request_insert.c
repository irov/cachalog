#include "cachalog_config/cachalog_config.h"
#include "cachalog_json/cachalog_json.h"
#include "cachalog_utils/cachalog_time.h"

#include "cachalog_log/cachalog_log.h"

#include "cachalog_service.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////
typedef struct json_foreach_ud_t
{
    ch_service_t * service;
    ch_record_t * record;
    ch_time_t timestamp;
} json_foreach_ud_t;
//////////////////////////////////////////////////////////////////////////
static ch_result_t __ch_grid_json_attributes_visitor( ch_size_t _index, const ch_json_handle_t * _key, const ch_json_handle_t * _value, void * _ud )
{
    if( _index >= CH_RECORD_ATTRIBUTES_MAX )
    {
        return CH_FAILURE;
    }

    json_foreach_ud_t * ud = (json_foreach_ud_t *)_ud;

    ch_attribute_t * attribute;
    if( ch_service_get_attribute( ud->service, ud->timestamp, &attribute ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    ch_json_copy_string( _key, attribute->name, sizeof( attribute->name ), CH_NULLPTR );
    ch_json_copy_string( _value, attribute->value, sizeof( attribute->value ), CH_NULLPTR );

    ud->record->attributes[_index] = attribute;

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static ch_result_t __ch_grid_json_tags_visitor( ch_size_t _index, const ch_json_handle_t * _value, void * _ud )
{
    if( _index >= CH_RECORD_TAGS_MAX )
    {
        return CH_FAILURE;
    }

    json_foreach_ud_t * ud = (json_foreach_ud_t *)_ud;

    ch_tag_t * tag;
    if( ch_service_get_tag( ud->service, ud->timestamp, &tag ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    ch_json_copy_string( _value, tag->value, sizeof( tag->value ), CH_NULLPTR );

    ud->record->tags[_index] = tag;

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

    record->flags |= 1 << CH_RECORD_ATTRIBUTE_SERVICE;

    ch_bool_t required = CH_TRUE;

    ch_json_copy_field_string_required( _json, "service", record->service, sizeof( record->service ), CH_NULLPTR, &required );
    ch_json_copy_field_string( _json, "user.id", record->user_id, sizeof( record->user_id ), CH_NULLPTR, "" );
    ch_json_copy_field_string( _json, "category", record->category, sizeof( record->category ), CH_NULLPTR, "" );

    ch_size_t file_length;
    if( ch_json_get_field_string_length( _json, "file", &file_length ) == CH_SUCCESSFUL )
    {
        ch_message_t * file;
        if( ch_service_get_message( _service, timestamp, file_length, &file ) == CH_FAILURE )
        {
            return CH_HTTP_INTERNAL;
        }

        ch_json_copy_field_string( _json, "file", file->text, file->capacity, CH_NULLPTR, "" );

        record->file = file;
    }
    else
    {
        ch_message_t * file;
        if( ch_service_get_message_empty( _service, timestamp, &file ) == CH_FAILURE )
        {
            return CH_HTTP_INTERNAL;
        }

        record->file = file;
    }

    ch_json_get_field_uint32( _json, "line", &record->line, 0 );
    ch_json_get_field_uint32_required( _json, "level", &record->level, &required );
    ch_json_get_field_uint64( _json, "timestamp", &record->timestamp, 0 );
    ch_json_get_field_uint64( _json, "live", &record->live, 0 );
    ch_json_copy_field_string( _json, "build.environment", record->build_environment, sizeof( record->build_environment ), CH_NULLPTR, "" );
    ch_json_get_field_boolean( _json, "build.release", &record->build_release, CH_TRUE );
    ch_json_copy_field_string( _json, "build.version", record->build_version, sizeof( record->build_version ), CH_NULLPTR, "" );
    ch_json_get_field_uint64( _json, "build.number", &record->build_number, 0 );
    ch_json_copy_field_string( _json, "device.model", record->device_model, sizeof( record->device_model ), CH_NULLPTR, "" );
    ch_json_copy_field_string( _json, "os.family", record->os_family, sizeof( record->os_family ), CH_NULLPTR, "" );
    ch_json_copy_field_string( _json, "os.version", record->os_version, sizeof( record->os_version ), CH_NULLPTR, "" );

    ch_size_t message_length;
    if( ch_json_get_field_string_length( _json, "message", &message_length ) == CH_SUCCESSFUL )
    {
        ch_message_t * message;
        if( ch_service_get_message( _service, timestamp, message_length, &message ) == CH_FAILURE )
        {
            return CH_HTTP_INTERNAL;
        }

        ch_json_copy_field_string_required( _json, "message", message->text, message->capacity, CH_NULLPTR, &required );

        record->message = message;
    }
    else
    {
        return CH_HTTP_BADREQUEST;
    }

    if( required == CH_FALSE )
    {
        return CH_HTTP_BADREQUEST;
    }

    ch_json_handle_t * json_attributes;
    if( ch_json_get_field( _json, "attributes", &json_attributes ) == CH_SUCCESSFUL )
    {
        json_foreach_ud_t ud;
        ud.service = _service;
        ud.record = record;
        ud.timestamp = timestamp;

        if( ch_json_foreach_object( json_attributes, &__ch_grid_json_attributes_visitor, &ud ) == CH_FAILURE )
        {
            return CH_HTTP_INTERNAL;
        }
    }

    ch_json_handle_t * json_tags;
    if( ch_json_get_field( _json, "tags", &json_tags ) == CH_SUCCESSFUL )
    {
        json_foreach_ud_t ud;
        ud.service = _service;
        ud.record = record;
        ud.timestamp = timestamp;

        if( ch_json_foreach_array( json_tags, &__ch_grid_json_tags_visitor, &ud ) == CH_FAILURE )
        {
            return CH_HTTP_INTERNAL;
        }
    }

    return CH_HTTP_OK;
}
//////////////////////////////////////////////////////////////////////////