#ifndef CACHALOG_HTTP_H_
#define CACHALOG_HTTP_H_

#include "cachalog_config/cachalog_config.h"
#include "cachalog_json/cachalog_json.h"

#include "evhttp.h"

ch_result_t ch_http_get_request_data( struct evhttp_request * _request, void * _buffer, ch_size_t _capacity, ch_size_t * _size );
ch_bool_t ch_http_is_request_json( struct evhttp_request * _request );
ch_result_t ch_http_get_request_json( struct evhttp_request * _request, void * _pool, ch_size_t _capacity, ch_json_handle_t ** _handle );
ch_result_t ch_http_get_request_header( struct evhttp_request * _request, const char * _header, const char ** _value );

#endif
