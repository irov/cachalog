#include "cachalog_config/cachalog_config.h"
#include "cachalog_json/cachalog_json.h"
#include "cachalog_utils/cachalog_time.h"

#include "cachalog_log/cachalog_log.h"

#include "cachalog_service.h"

#include <string.h>
#include <stdio.h>

//////////////////////////////////////////////////////////////////////////
typedef struct records_visitor_select_t
{
    char * response;
    ch_size_t * size;
} records_visitor_select_t;
//////////////////////////////////////////////////////////////////////////
static void __ch_service_records_visitor_t( uint64_t _index, const ch_record_t * _record, void * _ud )
{
    records_visitor_select_t * ud = (records_visitor_select_t *)_ud;

    int response_offset = sprintf( ud->response + *ud->size, "%s{\"service\":\"%s\",\"user.id\":\"%s\",\"message\":\"%s\",\"file\":\"%s\",\"line\":%u,\"category\":\"%s\",\"level\":%u,\"timestamp\":%llu,\"live\":%llu,\"build.environment\":\"%s\",\"build.release\":%s,\"build.version\":\"%s\",\"build.number\":%llu,\"device.model\":\"%s\",\"os.family\":\"%s\",\"os.version\":\"%s\",\"attributes\":{"
        , _index == 0 ? "" : ","
        , _record->service
        , _record->user_id
        , _record->message->text
        , _record->file->text
        , _record->line
        , _record->category
        , _record->level
        , _record->timestamp
        , _record->live
        , _record->build_environment
        , _record->build_release ? "true" : "false"
        , _record->build_version
        , _record->build_number
        , _record->device_model
        , _record->os_family
        , _record->os_version
    );

    for( ch_size_t attributes_index = 0; attributes_index != CH_RECORD_ATTRIBUTES_MAX; ++attributes_index )
    {
        const ch_attribute_t * attribute = _record->attributes[attributes_index];

        if( attribute == CH_NULLPTR )
        {
            break;
        }

        if( strcmp( attribute->name, "REMOVED" ) == 0 )
        {
            continue;
        }

        if( strcmp( attribute->value, "REMOVED" ) == 0 )
        {
            continue;
        }

        response_offset += sprintf( ud->response + *ud->size + response_offset, "%s\"%s\":\"%s\""
            , attributes_index == 0 ? "" : ","
            , attribute->name
            , attribute->value
        );
    }

    response_offset += sprintf( ud->response + *ud->size + response_offset, "}}" );

    *ud->size += response_offset;
}
//////////////////////////////////////////////////////////////////////////
ch_http_code_t ch_grid_request_select( const ch_json_handle_t * _json, ch_service_t * _service, char * _response, ch_size_t * _size )
{
    CH_UNUSED( _response );
    CH_UNUSED( _size );

    ch_time_t timestamp;
    ch_time( &timestamp );

    ch_time_t time_offset;
    ch_time_t time_limit;
    ch_size_t count_limit;
    ch_json_get_field_uint64( _json, "time.offset", &time_offset, 0 );
    ch_json_get_field_uint64( _json, "time.limit", &time_limit, CH_TIME_SECONDS_IN_DAY );
    ch_json_get_field_size_t( _json, "count.limit", &count_limit, 100 );

    int response_offset = sprintf( _response, "{\"records\":[" );

    *_size = response_offset;

    records_visitor_select_t ud;
    ud.response = _response;
    ud.size = _size;

    if( ch_service_select_records( _service, timestamp, time_offset, time_limit, count_limit, &__ch_service_records_visitor_t, &ud ) == CH_FAILURE )
    {
        return CH_HTTP_INTERNAL;
    }

    *_size += sprintf( _response + *_size, "]}" );

    return CH_HTTP_OK;
}
//////////////////////////////////////////////////////////////////////////