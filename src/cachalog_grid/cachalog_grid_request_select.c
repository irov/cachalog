#include "cachalog_config/cachalog_config.h"
#include "cachalog_json/cachalog_json.h"
#include "cachalog_utils/cachalog_time.h"

#include "cachalog_log/cachalog_log.h"

#include "cachalog_service.h"

#include <string.h>
#include <stdio.h>

//////////////////////////////////////////////////////////////////////////
typedef struct records_filter_t
{
    char service[CH_RECORD_SERVICE_MAX];
    char user_id[CH_RECORD_USER_ID_MAX];
    char message[CH_MESSAGE_TEXT_MAX];
    char file[CH_MESSAGE_TEXT_MAX];
    ch_bool_t is_line;
    uint32_t line;
    char category[CH_RECORD_CATEGORY_MAX];
    ch_bool_t is_level;
    uint32_t level;
    ch_bool_t is_timestamp;
    ch_time_t timestamp;
    ch_bool_t is_live;
    ch_time_t live;
    char build_environment[CH_RECORD_BUILD_ENVIRONMENT_MAX];
    ch_bool_t is_build_release;
    ch_bool_t build_release;
    char build_version[CH_RECORD_BUILD_VERSION_MAX];
    ch_bool_t is_build_number;
    uint64_t build_number;
    char device_model[CH_RECORD_DEVICE_MODEL_MAX];
    char os_family[CH_RECORD_OS_FAMILY_MAX];
    char os_version[CH_RECORD_OS_VERSION_MAX];
    ch_size_t attributes_count;
    char attributes_name[CH_RECORD_ATTRIBUTES_MAX][CH_ATTRIBUTE_NAME_MAX];
    char attributes_value[CH_RECORD_ATTRIBUTES_MAX][CH_ATTRIBUTE_VALUE_MAX];
    ch_size_t tags_count;
    char tags_value[CH_RECORD_TAGS_MAX][CH_TAG_VALUE_MAX];
} records_filter_t;
//////////////////////////////////////////////////////////////////////////
typedef struct records_visitor_select_t
{
    const records_filter_t * filter;

    char * response;
    ch_size_t * size;
} records_visitor_select_t;
//////////////////////////////////////////////////////////////////////////
static void __ch_service_records_visitor_t( uint64_t _index, const ch_record_t * _record, void * _ud )
{
    records_visitor_select_t * ud = (records_visitor_select_t *)_ud;

    const records_filter_t * filter = ud->filter;

    if( filter != CH_NULLPTR )
    {
        if( filter->service[0] != '\0' && strstr( filter->service, _record->service ) == CH_NULLPTR )
        {
            return;
        }

        if( filter->user_id[0] != '\0' && strstr( filter->user_id, _record->user_id ) == CH_NULLPTR )
        {
            return;
        }

        if( filter->message[0] != '\0' && strstr( filter->message, _record->message->text ) == CH_NULLPTR )
        {
            return;
        }

        if( filter->file[0] != '\0' && strstr( filter->file, _record->file->text ) == CH_NULLPTR )
        {
            return;
        }

        if( filter->is_line == CH_TRUE && filter->line != _record->line )
        {
            return;
        }

        if( filter->category[0] != '\0' && strstr( filter->category, _record->category ) == CH_NULLPTR )
        {
            return;
        }

        if( filter->is_level == CH_TRUE && filter->level != _record->level )
        {
            return;
        }

        if( filter->is_timestamp == CH_TRUE && filter->timestamp != _record->timestamp )
        {
            return;
        }

        if( filter->is_live == CH_TRUE && filter->live != _record->live )
        {
            return;
        }

        if( filter->build_environment[0] != '\0' && strstr( filter->build_environment, _record->build_environment ) == CH_NULLPTR )
        {
            return;
        }

        if( filter->is_build_release == CH_TRUE && filter->build_release != _record->build_release )
        {
            return;
        }

        if( filter->build_version[0] != '\0' && strstr( filter->build_version, _record->build_version ) == CH_NULLPTR )
        {
            return;
        }

        if( filter->is_build_number == CH_TRUE && filter->build_number != _record->build_number )
        {
            return;
        }

        if( filter->device_model[0] != '\0' && strstr( filter->device_model, _record->device_model ) == CH_NULLPTR )
        {
            return;
        }

        if( filter->os_family[0] != '\0' && strstr( filter->os_family, _record->os_family ) == CH_NULLPTR )
        {
            return;
        }

        if( filter->os_version[0] != '\0' && strstr( filter->os_version, _record->os_version ) == CH_NULLPTR )
        {
            return;
        }

        for( ch_size_t attributes_index = 0; attributes_index != filter->attributes_count; ++attributes_index )
        {
            const char * attribute_name = filter->attributes_name[attributes_index];
            const char * attribute_value = filter->attributes_value[attributes_index];

            ch_bool_t found = CH_FALSE;

            for( ch_size_t record_attributes_index = 0; record_attributes_index != CH_RECORD_ATTRIBUTES_MAX; ++record_attributes_index )
            {
                const ch_attribute_t * attribute = _record->attributes[record_attributes_index];

                if( attribute == CH_NULLPTR )
                {
                    continue;
                }

                if( strstr( attribute->name, attribute_name ) == CH_NULLPTR )
                {
                    continue;
                }

                if( strstr( attribute->value, attribute_value ) == CH_NULLPTR )
                {
                    continue;
                }

                found = CH_TRUE;

                break;
            }

            if( found == CH_FALSE )
            {
                return;
            }
        }

        for( ch_size_t tags_index = 0; tags_index != filter->tags_count; ++tags_index )
        {
            const char * tag_value = filter->tags_value[tags_index];

            ch_bool_t found = CH_FALSE;

            for( ch_size_t record_tags_index = 0; record_tags_index != CH_RECORD_TAGS_MAX; ++record_tags_index )
            {
                const ch_tag_t * tag = _record->tags[record_tags_index];

                if( tag == CH_NULLPTR )
                {
                    continue;
                }

                if( strstr( tag->value, tag_value ) == CH_NULLPTR )
                {
                    continue;
                }

                found = CH_TRUE;

                break;
            }

            if( found == CH_FALSE )
            {
                return;
            }
        }
    }

    int response_offset = sprintf( ud->response + *ud->size, "%s{\"service\":\"%s\",\"user.id\":\"%s\",\"message\":\"%s\",\"file\":\"%s\",\"line\":%u,\"category\":\"%s\",\"level\":%u,\"timestamp\":%llu,\"live\":%llu,\"build.environment\":\"%s\",\"build.release\":%s,\"build.version\":\"%s\",\"build.number\":%llu,\"device.model\":\"%s\",\"os.family\":\"%s\",\"os.version\":\"%s\""
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

    response_offset += sprintf( ud->response + *ud->size + response_offset, ",\"attributes\":{" );

    for( ch_size_t attributes_index = 0; attributes_index != CH_RECORD_ATTRIBUTES_MAX; ++attributes_index )
    {
        const ch_attribute_t * attribute = _record->attributes[attributes_index];

        if( attribute == CH_NULLPTR )
        {
            break;
        }

        response_offset += sprintf( ud->response + *ud->size + response_offset, "%s\"%s\":\"%s\""
            , attributes_index == 0 ? "" : ","
            , attribute->name
            , attribute->value
        );
    }

    response_offset += sprintf( ud->response + *ud->size + response_offset, "}" );

    response_offset += sprintf( ud->response + *ud->size + response_offset, ",\"tags\":[" );

    for( ch_size_t tags_index = 0; tags_index != CH_RECORD_TAGS_MAX; ++tags_index )
    {
        const ch_tag_t * tag = _record->tags[tags_index];

        if( tag == CH_NULLPTR )
        {
            break;
        }

        response_offset += sprintf( ud->response + *ud->size + response_offset, "%s\"%s\""
            , tags_index == 0 ? "" : ","
            , tag->value
        );
    }

    response_offset += sprintf( ud->response + *ud->size + response_offset, "]" );

    response_offset += sprintf( ud->response + *ud->size + response_offset, "}" );

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

    records_filter_t filter;
    filter.service[0] = '\0';
    filter.user_id[0] = '\0';
    filter.message[0] = '\0';
    filter.file[0] = '\0';
    filter.is_line = CH_FALSE;
    filter.line = 0;
    filter.category[0] = '\0';
    filter.is_level = CH_FALSE;
    filter.level = 0;
    filter.is_timestamp = CH_FALSE;
    filter.timestamp = 0;
    filter.is_live = CH_FALSE;
    filter.live = 0;
    filter.build_environment[0] = '\0';
    filter.is_build_release = CH_FALSE;
    filter.build_release = CH_FALSE;
    filter.build_version[0] = '\0';
    filter.is_build_number = CH_FALSE;
    filter.build_number = 0;
    filter.device_model[0] = '\0';
    filter.os_family[0] = '\0';
    filter.os_version[0] = '\0';
    filter.attributes_count = 0;
    filter.attributes_name[0][0] = '\0';
    filter.attributes_value[0][0] = '\0';
    filter.tags_count = 0;
    filter.tags_value[0][0] = '\0';

    ch_json_handle_t * json_filter = CH_NULLPTR;
    if( ch_json_get_field( _json, "filter", &json_filter ) == CH_SUCCESSFUL )
    {
        filter.is_line = CH_TRUE;
        filter.is_level = CH_TRUE;
        filter.is_timestamp = CH_TRUE;
        filter.is_live = CH_TRUE;
        filter.is_build_release = CH_TRUE;
        filter.is_build_number = CH_TRUE;

        ch_json_copy_field_string( json_filter, "service", filter.service, CH_RECORD_SERVICE_MAX, CH_NULLPTR, "" );
        ch_json_copy_field_string( json_filter, "user.id", filter.user_id, CH_RECORD_USER_ID_MAX, CH_NULLPTR, "" );
        ch_json_copy_field_string( json_filter, "message", filter.message, CH_MESSAGE_TEXT_MAX, CH_NULLPTR, "" );
        ch_json_copy_field_string( json_filter, "file", filter.file, CH_MESSAGE_TEXT_MAX, CH_NULLPTR, "" );
        ch_json_get_field_uint32_required( json_filter, "line", &filter.line, &filter.is_line );
        ch_json_copy_field_string( json_filter, "category", filter.category, CH_RECORD_CATEGORY_MAX, CH_NULLPTR, "" );
        ch_json_get_field_uint32_required( json_filter, "level", &filter.level, &filter.is_level );
        ch_json_get_field_uint64_required( json_filter, "timestamp", &filter.timestamp, &filter.is_timestamp );
        ch_json_get_field_uint64_required( json_filter, "live", &filter.live, &filter.is_live );
        ch_json_copy_field_string( json_filter, "build.environment", filter.build_environment, CH_RECORD_BUILD_ENVIRONMENT_MAX, CH_NULLPTR, "" );
        ch_json_get_field_boolean_required( json_filter, "build.release", &filter.build_release, &filter.is_build_release );
        ch_json_copy_field_string( json_filter, "build.version", filter.build_version, CH_RECORD_BUILD_VERSION_MAX, CH_NULLPTR, "" );
        ch_json_get_field_uint64_required( json_filter, "build.number", &filter.build_number, &filter.is_build_number );
        ch_json_copy_field_string( json_filter, "device.model", filter.device_model, CH_RECORD_DEVICE_MODEL_MAX, CH_NULLPTR, "" );
        ch_json_copy_field_string( json_filter, "os.family", filter.os_family, CH_RECORD_OS_FAMILY_MAX, CH_NULLPTR, "" );
        ch_json_copy_field_string( json_filter, "os.version", filter.os_version, CH_RECORD_OS_VERSION_MAX, CH_NULLPTR, "" );

        ch_json_handle_t * json_attributes;
        if( ch_json_get_field( json_filter, "attributes", &json_attributes ) == CH_SUCCESSFUL )
        {
            ch_size_t attributes_count;
            if( ch_json_get_array_count( json_attributes, &attributes_count ) == CH_FAILURE )
            {
                return CH_HTTP_BADREQUEST;
            }

            for( ch_size_t index = 0; index != attributes_count; ++index )
            {
                ch_json_handle_t * json_attribute;
                if( ch_json_get_array_element( json_attributes, index, &json_attribute ) == CH_SUCCESSFUL )
                {
                    ch_json_copy_field_string( json_attribute, "name", filter.attributes_name[index], CH_ATTRIBUTE_NAME_MAX, CH_NULLPTR, "" );
                    ch_json_copy_field_string( json_attribute, "value", filter.attributes_value[index], CH_ATTRIBUTE_VALUE_MAX, CH_NULLPTR, "" );
                }
            }

            filter.attributes_count = attributes_count;
        }

        ch_json_handle_t * json_tags;
        if( ch_json_get_field( json_filter, "tags", &json_tags ) == CH_SUCCESSFUL )
        {
            ch_size_t tags_count;
            if( ch_json_get_array_count( json_tags, &tags_count ) == CH_FAILURE )
            {
                return CH_HTTP_BADREQUEST;
            }

            for( ch_size_t index = 0; index != tags_count; ++index )
            {
                ch_json_handle_t * json_tag;
                if( ch_json_get_array_element( json_tags, index, &json_tag ) == CH_SUCCESSFUL )
                {
                    if( ch_json_copy_string( json_tag, filter.tags_value[index], CH_TAG_VALUE_MAX, CH_NULLPTR ) == CH_FAILURE )
                    {
                        return CH_HTTP_BADREQUEST;
                    }
                }
            }

            filter.tags_count = tags_count;
        }
    }

    int response_offset = sprintf( _response, "{\"records\":[" );

    *_size = response_offset;

    records_visitor_select_t ud;
    if( json_filter != CH_NULLPTR )
    {
        ud.filter = &filter;
    }
    else
    {
        ud.filter = CH_NULLPTR;
    }

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