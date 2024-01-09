#ifndef CACHALOT_JSON_H_
#define CACHALOT_JSON_H_

#include "cachalot_config/cachalot_config.h"

typedef struct ch_json_handle_t ch_json_handle_t;

ch_result_t ch_json_create( const void * _data, ch_size_t _size, void * _pool, ch_size_t _capacity, ch_json_handle_t ** _handle );

typedef enum ch_json_type_t
{
    e_ch_json_object,
    e_ch_json_array,
    e_ch_json_string,
    e_ch_json_integer,
    e_ch_json_real,
    e_ch_json_true,
    e_ch_json_false,
    e_ch_json_null,
    e_ch_json_invalid,
} ch_json_type_t;

ch_json_type_t ch_json_get_type( const ch_json_handle_t * _handle );

ch_bool_t ch_json_is_num( const ch_json_handle_t * _handle );

ch_result_t ch_json_get_array_count( const ch_json_handle_t * _handle, ch_size_t * const _count );
ch_result_t ch_json_get_array_element( const ch_json_handle_t * _handle, ch_size_t _index, ch_json_handle_t ** _out );

ch_result_t ch_json_get_field( const ch_json_handle_t * _handle, const char * _key, ch_json_handle_t ** _out );
ch_result_t ch_json_get_fields_count( const ch_json_handle_t * _handle, ch_size_t * const _count );

ch_result_t ch_json_to_string( const ch_json_handle_t * _handle, const char ** _value, ch_size_t * const _length );
ch_result_t ch_json_copy_string( const ch_json_handle_t * _handle, char * _value, ch_size_t _capacity, ch_size_t * const _length );
ch_result_t ch_json_to_boolean( const ch_json_handle_t * _handle, ch_bool_t * const _value );
ch_result_t ch_json_to_int16( const ch_json_handle_t * _handle, int16_t * const _value );
ch_result_t ch_json_to_int32( const ch_json_handle_t * _handle, int32_t * const _value );
ch_result_t ch_json_to_int64( const ch_json_handle_t * _handle, int64_t * const _value );
ch_result_t ch_json_to_uint16( const ch_json_handle_t * _handle, uint16_t * const _value );
ch_result_t ch_json_to_uint32( const ch_json_handle_t * _handle, uint32_t * const _value );
ch_result_t ch_json_to_uint64( const ch_json_handle_t * _handle, uint64_t * const _value );
ch_result_t ch_json_to_size_t( const ch_json_handle_t * _handle, ch_size_t * const _value );
ch_result_t ch_json_to_real( const ch_json_handle_t * _handle, double * const _value );

ch_result_t ch_json_get_field_string_length( const ch_json_handle_t * _handle, const char * _key, ch_size_t * _length );
ch_result_t ch_json_get_field_string( const ch_json_handle_t * _handle, const char * _key, const char ** _value, ch_size_t * _length, const char * _default );
ch_result_t ch_json_copy_field_string( const ch_json_handle_t * _handle, const char * _key, char * const _value, ch_size_t _capacity, ch_size_t * _length, const char * _default );
ch_result_t ch_json_get_field_boolean( const ch_json_handle_t * _handle, const char * _key, ch_bool_t * const _value, int16_t _default );
ch_result_t ch_json_get_field_int16( const ch_json_handle_t * _handle, const char * _key, int16_t * const _value, int16_t _default );
ch_result_t ch_json_get_field_int32( const ch_json_handle_t * _handle, const char * _key, int32_t * const _value, int32_t _default );
ch_result_t ch_json_get_field_int64( const ch_json_handle_t * _handle, const char * _key, int64_t * const _value, int64_t _default );
ch_result_t ch_json_get_field_uint16( const ch_json_handle_t * _handle, const char * _key, uint16_t * const _value, uint16_t _default );
ch_result_t ch_json_get_field_uint32( const ch_json_handle_t * _handle, const char * _key, uint32_t * const _value, uint32_t _default );
ch_result_t ch_json_get_field_uint64( const ch_json_handle_t * _handle, const char * _key, uint64_t * const _value, uint64_t _default );
ch_result_t ch_json_get_field_size_t( const ch_json_handle_t * _handle, const char * _key, ch_size_t * const _value, ch_size_t _default );
ch_result_t ch_json_get_field_string_length_required( const ch_json_handle_t * _handle, const char * _key, ch_size_t * const _length, ch_bool_t * const _required );
ch_result_t ch_json_copy_field_string_required( const ch_json_handle_t * _handle, const char * _key, char * const _value, ch_size_t _capacity, ch_size_t * _length, ch_bool_t * const _required );
ch_result_t ch_json_get_field_boolean_required( const ch_json_handle_t * _handle, const char * _key, ch_bool_t * const _value, ch_bool_t * const _required );
ch_result_t ch_json_get_field_int32_required( const ch_json_handle_t * _handle, const char * _key, int32_t * const _value, ch_bool_t * const _required );
ch_result_t ch_json_get_field_int64_required( const ch_json_handle_t * _handle, const char * _key, int64_t * const _value, ch_bool_t * const _required );
ch_result_t ch_json_get_field_uint32_required( const ch_json_handle_t * _handle, const char * _key, uint32_t * const _value, ch_bool_t * const _required );
ch_result_t ch_json_get_field_uint64_required( const ch_json_handle_t * _handle, const char * _key, uint64_t * const _value, ch_bool_t * const _required );

typedef ch_result_t( *ch_json_object_visitor_t )(ch_size_t _index, const ch_json_handle_t * _key, const ch_json_handle_t * _value, void * _ud);
typedef ch_result_t( *ch_json_array_visitor_t )(ch_size_t _index, const ch_json_handle_t * _value, void * _ud);

ch_result_t ch_json_foreach_object( const ch_json_handle_t * _handle, ch_json_object_visitor_t _visitor, void * _ud );
ch_result_t ch_json_foreach_array( const ch_json_handle_t * _handle, ch_json_array_visitor_t _visitor, void * _ud );

ch_result_t ch_json_foreach_field_object( const ch_json_handle_t * _handle, const char * _key, ch_json_object_visitor_t _visitor, void * _ud );
ch_result_t ch_json_foreach_field_array( const ch_json_handle_t * _handle, const char * _key, ch_json_array_visitor_t _visitor, void * _ud );
ch_result_t ch_json_foreach_field_object_required( const ch_json_handle_t * _handle, const char * _key, ch_json_object_visitor_t _visitor, void * _ud, ch_bool_t * const _required );
ch_result_t ch_json_foreach_field_array_required( const ch_json_handle_t * _handle, const char * _key, ch_json_array_visitor_t _visitor, void * _ud, ch_bool_t * const _required );

#endif
