#include "ch_grid_request.h"

#include "ch_service.h"

#include "hb_json/hb_json.h"
#include "hb_utils/hb_time.h"

#include "hb_log/hb_log.h"

#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

//////////////////////////////////////////////////////////////////////////
typedef enum ch_records_filter_attribute_value_type_e
{
    e_records_attributes_boolean,
    e_records_attributes_integer,
    e_records_attributes_string
} ch_records_filter_attribute_value_type_e;
//////////////////////////////////////////////////////////////////////////
typedef struct ch_records_filter_attribute_t
{
    ch_records_filter_attribute_value_type_e type;

    hb_json_string_t name;

    hb_bool_t value_boolean;
    uint64_t value_integer;
    hb_json_string_t value_string;
} ch_records_filter_attribute_t;
//////////////////////////////////////////////////////////////////////////
typedef struct ch_records_filter_t
{
    uint64_t flags;

    hb_json_string_t user_id;
    uint32_t level;

    hb_json_string_t service;
    hb_json_string_t thread;

    hb_json_string_t message;

    hb_json_string_t file;
    uint32_t line;

    hb_time_t timestamp;
    hb_time_t live;

    hb_json_string_t build_environment;
    hb_bool_t build_release;
    hb_json_string_t build_version;
    uint64_t build_number;

    hb_json_string_t device_model;

    hb_json_string_t os_family;
    hb_json_string_t os_version;

    hb_size_t attributes_count;

    ch_records_filter_attribute_t attributes[CH_RECORD_ATTRIBUTES_MAX];
} ch_records_filter_t;
//////////////////////////////////////////////////////////////////////////
typedef struct ch_records_visitor_select_t
{
    ch_records_filter_t filter;

    hb_json_string_t search;
    hb_bool_t search_is_integer;
    uint64_t search_integer;

    hb_size_t tags_count;
    hb_json_string_t tags[CH_RECORD_TAGS_MAX];

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
static const char * __hb_json_string_strstr( hb_json_string_t _string, const char * _search )
{
    const char * string_value = _string.value;
    hb_size_t string_size = _string.size;

    const char * b = _search;

    if( *b == '\0' )
    {
        return string_value;
    }

    for( const char
        * it_string_char = string_value,
        *it_string_char_end = string_value + string_size;
        it_string_char != it_string_char_end;
        ++it_string_char )
    {
        if( *it_string_char != *b )
        {
            continue;
        }

        const char * a = it_string_char;

        for( ;; )
        {
            if( *b == '\0' )
            {
                return it_string_char;
            }

            if( *a++ != *b++ )
            {
                break;
            }
        }

        b = _search;
    }

    return HB_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
static hb_bool_t __ch_service_records_filter_search( hb_json_string_t _search, const ch_record_t * _record )
{
    if( __hb_json_string_strstr( _search, _record->project ) != HB_NULLPTR )
    {
        return HB_TRUE;
    }

    if( __hb_json_string_strstr( _search, _record->user_id ) != HB_NULLPTR )
    {
        return HB_TRUE;
    }

    if( __hb_json_string_strstr( _search, _record->service ) != HB_NULLPTR )
    {
        return HB_TRUE;
    }

    if( __hb_json_string_strstr( _search, _record->thread ) != HB_NULLPTR )
    {
        return HB_TRUE;
    }

    if( __hb_json_string_strstr( _search, _record->message->text ) != HB_NULLPTR )
    {
        return HB_TRUE;
    }

    if( __hb_json_string_strstr( _search, _record->build_environment ) != HB_NULLPTR )
    {
        return HB_TRUE;
    }

    if( __hb_json_string_strstr( _search, _record->build_version ) != HB_NULLPTR )
    {
        return HB_TRUE;
    }

    if( __hb_json_string_strstr( _search, _record->device_model ) != HB_NULLPTR )
    {
        return HB_TRUE;
    }

    if( __hb_json_string_strstr( _search, _record->os_family ) != HB_NULLPTR )
    {
        return HB_TRUE;
    }

    if( __hb_json_string_strstr( _search, _record->os_version ) != HB_NULLPTR )
    {
        return HB_TRUE;
    }

    if( CH_HAS_RECORD_FLAG( _record->flags, CH_RECORD_ATTRIBUTE_ATTRIBUTES ) )
    {
        HB_RING_FOREACH( record, ch_attribute_t, _record->attributes, attribute )
        {
            if( __hb_json_string_strstr( _search, attribute->name ) != HB_NULLPTR )
            {
                return HB_TRUE;
            }

            switch( attribute->value_type )
            {
            case CH_ATTRIBUTE_TYPE_STRING:
                {
                    if( __hb_json_string_strstr( _search, attribute->value_string ) != HB_NULLPTR )
                    {
                        return HB_TRUE;
                    }
                }break;
            }
        }
    }

    if( CH_HAS_RECORD_FLAG( _record->flags, CH_RECORD_ATTRIBUTE_TAGS ) )
    {
        HB_RING_FOREACH( record, ch_tag_t, _record->tags, tag )
        {
            if( __hb_json_string_strstr( _search, tag->value ) != HB_NULLPTR )
            {
                return HB_TRUE;
            }
        }
    }

    return HB_FALSE;
}
//////////////////////////////////////////////////////////////////////////
static hb_bool_t __ch_service_records_filter_search_integer( uint64_t _search, const ch_record_t * _record )
{
    if( _search == _record->timestamp )
    {
        return HB_TRUE;
    }

    if( _search == _record->live )
    {
        return HB_TRUE;
    }

    if( _search == _record->build_number )
    {
        return HB_TRUE;
    }

    if( CH_HAS_RECORD_FLAG( _record->flags, CH_RECORD_ATTRIBUTE_ATTRIBUTES ) )
    {
        HB_RING_FOREACH( record, ch_attribute_t, _record->attributes, attribute )
        {
            switch( attribute->value_type )
            {
            case CH_ATTRIBUTE_TYPE_INTEGER:
                {
                    if( _search == attribute->value_integer )
                    {
                        return HB_TRUE;
                    }
                }break;
            }
        }
    }

    return HB_FALSE;
}
//////////////////////////////////////////////////////////////////////////
static hb_bool_t __ch_service_records_filter_tag( hb_json_string_t _tag, const ch_record_t * _record )
{
    HB_RING_FOREACH( record, ch_tag_t, _record->tags, tag )
    {
        if( __hb_json_string_strstr( _tag, tag->value ) == HB_NULLPTR )
        {
            continue;
        }

        return HB_TRUE;
    }

    return HB_FALSE;
}
//////////////////////////////////////////////////////////////////////////
static hb_bool_t __ch_service_records_filter_string_arguments( hb_json_string_t _name, hb_json_string_t _value, const ch_record_t * _record )
{
    HB_RING_FOREACH( record, ch_attribute_t, _record->attributes, attribute )
    {
        if( attribute->value_type != CH_ATTRIBUTE_TYPE_STRING ||
            __hb_json_string_strstr( _name, attribute->name ) == HB_NULLPTR ||
            __hb_json_string_strstr( _value, attribute->value_string ) == HB_NULLPTR )
        {
            continue;
        }

        return HB_TRUE;
    }

    return HB_FALSE;
}
//////////////////////////////////////////////////////////////////////////
static void __ch_service_records_visitor_t( uint64_t _index, const ch_record_t * _record, void * _ud )
{
    ch_records_visitor_select_t * ud = (ch_records_visitor_select_t *)_ud;

    const ch_records_filter_t * filter = &ud->filter;

    if( filter->flags != CH_RECORD_ATTRIBUTE_NONE )
    {
        if( (_record->flags & filter->flags) != filter->flags )
        {
            return;
        }

        if( CH_HAS_RECORD_FLAG( filter->flags, CH_RECORD_ATTRIBUTE_USER_ID ) && __hb_json_string_strstr( filter->user_id, _record->user_id ) == HB_NULLPTR )
        {
            return;
        }

        if( CH_HAS_RECORD_FLAG( filter->flags, CH_RECORD_ATTRIBUTE_LEVEL ) && filter->level != _record->level )
        {
            return;
        }

        if( CH_HAS_RECORD_FLAG( filter->flags, CH_RECORD_ATTRIBUTE_SERVICE ) && __hb_json_string_strstr( filter->service, _record->service ) == HB_NULLPTR )
        {
            return;
        }

        if( CH_HAS_RECORD_FLAG( filter->flags, CH_RECORD_ATTRIBUTE_THREAD ) && __hb_json_string_strstr( filter->thread, _record->thread ) == HB_NULLPTR )
        {
            return;
        }

        if( CH_HAS_RECORD_FLAG( filter->flags, CH_RECORD_ATTRIBUTE_MESSAGE ) && __hb_json_string_strstr( filter->message, _record->message->text ) == HB_NULLPTR )
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

        if( CH_HAS_RECORD_FLAG( filter->flags, CH_RECORD_ATTRIBUTE_BUILD_ENVIRONMENT ) && __hb_json_string_strstr( filter->build_environment, _record->build_environment ) == HB_NULLPTR )
        {
            return;
        }

        if( CH_HAS_RECORD_FLAG( filter->flags, CH_RECORD_ATTRIBUTE_BUILD_RELEASE ) && filter->build_release != _record->build_release )
        {
            return;
        }

        if( CH_HAS_RECORD_FLAG( filter->flags, CH_RECORD_ATTRIBUTE_BUILD_VERSION ) && __hb_json_string_strstr( filter->build_version, _record->build_version ) == HB_NULLPTR )
        {
            return;
        }

        if( CH_HAS_RECORD_FLAG( filter->flags, CH_RECORD_ATTRIBUTE_BUILD_NUMBER ) && filter->build_number != _record->build_number )
        {
            return;
        }

        if( CH_HAS_RECORD_FLAG( filter->flags, CH_RECORD_ATTRIBUTE_DEVICE_MODEL ) && __hb_json_string_strstr( filter->device_model, _record->device_model ) == HB_NULLPTR )
        {
            return;
        }

        if( CH_HAS_RECORD_FLAG( filter->flags, CH_RECORD_ATTRIBUTE_OS_FAMILY ) && __hb_json_string_strstr( filter->os_family, _record->os_family ) == HB_NULLPTR )
        {
            return;
        }

        if( CH_HAS_RECORD_FLAG( filter->flags, CH_RECORD_ATTRIBUTE_OS_VERSION ) && __hb_json_string_strstr( filter->os_version, _record->os_version ) == HB_NULLPTR )
        {
            return;
        }

        if( CH_HAS_RECORD_FLAG( filter->flags, CH_RECORD_ATTRIBUTE_ATTRIBUTES ) && CH_HAS_RECORD_FLAG( _record->flags, CH_RECORD_ATTRIBUTE_ATTRIBUTES ) )
        {
            for( hb_size_t attributes_index = 0; attributes_index != filter->attributes_count; ++attributes_index )
            {
                const ch_records_filter_attribute_t * attribute = filter->attributes + attributes_index;

                if( attribute->type == e_records_attributes_string )
                {
                    if( __ch_service_records_filter_string_arguments( attribute->name, attribute->value_string, _record ) == HB_FALSE )
                    {
                        return;
                    }
                }
            }
        }
    }

    if( ud->search.value != HB_NULLPTR )
    {
        if( __ch_service_records_filter_search( ud->search, _record ) == HB_FALSE )
        {
            return;
        }

        if( ud->search_is_integer == HB_TRUE )
        {
            if( __ch_service_records_filter_search_integer( ud->search_integer, _record ) == HB_FALSE )
            {
                return;
            }
        }
    }

    if( ud->tags_count != 0 && CH_HAS_RECORD_FLAG( _record->flags, CH_RECORD_ATTRIBUTE_TAGS ) )
    {
        for( hb_size_t tags_index = 0; tags_index != ud->tags_count; ++tags_index )
        {
            hb_json_string_t tag_value = ud->tags[tags_index];

            if( __ch_service_records_filter_tag( tag_value, _record ) == HB_FALSE )
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

    if( CH_HAS_RECORD_FLAG( _record->flags, CH_RECORD_ATTRIBUTE_THREAD ) )
    {
        __response_write( ud, ",\"thread\":\"%s\""
            , _record->thread
        );
    }

    if( CH_HAS_RECORD_FLAG( _record->flags, CH_RECORD_ATTRIBUTE_MESSAGE ) )
    {
        __response_write( ud, ",\"message\":\"%s\""
            , _record->message->text
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

        HB_RING_FOREACH( record, ch_attribute_t, _record->attributes, attribute )
        {
            if( attribute != _record->attributes )
            {
                __response_write( ud, "," );
            }

            switch( attribute->value_type )
            {
            case CH_ATTRIBUTE_TYPE_BOOLEAN:
                {
                    __response_write( ud, "\"%s\":%s"
                        , attribute->name
                        , attribute->value_boolean == HB_TRUE ? "true" : "false"
                    );
                } break;
            case CH_ATTRIBUTE_TYPE_INTEGER:
                {
                    __response_write( ud, "\"%s\":%" PRIu64 ""
                        , attribute->name
                        , attribute->value_integer
                    );
                } break;
            case CH_ATTRIBUTE_TYPE_STRING:
                {
                    __response_write( ud, "\"%s\":\"%s\""
                        , attribute->name
                        , attribute->value_string
                    );
                } break;
            }
        }

        __response_write( ud, "}" );
    }

    if( CH_HAS_RECORD_FLAG( _record->flags, CH_RECORD_ATTRIBUTE_TAGS ) )
    {
        __response_write( ud, ",\"tags\":[" );

        HB_RING_FOREACH( record, ch_tag_t, _record->tags, tag )
        {
            if( tag != _record->tags )
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
static void __select_filter_get_field_string( ch_records_visitor_select_t * _ud, uint32_t _tag, const hb_json_handle_t * _json, const char * _key, hb_json_string_t * const _value )
{
    if( hb_json_get_field_string( _json, _key, _value ) == HB_SUCCESSFUL )
    {
        _ud->filter.flags |= CH_GET_RECORD_FLAG( _tag );
    }
}
//////////////////////////////////////////////////////////////////////////
static void __select_filter_get_field_boolean( ch_records_visitor_select_t * _ud, uint32_t _tag, const hb_json_handle_t * _json, const char * _key, hb_bool_t * const _value )
{
    if( hb_json_get_field_boolean( _json, _key, _value ) == HB_SUCCESSFUL )
    {
        _ud->filter.flags |= CH_GET_RECORD_FLAG( _tag );
    }
}
//////////////////////////////////////////////////////////////////////////
static void __select_filter_get_field_uint32( ch_records_visitor_select_t * _ud, uint32_t _tag, const hb_json_handle_t * _json, const char * _key, uint32_t * const _value )
{
    if( hb_json_get_field_uint32( _json, _key, _value ) == HB_SUCCESSFUL )
    {
        _ud->filter.flags |= CH_GET_RECORD_FLAG( _tag );
    }
}
//////////////////////////////////////////////////////////////////////////
static void __select_filter_get_field_uint64( ch_records_visitor_select_t * _ud, uint32_t _tag, const hb_json_handle_t * _json, const char * _key, uint64_t * const _value )
{
    if( hb_json_get_field_uint64( _json, _key, _value ) == HB_SUCCESSFUL )
    {
        _ud->filter.flags |= CH_GET_RECORD_FLAG( _tag );
    }
}
//////////////////////////////////////////////////////////////////////////
static hb_bool_t __strntoull( const char * _str, size_t _size, uint64_t * const _value )
{
    char buffer[20 + 1];

    size_t sz = _size;
    const char * it = _str;

    for( ; it && sz && *it == ' '; it++, sz-- )
        ;

    for( ; it && sz && *(it + sz - 1) == ' '; sz-- )
        ;

    if( sz == 0 || sz >= sizeof( buffer ) )
    {
        return HB_FALSE;
    }

    memcpy( buffer, it, sz );
    buffer[sz] = '\0';

    char * end;
    unsigned long long ret = strtoull( buffer, &end, 10 );

    if( buffer == end )
    {
        return HB_FALSE;
    }

    *_value = (uint64_t)ret;

    return HB_TRUE;
}
//////////////////////////////////////////////////////////////////////////
ch_http_code_t ch_grid_request_select( const ch_grid_request_args_t * _args )
{
    hb_time_t timestamp;
    hb_time( &timestamp );

    hb_time_t time_offset = 0;
    hb_time_t time_limit = HB_TIME_SECONDS_IN_DAY;
    hb_size_t count_limit = 100;
    hb_json_get_field_uint64( _args->json, "time.offset", &time_offset );
    hb_json_get_field_uint64( _args->json, "time.limit", &time_limit );
    hb_json_get_field_size_t( _args->json, "count.limit", &count_limit );

    ch_records_visitor_select_t ud;

    ud.filter.flags = CH_RECORD_ATTRIBUTE_NONE;

    const hb_json_handle_t * json_filter = HB_NULLPTR;
    if( hb_json_get_field( _args->json, "filter", &json_filter ) == HB_SUCCESSFUL )
    {
        if( hb_json_is_object( json_filter ) == HB_FALSE )
        {
            snprintf( _args->reason, CH_GRID_REASON_MAX_SIZE, "filter is not object" );

            return CH_HTTP_BADREQUEST;
        }

        __select_filter_get_field_string( &ud, CH_RECORD_ATTRIBUTE_USER_ID, json_filter, "user.id", &ud.filter.user_id );
        __select_filter_get_field_string( &ud, CH_RECORD_ATTRIBUTE_SERVICE, json_filter, "service", &ud.filter.service );
        __select_filter_get_field_string( &ud, CH_RECORD_ATTRIBUTE_MESSAGE, json_filter, "message", &ud.filter.message );
        __select_filter_get_field_uint32( &ud, CH_RECORD_ATTRIBUTE_LEVEL, json_filter, "level", &ud.filter.level );
        __select_filter_get_field_uint64( &ud, CH_RECORD_ATTRIBUTE_TIMESTAMP, json_filter, "timestamp", &ud.filter.timestamp );
        __select_filter_get_field_uint64( &ud, CH_RECORD_ATTRIBUTE_LIVE, json_filter, "live", &ud.filter.live );
        __select_filter_get_field_string( &ud, CH_RECORD_ATTRIBUTE_BUILD_ENVIRONMENT, json_filter, "build.environment", &ud.filter.build_environment );
        __select_filter_get_field_boolean( &ud, CH_RECORD_ATTRIBUTE_BUILD_RELEASE, json_filter, "build.release", &ud.filter.build_release );
        __select_filter_get_field_string( &ud, CH_RECORD_ATTRIBUTE_BUILD_VERSION, json_filter, "build.version", &ud.filter.build_version );
        __select_filter_get_field_uint64( &ud, CH_RECORD_ATTRIBUTE_BUILD_NUMBER, json_filter, "build.number", &ud.filter.build_number );
        __select_filter_get_field_string( &ud, CH_RECORD_ATTRIBUTE_DEVICE_MODEL, json_filter, "device.model", &ud.filter.device_model );
        __select_filter_get_field_string( &ud, CH_RECORD_ATTRIBUTE_OS_FAMILY, json_filter, "os.family", &ud.filter.os_family );
        __select_filter_get_field_string( &ud, CH_RECORD_ATTRIBUTE_OS_VERSION, json_filter, "os.version", &ud.filter.os_version );

        const hb_json_handle_t * json_attributes;
        if( hb_json_get_field( json_filter, "attributes", &json_attributes ) == HB_SUCCESSFUL )
        {
            ud.filter.flags |= CH_GET_RECORD_FLAG( CH_RECORD_ATTRIBUTE_ATTRIBUTES );

            if( hb_json_is_array( json_attributes ) == HB_FALSE )
            {
                snprintf( _args->reason, CH_GRID_REASON_MAX_SIZE, "filter.attributes is not array" );

                return CH_HTTP_BADREQUEST;
            }

            hb_size_t attributes_count = hb_json_get_array_size( json_attributes );

            for( hb_size_t index = 0; index != attributes_count; ++index )
            {
                const hb_json_handle_t * json_attribute;
                if( hb_json_array_get_element( json_attributes, index, &json_attribute ) == HB_FAILURE )
                {
                    snprintf( _args->reason, CH_GRID_REASON_MAX_SIZE, "filter.attributes[%u] is not object", index );

                    return CH_HTTP_BADREQUEST;
                }

                if( hb_json_get_field_string( json_attribute, "name", &ud.filter.attributes[index].name ) == HB_FAILURE )
                {
                    snprintf( _args->reason, CH_GRID_REASON_MAX_SIZE, "filter.attributes[%u].name is not string", index );

                    return CH_HTTP_BADREQUEST;
                }

                const hb_json_handle_t * attribute_value;
                if( hb_json_get_field( json_attribute, "value", &attribute_value ) == HB_FAILURE )
                {
                    snprintf( _args->reason, CH_GRID_REASON_MAX_SIZE, "filter.attributes[%u].value is not found", index );

                    return CH_HTTP_BADREQUEST;
                }

                ch_records_filter_attribute_t * filter_attribute = ud.filter.attributes + index;

                hb_json_type_e attribute_value_type = hb_json_get_type( attribute_value );

                switch( attribute_value_type )
                {
                case e_hb_json_false:
                    {
                        filter_attribute->value_boolean = HB_FALSE;
                    }break;
                case e_hb_json_true:
                    {
                        filter_attribute->value_boolean = HB_TRUE;
                    }break;
                case e_hb_json_integer:
                    {
                        if( hb_json_get_field_uint64( json_attribute, "value", &filter_attribute->value_integer ) == HB_FAILURE )
                        {
                            snprintf( _args->reason, CH_GRID_REASON_MAX_SIZE, "filter.attributes[%u].value is not string", index );

                            return CH_HTTP_BADREQUEST;
                        }
                    }break;
                case e_hb_json_string:
                    {
                        if( hb_json_get_field_string( json_attribute, "value", &filter_attribute->value_string ) == HB_FAILURE )
                        {
                            snprintf( _args->reason, CH_GRID_REASON_MAX_SIZE, "filter.attributes[%u].value is not string", index );

                            return CH_HTTP_BADREQUEST;
                        }
                    }break;
                default:
                    {
                        snprintf( _args->reason, CH_GRID_REASON_MAX_SIZE, "filter.attributes[%u].value is not boolean, integer or string", index );

                        return CH_HTTP_BADREQUEST;
                        break;
                    }
                }
            }

            ud.filter.attributes_count = attributes_count;
        }

        const hb_json_handle_t * json_tags;
        if( hb_json_get_field( json_filter, "tags", &json_tags ) == HB_SUCCESSFUL )
        {
            ud.filter.flags |= CH_GET_RECORD_FLAG( CH_RECORD_ATTRIBUTE_TAGS );

            if( hb_json_is_array( json_tags ) == HB_FALSE )
            {
                snprintf( _args->reason, CH_GRID_REASON_MAX_SIZE, "filter.tags is not array" );

                return CH_HTTP_BADREQUEST;
            }

            hb_size_t tags_count = hb_json_get_array_size( json_tags );

            for( hb_size_t index = 0; index != tags_count; ++index )
            {
                const hb_json_handle_t * json_tag;
                if( hb_json_array_get_element( json_tags, index, &json_tag ) == HB_FAILURE )
                {
                    snprintf( _args->reason, CH_GRID_REASON_MAX_SIZE, "invalid get element filter.tags[%u]", index );

                    return CH_HTTP_BADREQUEST;
                }

                if( hb_json_to_string( json_tag, &ud.tags[index] ) == HB_FAILURE )
                {
                    snprintf( _args->reason, CH_GRID_REASON_MAX_SIZE, "filter.tags[%u] is not string", index );

                    return CH_HTTP_BADREQUEST;
                }
            }

            ud.tags_count = tags_count;
        }
    }

    ud.search.value = HB_NULLPTR;
    ud.search.size = 0;
    ud.search_is_integer = HB_FALSE;
    ud.search_integer = 0;

    const hb_json_handle_t * json_search;
    if( hb_json_get_field( _args->json, "search", &json_search ) == HB_SUCCESSFUL )
    {
        hb_json_to_string( json_search, &ud.search );

        ud.search_is_integer = __strntoull( ud.search.value, ud.search.size, &ud.search_integer );
    }

    ud.tags_count = 0;

    const hb_json_handle_t * json_tags;
    if( hb_json_get_field( _args->json, "tags", &json_tags ) == HB_SUCCESSFUL )
    {
        if( hb_json_is_array( json_tags ) == HB_FALSE )
        {
            snprintf( _args->reason, CH_GRID_REASON_MAX_SIZE, "tags is not array" );

            return CH_HTTP_BADREQUEST;
        }

        hb_size_t tags_count = hb_json_get_array_size( json_tags );

        for( hb_size_t index = 0; index != tags_count; ++index )
        {
            const hb_json_handle_t * json_tag;
            if( hb_json_array_get_element( json_tags, index, &json_tag ) == HB_FAILURE )
            {
                snprintf( _args->reason, CH_GRID_REASON_MAX_SIZE, "invalid get element tags[%u]", index );

                return CH_HTTP_BADREQUEST;
            }

            if( hb_json_to_string( json_tag, &ud.tags[index] ) == HB_FAILURE )
            {
                snprintf( _args->reason, CH_GRID_REASON_MAX_SIZE, "tags[%u] is not string", index );

                return CH_HTTP_BADREQUEST;
            }
        }

        ud.tags_count = tags_count;
    }

    ud.response = _args->response;
    ud.capacity = CH_GRID_RESPONSE_DATA_MAX_SIZE;
    ud.size = _args->response_size;

    __response_write( &ud, "{\"project\":\"%s\",\"records\":[", _args->project );

    ch_service_select_records( _args->service, _args->project, timestamp, time_offset, time_limit, count_limit, &__ch_service_records_visitor_t, &ud );

    __response_write( &ud, "]}" );

    return CH_HTTP_OK;
}
//////////////////////////////////////////////////////////////////////////