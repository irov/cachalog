#include "ch_grid_request.h"

#include <string.h>

extern ch_http_code_t ch_grid_request_insert( const ch_grid_request_args_t * _args );
extern ch_http_code_t ch_grid_request_select( const ch_grid_request_args_t * _args );

static ch_grid_cmd_inittab_t grid_cmds[] =
{
    {"insert", &ch_grid_request_insert},
    {"select", &ch_grid_request_select},
};

ch_grid_request_func_t ch_grid_request_get( const char * _name )
{
    for( ch_grid_cmd_inittab_t
        * cmd_inittab = grid_cmds,
        *cmd_inittab_end = grid_cmds + sizeof( grid_cmds ) / sizeof( grid_cmds[0] );
        cmd_inittab != cmd_inittab_end;
        ++cmd_inittab )
    {
        if( strcmp( cmd_inittab->name, _name ) != 0 )
        {
            continue;
        }

        ch_grid_request_func_t request = cmd_inittab->request;

        return request;
    }

    return HB_NULLPTR;
}