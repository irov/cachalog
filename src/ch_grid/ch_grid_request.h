#ifndef CH_GRID_REQUEST_H_
#define CH_GRID_REQUEST_H_

#include "ch_config.h"
#include "ch_service.h"

#include "hb_json/hb_json.h"

typedef struct ch_grid_request_args_t
{
    uint32_t process_id;
    uint64_t request_id;
    ch_service_t * service;
    const char * project;
    const hb_json_handle_t * json;

    char * response;
    hb_size_t * response_size;

    char * reason;
} ch_grid_request_args_t;

typedef ch_http_code_t( *ch_grid_request_func_t )( const ch_grid_request_args_t * _args );

typedef struct ch_grid_cmd_inittab_t
{
    const char * name;
    ch_grid_request_func_t request;
} ch_grid_cmd_inittab_t;

ch_grid_request_func_t ch_grid_request_get( const char * _name );

#endif