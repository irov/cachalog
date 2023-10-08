#include "cachalog_json.h"

#include "cachalog_log/cachalog_log.h"
#include "cachalog_memory/cachalog_memory.h"

#include "yyjson.h"

#include <string.h>
#include <memory.h>

//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_create( const void * _data, ch_size_t _size, void * _pool, ch_size_t _capacity, ch_json_handle_t ** _handle )
{
    yyjson_alc alc;
    yyjson_alc_pool_init( &alc, _pool, _capacity );

    yyjson_read_err err;
    yyjson_doc * doc = yyjson_read_opts( (char *)_data, _size, YYJSON_READ_INSITU, &alc, &err );

    if( doc == CH_NULLPTR )
    {
        CH_LOG_MESSAGE_ERROR( "json", "json '%.*s' error pos: %zu code: %u message: %s"
            , _size
            , (const char *)_data
            , err.pos
            , err.code
            , err.msg
        );

        return CH_FAILURE;
    }

    yyjson_val * root = yyjson_doc_get_root( doc );

    *_handle = (ch_json_handle_t *)root;

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_array_count( const ch_json_handle_t * _handle, ch_size_t * const _size )
{
    yyjson_val * val = (yyjson_val *)_handle;

    if( yyjson_is_arr( val ) == false )
    {
        return CH_FAILURE;
    }

    size_t size = yyjson_arr_size( val );

    *_size = (ch_size_t)size;

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_array_get( const ch_json_handle_t * _handle, ch_size_t _index, ch_json_handle_t ** _out )
{
    yyjson_val * val = (yyjson_val *)_handle;

    yyjson_val * jvalue = yyjson_arr_get( val, _index );

    if( jvalue == CH_NULLPTR )
    {
        return CH_FAILURE;
    }

    *_out = (ch_json_handle_t *)jvalue;

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_get_field( const ch_json_handle_t * _handle, const char * _key, ch_json_handle_t ** _out )
{
    yyjson_val * val = (yyjson_val *)_handle;

    yyjson_val * jvalue = yyjson_obj_get( val, _key );

    if( jvalue == CH_NULLPTR )
    {
        *_out = CH_NULLPTR;

        return CH_FAILURE;
    }

    *_out = (ch_json_handle_t *)jvalue;

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_get_fields_count( const ch_json_handle_t * _handle, ch_size_t * const _size )
{
    yyjson_val * val = (yyjson_val *)_handle;

    if( yyjson_is_obj( val ) == false )
    {
        return CH_FAILURE;
    }

    size_t jcount = yyjson_obj_size( val );

    *_size = (ch_size_t)jcount;

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_json_type_t ch_json_get_type( const ch_json_handle_t * _handle )
{
    yyjson_val * val = (yyjson_val *)_handle;

    yyjson_type t = yyjson_get_type( val );

    switch( t )
    {
    case YYJSON_TYPE_OBJ:
        return e_ch_json_object;
    case YYJSON_TYPE_ARR:
        return e_ch_json_array;
    case YYJSON_TYPE_STR:
        return e_ch_json_string;
    case YYJSON_TYPE_NUM:
        {
            yyjson_subtype subtype = yyjson_get_subtype( val );

            switch( subtype )
            {
            case YYJSON_SUBTYPE_UINT:
                return e_ch_json_integer;
            case YYJSON_SUBTYPE_SINT:
                return e_ch_json_integer;
            case YYJSON_SUBTYPE_REAL:
                return e_ch_json_real;
            default:
                return e_ch_json_invalid;
            }
        }
    case YYJSON_TYPE_BOOL:
        {
            yyjson_subtype subtype = yyjson_get_subtype( val );

            switch( subtype )
            {
                case YYJSON_SUBTYPE_FALSE:
                    return e_ch_json_false;
                case YYJSON_SUBTYPE_TRUE:
                    return e_ch_json_true;
                default:
                    return e_ch_json_invalid;
            }
        }break;
    case YYJSON_TYPE_NULL:
        return e_ch_json_null;
    default:
        return e_ch_json_invalid;
    }
}
//////////////////////////////////////////////////////////////////////////
ch_bool_t ch_json_is_num( const ch_json_handle_t * _handle )
{
    yyjson_val * val = (yyjson_val *)_handle;

    if( yyjson_is_num( val ) == false )
    {
        return CH_FALSE;
    }

    return CH_TRUE;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_to_string( const ch_json_handle_t * _handle, const char ** _value, ch_size_t * const _size )
{
    yyjson_val * val = (yyjson_val *)_handle;

    if( yyjson_is_str( val ) == false )
    {
        return CH_FAILURE;
    }

    const char * value = unsafe_yyjson_get_str( val );
    ch_size_t size = unsafe_yyjson_get_len( val );

    *_value = value;

    if( _size != CH_NULLPTR )
    {
        *_size = size;
    }

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_copy_string( const ch_json_handle_t * _handle, char * _value, ch_size_t _capacity, ch_size_t * const _length )
{
    yyjson_val * val = (yyjson_val *)_handle;

    if( yyjson_is_str( val ) == false )
    {
        return CH_FAILURE;
    }

    const char * value = unsafe_yyjson_get_str( val );
    strncpy( _value, value, _capacity );

    if( _length != CH_NULLPTR )
    {
        ch_size_t length = unsafe_yyjson_get_len( val );
        *_length = length;
    }

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_to_bool( const ch_json_handle_t * _handle, ch_bool_t * const _value )
{
    yyjson_val * val = (yyjson_val *)_handle;

    if( yyjson_is_bool( val ) == false )
    {
        return CH_FAILURE;
    }

    bool value = unsafe_yyjson_get_bool( val );

    *_value = (ch_bool_t)value;

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_to_int16( const ch_json_handle_t * _handle, int16_t * const _value )
{
    yyjson_val * val = (yyjson_val *)_handle;

    if( yyjson_is_sint( val ) == true )
    {
        int64_t value = unsafe_yyjson_get_sint( val );

        *_value = (int16_t)value;

        return CH_SUCCESSFUL;
    }
    else if( yyjson_is_uint( val ) == true )
    {
        uint64_t value = unsafe_yyjson_get_uint( val );

        *_value = (int16_t)value;

        return CH_SUCCESSFUL;
    }
    else if( yyjson_is_real( val ) == true )
    {
        double value = unsafe_yyjson_get_real( val );

        *_value = (int16_t)value;

        return CH_SUCCESSFUL;
    }

    return CH_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_to_int32( const ch_json_handle_t * _handle, int32_t * const _value )
{
    yyjson_val * val = (yyjson_val *)_handle;

    if( yyjson_is_sint( val ) == true )
    {
        int64_t value = unsafe_yyjson_get_sint( val );

        *_value = (int32_t)value;

        return CH_SUCCESSFUL;
    }
    else if( yyjson_is_uint( val ) == true )
    {
        uint64_t value = unsafe_yyjson_get_uint( val );

        *_value = (int32_t)value;

        return CH_SUCCESSFUL;
    }
    else if( yyjson_is_real( val ) == true )
    {
        double value = unsafe_yyjson_get_real( val );

        *_value = (int32_t)value;

        return CH_SUCCESSFUL;
    }

    return CH_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_to_uint16( const ch_json_handle_t * _handle, uint16_t * const _value )
{
    yyjson_val * val = (yyjson_val *)_handle;

    if( yyjson_is_sint( val ) == true )
    {
        int64_t value = unsafe_yyjson_get_sint( val );

        *_value = (uint16_t)value;

        return CH_SUCCESSFUL;
    }
    else if( yyjson_is_uint( val ) == true )
    {
        uint64_t value = unsafe_yyjson_get_uint( val );

        *_value = (uint16_t)value;

        return CH_SUCCESSFUL;
    }
    else if( yyjson_is_real( val ) == true )
    {
        double value = unsafe_yyjson_get_real( val );

        *_value = (uint16_t)value;

        return CH_SUCCESSFUL;
    }

    return CH_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_to_uint32( const ch_json_handle_t * _handle, uint32_t * const _value )
{
    yyjson_val * val = (yyjson_val *)_handle;

    if( yyjson_is_sint( val ) == true )
    {
        int64_t value = unsafe_yyjson_get_sint( val );

        *_value = (uint32_t)value;

        return CH_SUCCESSFUL;
    }
    else if( yyjson_is_uint( val ) == true )
    {
        uint64_t value = unsafe_yyjson_get_uint( val );

        *_value = (uint32_t)value;

        return CH_SUCCESSFUL;
    }
    else if( yyjson_is_real( val ) == true )
    {
        double value = unsafe_yyjson_get_real( val );

        *_value = (uint32_t)value;

        return CH_SUCCESSFUL;
    }

    return CH_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_to_int64( const ch_json_handle_t * _handle, int64_t * const _value )
{
    yyjson_val * val = (yyjson_val *)_handle;

    if( yyjson_is_sint( val ) == true )
    {
        int64_t value = unsafe_yyjson_get_sint( val );

        *_value = (int64_t)value;

        return CH_SUCCESSFUL;
    }
    else if( yyjson_is_uint( val ) == true )
    {
        uint64_t value = unsafe_yyjson_get_uint( val );

        *_value = (int64_t)value;

        return CH_SUCCESSFUL;
    }
    else if( yyjson_is_real( val ) == true )
    {
        double value = unsafe_yyjson_get_real( val );

        *_value = (int64_t)value;

        return CH_SUCCESSFUL;
    }

    return CH_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_to_uint64( const ch_json_handle_t * _handle, uint64_t * const _value )
{
    yyjson_val * val = (yyjson_val *)_handle;

    if( yyjson_is_sint( val ) == true )
    {
        int64_t value = unsafe_yyjson_get_sint( val );

        *_value = (uint64_t)value;

        return CH_SUCCESSFUL;
    }
    else if( yyjson_is_uint( val ) == true )
    {
        uint64_t value = unsafe_yyjson_get_uint( val );

        *_value = (uint64_t)value;

        return CH_SUCCESSFUL;
    }
    else if( yyjson_is_real( val ) == true )
    {
        double value = unsafe_yyjson_get_real( val );

        *_value = (uint64_t)value;

        return CH_SUCCESSFUL;
    }

    return CH_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_to_size_t( const ch_json_handle_t * _handle, ch_size_t * const _value )
{
    yyjson_val * val = (yyjson_val *)_handle;

    if( yyjson_is_sint( val ) == true )
    {
        int64_t value = unsafe_yyjson_get_sint( val );

        *_value = (ch_size_t)value;

        return CH_SUCCESSFUL;
    }
    else if( yyjson_is_uint( val ) == true )
    {
        uint64_t value = unsafe_yyjson_get_uint( val );

        *_value = (ch_size_t)value;

        return CH_SUCCESSFUL;
    }
    else if( yyjson_is_real( val ) == true )
    {
        double value = unsafe_yyjson_get_real( val );

        *_value = (ch_size_t)value;

        return CH_SUCCESSFUL;
    }

    return CH_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_to_real( const ch_json_handle_t * _handle, double * const _value )
{
    yyjson_val * val = (yyjson_val *)_handle;

    if( yyjson_is_sint( val ) == true )
    {
        int64_t value = unsafe_yyjson_get_sint( val );

        *_value = (double)value;

        return CH_SUCCESSFUL;
    }
    else if( yyjson_is_uint( val ) == true )
    {
        uint64_t value = unsafe_yyjson_get_uint( val );

        *_value = (double)value;

        return CH_SUCCESSFUL;
    }
    else if( yyjson_is_real( val ) == true )
    {
        double value = unsafe_yyjson_get_real( val );

        *_value = (double)value;

        return CH_SUCCESSFUL;
    }

    return CH_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_get_field_string( const ch_json_handle_t * _handle, const char * _key, const char ** _value, ch_size_t * _length, const char * _default )
{
    ch_json_handle_t * field;
    if( ch_json_get_field( _handle, _key, &field ) == CH_FAILURE )
    {
        *_value = _default;
        
        return CH_SUCCESSFUL;
    }

    if( ch_json_to_string( field, _value, _length ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_copy_field_string( const ch_json_handle_t * _handle, const char * _key, char * _value, ch_size_t _capacity, ch_size_t * _length, const char * _default )
{
    yyjson_val * val = (yyjson_val *)_handle;

    yyjson_val * field = yyjson_obj_get( val, _key );

    if( field == CH_NULLPTR )
    {
        if( _default != CH_NULLPTR )
        {
            strncpy( _value, _default, _capacity );

            if( _length != CH_NULLPTR )
            {
                size_t length = strlen( _default );
                *_length = length;
            }
        }

        return CH_SUCCESSFUL;
    }

    if( yyjson_is_str( field ) == false )
    {
        return CH_FAILURE;
    }

    const char * value = unsafe_yyjson_get_str( field );
    strncpy( _value, value, _capacity );

    if( _length != CH_NULLPTR )
    {
        size_t length = unsafe_yyjson_get_len( field );
        *_length = length;
    }

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_get_field_string_length_required( const ch_json_handle_t * _handle, const char * _key, ch_size_t * _length, ch_bool_t * const _result )
{
    yyjson_val * val = (yyjson_val *)_handle;

    yyjson_val * field = yyjson_obj_get( val, _key );

    if( field == CH_NULLPTR )
    {
        *_result = CH_FALSE;

        return CH_SUCCESSFUL;
    }

    if( yyjson_is_str( field ) == false )
    {
        return CH_FAILURE;
    }

    size_t length = unsafe_yyjson_get_len( field );
    
    *_length = length;

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_copy_field_string_required( const ch_json_handle_t * _handle, const char * _key, char * _value, ch_size_t _capacity, ch_size_t * _length, ch_bool_t * _result )
{
    yyjson_val * val = (yyjson_val *)_handle;

    yyjson_val * field = yyjson_obj_get( val, _key );

    if( field == CH_NULLPTR )
    {
        *_result = CH_FALSE;

        return CH_SUCCESSFUL;
    }

    if( yyjson_is_str( field ) == false )
    {
        return CH_FAILURE;
    }

    const char * value = unsafe_yyjson_get_str( field );
    strncpy( _value, value, _capacity );

    if( _length != CH_NULLPTR )
    {
        size_t length = unsafe_yyjson_get_len( field );
        *_length = length;
    }

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_get_field_int16( const ch_json_handle_t * _handle, const char * _key, int16_t * _value, int16_t _default )
{
    ch_json_handle_t * field;
    if( ch_json_get_field( _handle, _key, &field ) == CH_FAILURE )
    {
        *_value = _default;

        return CH_SUCCESSFUL;
    }

    if( ch_json_to_int16( field, _value ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_get_field_int32( const ch_json_handle_t * _handle, const char * _key, int32_t * _value, int32_t _default )
{
    ch_json_handle_t * field;
    if( ch_json_get_field( _handle, _key, &field ) == CH_FAILURE )
    {
        *_value = _default;

        return CH_SUCCESSFUL;
    }

    if( ch_json_to_int32( field, _value ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_get_field_int64( const ch_json_handle_t * _handle, const char * _key, int64_t * _value, int64_t _default )
{
    ch_json_handle_t * field;
    if( ch_json_get_field( _handle, _key, &field ) == CH_FAILURE )
    {
        *_value = _default;

        return CH_SUCCESSFUL;
    }

    if( ch_json_to_int64( field, _value ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_get_field_uint16( const ch_json_handle_t * _handle, const char * _key, uint16_t * const _value, uint16_t _default )
{
    ch_json_handle_t * field;
    if( ch_json_get_field( _handle, _key, &field ) == CH_FAILURE )
    {
        *_value = _default;

        return CH_SUCCESSFUL;
    }

    if( ch_json_to_uint16( field, _value ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_get_field_uint32( const ch_json_handle_t * _handle, const char * _key, uint32_t * const _value, uint32_t _default )
{
    ch_json_handle_t * field;
    if( ch_json_get_field( _handle, _key, &field ) == CH_FAILURE )
    {
        *_value = _default;

        return CH_SUCCESSFUL;
    }

    if( ch_json_to_uint32( field, _value ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_get_field_uint64( const ch_json_handle_t * _handle, const char * _key, uint64_t * _value, uint64_t _default )
{
    ch_json_handle_t * field;
    if( ch_json_get_field( _handle, _key, &field ) == CH_FAILURE )
    {
        *_value = _default;

        return CH_SUCCESSFUL;
    }

    if( ch_json_to_uint64( field, _value ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_get_field_size_t( const ch_json_handle_t * _handle, const char * _key, ch_size_t * _value, ch_size_t _default )
{
    ch_json_handle_t * field;
    if( ch_json_get_field( _handle, _key, &field ) == CH_FAILURE )
    {
        *_value = _default;

        return CH_SUCCESSFUL;
    }

    if( ch_json_to_size_t( field, _value ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_get_field_bool_required( const ch_json_handle_t * _handle, const char * _key, ch_bool_t * _value, ch_bool_t * const _required )
{
    ch_json_handle_t * field;
    if( ch_json_get_field( _handle, _key, &field ) == CH_FAILURE )
    {
        *_required = CH_FALSE;

        return CH_SUCCESSFUL;
    }

    if( ch_json_to_bool( field, _value ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_get_field_int32_required( const ch_json_handle_t * _handle, const char * _key, int32_t * _value, ch_bool_t * const _required )
{
    ch_json_handle_t * field;
    if( ch_json_get_field( _handle, _key, &field ) == CH_FAILURE )
    {
        *_required = CH_FALSE;

        return CH_SUCCESSFUL;
    }

    if( ch_json_to_int32( field, _value ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_get_field_uint32_required( const ch_json_handle_t * _handle, const char * _key, uint32_t * _value, ch_bool_t * const _required )
{
    ch_json_handle_t * field;
    if( ch_json_get_field( _handle, _key, &field ) == CH_FAILURE )
    {
        *_required = CH_FALSE;

        return CH_SUCCESSFUL;
    }

    if( ch_json_to_uint32( field, _value ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_get_field_int64_required( const ch_json_handle_t * _handle, const char * _key, int64_t * _value, ch_bool_t * const _required )
{
    ch_json_handle_t * field;
    if( ch_json_get_field( _handle, _key, &field ) == CH_FAILURE )
    {
        *_required = CH_FALSE;

        return CH_SUCCESSFUL;
    }

    if( ch_json_to_int64( field, _value ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_get_field_uint64_required( const ch_json_handle_t * _handle, const char * _key, uint64_t * _value, ch_bool_t * const _required )
{
    ch_json_handle_t * field;
    if( ch_json_get_field( _handle, _key, &field ) == CH_FAILURE )
    {
        *_required = CH_FALSE;

        return CH_SUCCESSFUL;
    }

    if( ch_json_to_uint64( field, _value ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_foreach_object( const ch_json_handle_t * _handle, ch_json_object_visitor_t _visitor, void * _ud )
{
    yyjson_val * obj = (yyjson_val *)_handle;

    size_t idx, max;
    yyjson_val * objkey;
    yyjson_val * objval;
    yyjson_obj_foreach( obj, idx, max, objkey, objval )
    {
        const ch_json_handle_t * objkeyhandle = (const ch_json_handle_t *)objkey;
        const ch_json_handle_t * objvalhandle = (const ch_json_handle_t *)objval;

        if( (*_visitor)(idx, objkeyhandle, objvalhandle, _ud) == CH_FAILURE )
        {
            return CH_FAILURE;
        }
    }

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_foreach_array( const ch_json_handle_t * _handle, ch_json_array_visitor_t _visitor, void * _ud )
{
    yyjson_val * arr = (yyjson_val *)_handle;

    size_t idx, max;
    yyjson_val * val;
    yyjson_arr_foreach( arr, idx, max, val )
    {
        const ch_json_handle_t * idhandle = (const ch_json_handle_t *)val;

        if( (*_visitor)(idx, idhandle, _ud) == CH_FAILURE )
        {
            return CH_FAILURE;
        }
    }

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_foreach_field_object( const ch_json_handle_t * _handle, const char * _key, ch_json_object_visitor_t _visitor, void * _ud )
{
    yyjson_val * val = (yyjson_val *)_handle;

    yyjson_val * field = yyjson_obj_get( val, _key );

    if( field == CH_NULLPTR )
    {
        return CH_FAILURE;
    }

    if( yyjson_is_obj( field ) == false )
    {
        return CH_FAILURE;
    }

    if( ch_json_foreach_object( (const ch_json_handle_t *)field, _visitor, _ud ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_foreach_field_array( const ch_json_handle_t * _handle, const char * _key, ch_json_array_visitor_t _visitor, void * _ud )
{
    yyjson_val * val = (yyjson_val *)_handle;

    yyjson_val * field = yyjson_obj_get( val, _key );

    if( field == CH_NULLPTR )
    {
        return CH_FAILURE;
    }

    if( yyjson_is_arr( field ) == false )
    {
        return CH_FAILURE;
    }

    if( ch_json_foreach_array( (const ch_json_handle_t *)field, _visitor, _ud ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_foreach_field_object_required( const ch_json_handle_t * _handle, const char * _key, ch_json_object_visitor_t _visitor, void * _ud, ch_bool_t * const _required )
{
    yyjson_val * val = (yyjson_val *)_handle;

    yyjson_val * field = yyjson_obj_get( val, _key );

    if( field == CH_NULLPTR )
    {
        *_required = CH_FALSE;

        return CH_SUCCESSFUL;
    }

    if( yyjson_is_obj( field ) == false )
    {
        return CH_FAILURE;
    }

    if( ch_json_foreach_object( (const ch_json_handle_t *)field, _visitor, _ud ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_json_foreach_field_array_required( const ch_json_handle_t * _handle, const char * _key, ch_json_array_visitor_t _visitor, void * _ud, ch_bool_t * const _required )
{
    yyjson_val * val = (yyjson_val *)_handle;

    yyjson_val * field = yyjson_obj_get( val, _key );

    if( field == CH_NULLPTR )
    {
        *_required = CH_FALSE;

        return CH_SUCCESSFUL;
    }

    if( yyjson_is_arr( field ) == false )
    {
        return CH_FAILURE;
    }

    if( ch_json_foreach_array( (const ch_json_handle_t *)field, _visitor, _ud ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////