#include "cachalog_http.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////
ch_result_t ch_http_get_request_data( struct evhttp_request * _request, void * _buffer, ch_size_t _capacity, ch_size_t * _size )
{
    struct evbuffer * input_buffer = evhttp_request_get_input_buffer( _request );

    ch_size_t multipart_length = evbuffer_get_length( input_buffer );

    if( multipart_length + 1 > _capacity )
    {
        return CH_FAILURE;
    }

    ev_ssize_t copyout_buffer_size = evbuffer_copyout( input_buffer, _buffer, multipart_length );

    if( copyout_buffer_size < 0 )
    {
        return CH_FAILURE;
    }

    ((char *)_buffer)[copyout_buffer_size] = '\0';

    *_size = copyout_buffer_size;

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_bool_t ch_http_is_request_json( struct evhttp_request * _request )
{
    struct evkeyvalq * headers = evhttp_request_get_input_headers( _request );

    const char * content_type = evhttp_find_header( headers, "Content-Type" );

    if( content_type == CH_NULLPTR )
    {
        return CH_FALSE;
    }

    if( strcmp( content_type, "application/json" ) != 0 )
    {
        return CH_FALSE;
    }

    return CH_TRUE;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_http_get_request_json( struct evhttp_request * _request, void * _pool, ch_size_t _capacity, ch_json_handle_t ** _handle )
{
    if( ch_http_is_request_json( _request ) == CH_FALSE )
    {
        return CH_FAILURE;
    }

    ch_data_t data;
    ch_size_t data_size;
    if( ch_http_get_request_data( _request, data, CH_DATA_MAX_SIZE, &data_size ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    if( ch_json_create( data, data_size, _pool, _capacity, _handle ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_http_get_request_header( struct evhttp_request * _request, const char * _header, const char ** _value )
{
    enum evhttp_cmd_type command_type = evhttp_request_get_command( _request );
    CH_UNUSED( command_type );

    struct evkeyvalq * headers = evhttp_request_get_input_headers( _request );

    const char * value = evhttp_find_header( headers, _header );

    if( value == CH_NULLPTR )
    {
        return CH_FAILURE;
    }

    *_value = value;

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////