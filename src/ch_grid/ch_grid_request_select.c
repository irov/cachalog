#include "ch_config.h"

#include "ch_service.h"

#include "hb_json/hb_json.h"
#include "hb_utils/hb_time.h"

#include "hb_log/hb_log.h"

#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

//////////////////////////////////////////////////////////////////////////
typedef struct ch_records_filter_t
{
    uint64_t flags;

    char user_id[CH_RECORD_USER_ID_MAX + 1];
    uint32_t level;

    char service[CH_RECORD_SERVICE_MAX + 1];

    char message[CH_MESSAGE_TEXT_MAX];

    char file[CH_MESSAGE_TEXT_MAX];
    uint32_t line;

    hb_time_t timestamp;
    hb_time_t live;

    char build_environment[CH_RECORD_BUILD_ENVIRONMENT_MAX + 1];
    hb_bool_t build_release;
    char build_version[CH_RECORD_BUILD_VERSION_MAX + 1];
    uint64_t build_number;

    char device_model[CH_RECORD_DEVICE_MODEL_MAX + 1];

    char os_family[CH_RECORD_OS_FAMILY_MAX + 1];
    char os_version[CH_RECORD_OS_VERSION_MAX + 1];

    hb_size_t attributes_count;
    char attributes_name[CH_RECORD_ATTRIBUTES_MAX][CH_ATTRIBUTE_NAME_MAX];
    char attributes_value[CH_RECORD_ATTRIBUTES_MAX][CH_ATTRIBUTE_VALUE_MAX];

    hb_size_t tags_count;
    char tags_value[CH_RECORD_TAGS_MAX][CH_TAG_VALUE_MAX];
} ch_records_filter_t;
//////////////////////////////////////////////////////////////////////////
typedef struct ch_records_visitor_select_t
{
    const ch_records_filter_t * filter;

    char * response;
    hb_size_t capacity;
    hb_size_t * size;
} ch_records_visitor_select_t;
//////////////////////////////////////////////////////////////////////////
static void __response_write( ch_records_visitor_select_t * _ud, const char * _format, ... )
{
    va_list args;
    va_start( args, _format );

    int write_count = vsnprintf( _ud->response + *_ud->size, _ud->capacity - *_ud->size, _format, args );

    va_end( args );

    *_ud->size += (hb_size_t)write_count;
}
//////////////////////////////////////////////////////////////////////////
static void __ch_service_records_visitor_t( uint64_t _index, const ch_record_t * _record, void * _ud )
{
    ch_records_visitor_select_t * ud = (ch_records_visitor_select_t *)_ud;

    const ch_records_filter_t * filter = ud->filter;

    if( filter != HB_NULLPTR )
    {
        if( (_record->flags & filter->flags) != filter->flags )
        {
            return;
        }

        if( CH_HAS_RECORD_FLAG( filter->flags, CH_RECORD_ATTRIBUTE_USER_ID ) && strstr( filter->user_id, _record->user_id ) == HB_NULLPTR )
        {
            return;
        }

        if( CH_HAS_RECORD_FLAG( filter->flags, CH_RECORD_ATTRIBUTE_LEVEL ) && filter->level != _record->level )
        {
            return;
        }

        if( CH_HAS_RECORD_FLAG( filter->flags, CH_RECORD_ATTRIBUTE_SERVICE ) && strstr( filter->service, _record->service ) == HB_NULLPTR )
        {
            return;
        }

        if( CH_HAS_RECORD_FLAG( filter->flags, CH_RECORD_ATTRIBUTE_MESSAGE ) && strstr( filter->message, _record->message->text ) == HB_NULLPTR )
        {
            return;
        }

        if( CH_HAS_RECORD_FLAG( filter->flags, CH_RECORD_ATTRIBUTE_FILE ) && strstr( filter->file, _record->file->text ) == HB_NULLPTR )
        {
            return;
        }

        if( CH_HAS_RECORD_FLAG( filter->flags, CH_RECORD_ATTRIBUTE_LINE ) && filter->line != _record->line )
        {
            return;
        }

        if( CH_HAS_RECORD_FLAG( filter->flags, CH_RECORD_ATTRIBUTE_TIMESTAMP ) && filter->timestamp != _record->timestamp )
        {
            return;
        }

        if( CH_HAS_RECORD_FLAG( filter->flags, CH_RECORD_ATTRIBUTE_LIVE ) && filter->live != _record->live )
        {
            return;
        }

        if( CH_HAS_RECORD_FLAG( filter->flags, CH_RECORD_ATTRIBUTE_BUILD_ENVIRONMENT ) && strstr( filter->build_environment, _record->build_environment ) == HB_NULLPTR )
        {
            return;
        }

        if( CH_HAS_RECORD_FLAG( filter->flags, CH_RECORD_ATTRIBUTE_BUILD_RELEASE ) && filter->build_release != _record->build_release )
        {
            return;
        }

        if( CH_HAS_RECORD_FLAG( filter->flags, CH_RECORD_ATTRIBUTE_BUILD_VERSION ) && strstr( filter->build_version, _record->build_version ) == HB_NULLPTR )
        {
            return;
        }

        if( CH_HAS_RECORD_FLAG( filter->flags, CH_RECORD_ATTRIBUTE_BUILD_NUMBER ) && filter->build_number != _record->build_number )
        {
            return;
        }

        if( CH_HAS_RECORD_FLAG( filter->flags, CH_RECORD_ATTRIBUTE_DEVICE_MODEL ) && strstr( filter->device_model, _record->device_model ) == HB_NULLPTR )
        {
            return;
        }

        if( CH_HAS_RECORD_FLAG( filter->flags, CH_RECORD_ATTRIBUTE_OS_FAMILY ) && strstr( filter->os_family, _record->os_family ) == HB_NULLPTR )
        {
            return;
        }

        if( CH_HAS_RECORD_FLAG( filter->flags, CH_RECORD_ATTRIBUTE_OS_VERSION ) && strstr( filter->os_version, _record->os_version ) == HB_NULLPTR )
        {
            return;
        }

        for( hb_size_t attributes_index = 0; attributes_index != filter->attributes_count; ++attributes_index )
        {
            const char * attribute_name = filter->attributes_name[attributes_index];
            const char * attribute_value = filter->attributes_value[attributes_index];

            hb_bool_t found = HB_FALSE;

            for( hb_size_t record_attributes_index = 0; record_attributes_index != CH_RECORD_ATTRIBUTES_MAX; ++record_attributes_index )
            {
                const ch_attribute_t * attribute = _record->attributes[record_attributes_index];

                if( attribute == HB_NULLPTR )
                {
                    continue;
                }

                if( strstr( attribute->name, attribute_name ) == HB_NULLPTR )
                {
                    continue;
                }

                if( strstr( attribute->value, attribute_value ) == HB_NULLPTR )
                {
                    continue;
                }

                found = HB_TRUE;

                break;
            }

            if( found == HB_FALSE )
            {
                return;
            }
        }

        for( hb_size_t tags_index = 0; tags_index != filter->tags_count; ++tags_index )
        {
            const char * tag_value = filter->tags_value[tags_index];

            hb_bool_t found = HB_FALSE;

            for( hb_size_t record_tags_index = 0; record_tags_index != CH_RECORD_TAGS_MAX; ++record_tags_index )
            {
                const ch_tag_t * tag = _record->tags[record_tags_index];

                if( tag == HB_NULLPTR )
                {
                    continue;
                }

                if( strstr( tag->value, tag_value ) == HB_NULLPTR )
                {
                    continue;
                }

                found = HB_TRUE;

                break;
            }

            if( found == HB_FALSE )
            {
                return;
            }
        }
    }

    if( _index != 0 )
    {
        __response_write( ud, "," );
    }

    __response_write( ud, "{" );

    if( CH_HAS_RECORD_FLAG( _record->flags, CH_RECORD_ATTRIBUTE_USER_ID ) )
    {
        __response_write( ud, "\"user.id\":\"%s\""
            , _record->user_id
        );
    }

    if( CH_HAS_RECORD_FLAG( _record->flags, CH_RECORD_ATTRIBUTE_LEVEL ) )
    {
        __response_write( ud, ",\"level\":%" PRIu32 ""
            , _record->level
        );
    }

    if( CH_HAS_RECORD_FLAG( _record->flags, CH_RECORD_ATTRIBUTE_SERVICE ) )
    {
        __response_write( ud, ",\"service\":\"%s\""
            , _record->service
        );
    }

    if( CH_HAS_RECORD_FLAG( _record->flags, CH_RECORD_ATTRIBUTE_MESSAGE ) )
    {
        __response_write( ud, ",\"message\":\"%s\""
            , _record->message->text
        );
    }

    if( CH_HAS_RECORD_FLAG( _record->flags, CH_RECORD_ATTRIBUTE_FILE ) )
    {
        __response_write( ud, ",\"file\":\"%s\""
            , _record->file->text
        );
    }

    if( CH_HAS_RECORD_FLAG( _record->flags, CH_RECORD_ATTRIBUTE_LINE ) )
    {
        __response_write( ud, ",\"line\":%" PRIu32 ""
            , _record->line
        );
    }

    if( CH_HAS_RECORD_FLAG( _record->flags, CH_RECORD_ATTRIBUTE_TIMESTAMP ) )
    {
        __response_write( ud, ",\"timestamp\":%" PRIu64 ""
            , _record->timestamp
        );
    }

    if( CH_HAS_RECORD_FLAG( _record->flags, CH_RECORD_ATTRIBUTE_LIVE ) )
    {
        __response_write( ud, ",\"live\":%" PRIu64 ""
            , _record->live
        );
    }

    if( CH_HAS_RECORD_FLAG( _record->flags, CH_RECORD_ATTRIBUTE_BUILD_ENVIRONMENT ) )
    {
        __response_write( ud, ",\"build.environment\":\"%s\""
            , _record->build_environment
        );
    }

    if( CH_HAS_RECORD_FLAG( _record->flags, CH_RECORD_ATTRIBUTE_BUILD_RELEASE ) )
    {
        __response_write( ud, ",\"build.release\":%s"
            , _record->build_release ? "true" : "false"
        );
    }

    if( CH_HAS_RECORD_FLAG( _record->flags, CH_RECORD_ATTRIBUTE_BUILD_VERSION ) )
    {
        __response_write( ud, ",\"build.version\":\"%s\""
            , _record->build_version
        );
    }

    if( CH_HAS_RECORD_FLAG( _record->flags, CH_RECORD_ATTRIBUTE_BUILD_NUMBER ) )
    {
        __response_write( ud, ",\"build.number\":%" PRIu64 ""
            , _record->build_number
        );
    }

    if( CH_HAS_RECORD_FLAG( _record->flags, CH_RECORD_ATTRIBUTE_DEVICE_MODEL ) )
    {
        __response_write( ud, ",\"device.model\":\"%s\""
            , _record->device_model
        );
    }

    if( CH_HAS_RECORD_FLAG( _record->flags, CH_RECORD_ATTRIBUTE_OS_FAMILY ) )
    {
        __response_write( ud, ",\"os.family\":\"%s\""
            , _record->os_family
        );
    }

    if( CH_HAS_RECORD_FLAG( _record->flags, CH_RECORD_ATTRIBUTE_OS_VERSION ) )
    {
        __response_write( ud, ",\"os.version\":\"%s\""
            , _record->os_version
        );
    }

    if( CH_HAS_RECORD_FLAG( _record->flags, CH_RECORD_ATTRIBUTE_ATTRIBUTES ) )
    {
        __response_write( ud, ",\"attributes\":{" );

        for( hb_size_t attributes_index = 0; attributes_index != CH_RECORD_ATTRIBUTES_MAX; ++attributes_index )
        {
            const ch_attribute_t * attribute = _record->attributes[attributes_index];

            if( attribute == HB_NULLPTR )
            {
                break;
            }

            if( attributes_index != 0 )
            {
                __response_write( ud, "," );
            }

            __response_write( ud, "\"%s\":\"%s\""
                , attribute->name
                , attribute->value
            );
        }

        __response_write( ud, "}" );
    }

    if( CH_HAS_RECORD_FLAG( _record->flags, CH_RECORD_ATTRIBUTE_TAGS ) )
    {
        __response_write( ud, ",\"tags\":[" );

        for( hb_size_t tags_index = 0; tags_index != CH_RECORD_TAGS_MAX; ++tags_index )
        {
            const ch_tag_t * tag = _record->tags[tags_index];

            if( tag == HB_NULLPTR )
            {
                break;
            }

            if( tags_index != 0 )
            {
                __response_write( ud, "," );
            }

            __response_write( ud, "\"%s\""
                , tag->value
            );
        }

        __response_write( ud, "]" );
    }

    __response_write( ud, "}" );
}
//////////////////////////////////////////////////////////////////////////
ch_http_code_t ch_grid_request_select( ch_service_t * _service, const char * _project, const hb_json_handle_t * _json, char * _response, hb_size_t _capacity, hb_size_t * _size )
{
    hb_time_t timestamp;
    hb_time( &timestamp );

    hb_time_t time_offset = 0;
    hb_time_t time_limit = HB_TIME_SECONDS_IN_DAY;
    hb_size_t count_limit = 100;
    hb_json_get_field_uint64( _json, "time.offset", &time_offset );
    hb_json_get_field_uint64( _json, "time.limit", &time_limit );
    hb_json_get_field_size_t( _json, "count.limit", &count_limit );

    ch_records_filter_t filter;
    filter.flags = 0;
    filter.service[0] = '\0';
    filter.user_id[0] = '\0';
    filter.message[0] = '\0';
    filter.file[0] = '\0';
    filter.line = 0;
    filter.level = 0;
    filter.timestamp = 0;
    filter.live = 0;
    filter.build_environment[0] = '\0';
    filter.build_release = HB_FALSE;
    filter.build_version[0] = '\0';
    filter.build_number = 0;
    filter.device_model[0] = '\0';
    filter.os_family[0] = '\0';
    filter.os_version[0] = '\0';
    filter.attributes_count = 0;
    filter.attributes_name[0][0] = '\0';
    filter.attributes_value[0][0] = '\0';
    filter.tags_count = 0;
    filter.tags_value[0][0] = '\0';

    const hb_json_handle_t * json_filter = HB_NULLPTR;
    if( hb_json_object_get_field( _json, "filter", &json_filter ) == HB_SUCCESSFUL )
    {
        hb_json_copy_field_string( json_filter, "user.id", filter.user_id, sizeof( filter.user_id ) );
        hb_json_copy_field_string( json_filter, "service", filter.service, sizeof( filter.service ) );
        hb_json_copy_field_string( json_filter, "message", filter.message, sizeof( filter.message ) );
        hb_json_copy_field_string( json_filter, "file", filter.file, sizeof( filter.file ) );
        hb_json_get_field_uint32( json_filter, "line", &filter.line );
        hb_json_get_field_uint32( json_filter, "level", &filter.level );
        hb_json_get_field_uint64( json_filter, "timestamp", &filter.timestamp );
        hb_json_get_field_uint64( json_filter, "live", &filter.live );
        hb_json_copy_field_string( json_filter, "build.environment", filter.build_environment, sizeof( filter.build_environment ) );
        hb_json_get_field_boolean( json_filter, "build.release", &filter.build_release );
        hb_json_copy_field_string( json_filter, "build.version", filter.build_version, sizeof( filter.build_version ) );
        hb_json_get_field_uint64( json_filter, "build.number", &filter.build_number );
        hb_json_copy_field_string( json_filter, "device.model", filter.device_model, sizeof( filter.device_model ) );
        hb_json_copy_field_string( json_filter, "os.family", filter.os_family, sizeof( filter.os_family ) );
        hb_json_copy_field_string( json_filter, "os.version", filter.os_version, sizeof( filter.os_version ) );

        const hb_json_handle_t * json_attributes;
        if( hb_json_object_get_field( json_filter, "attributes", &json_attributes ) == HB_SUCCESSFUL )
        {
            if( hb_json_is_array( json_attributes ) == HB_FALSE )
            {
                return CH_HTTP_BADREQUEST;
            }

            hb_size_t attributes_count = hb_json_get_array_size( json_attributes );

            for( hb_size_t index = 0; index != attributes_count; ++index )
            {
                const hb_json_handle_t * json_attribute;
                if( hb_json_array_get_element( json_attributes, index, &json_attribute ) == HB_FAILURE )
                {
                    return CH_HTTP_BADREQUEST;
                }

                if( hb_json_copy_field_string( json_attribute, "name", filter.attributes_name[index], sizeof( filter.attributes_name[index] ) ) == HB_FAILURE )
                {
                    return CH_HTTP_BADREQUEST;
                }

                if( hb_json_copy_field_string( json_attribute, "value", filter.attributes_value[index], sizeof( filter.attributes_value[index] ) ) == HB_FAILURE )
                {
                    return CH_HTTP_BADREQUEST;
                }
            }

            filter.attributes_count = attributes_count;
        }

        const hb_json_handle_t * json_tags;
        if( hb_json_object_get_field( json_filter, "tags", &json_tags ) == HB_SUCCESSFUL )
        {
            if( hb_json_is_array( json_tags ) == HB_FALSE )
            {
                return CH_HTTP_BADREQUEST;
            }

            hb_size_t tags_count = hb_json_get_array_size( json_tags );

            for( hb_size_t index = 0; index != tags_count; ++index )
            {
                const hb_json_handle_t * json_tag;
                if( hb_json_array_get_element( json_tags, index, &json_tag ) == HB_FAILURE )
                {
                    return CH_HTTP_BADREQUEST;
                }

                if( hb_json_copy_string( json_tag, filter.tags_value[index], sizeof( filter.tags_value[index] ), HB_NULLPTR ) == HB_FAILURE )
                {
                    return CH_HTTP_BADREQUEST;
                }
            }

            filter.tags_count = tags_count;
        }
    }

    ch_records_visitor_select_t ud;
    if( json_filter != HB_NULLPTR )
    {
        ud.filter = &filter;
    }
    else
    {
        ud.filter = HB_NULLPTR;
    }

    ud.response = _response;
    ud.capacity = _capacity;
    ud.size = _size;

    __response_write( &ud, "{\"project\":\"%s\",\"records\":[", _project );

    if( ch_service_select_records( _service, _project, timestamp, time_offset, time_limit, count_limit, &__ch_service_records_visitor_t, &ud ) == HB_FAILURE )
    {
        return CH_HTTP_INTERNAL;
    }

    __response_write( &ud, "]}" );

    return CH_HTTP_OK;
}
//////////////////////////////////////////////////////////////////////////