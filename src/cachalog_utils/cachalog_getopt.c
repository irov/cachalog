#include "cachalog_getopt.h"

#include <string.h>

ch_result_t ch_getopt( int argc, char * argv[], const char * _name, const char ** _value )
{
    for( int index = 1; index != argc; ++index )
    {
        if( strcmp( argv[index], _name ) != 0 )
        {
            continue;
        }

        if( index + 1 == argc )
        {
            return CH_FAILURE;
        }

        *_value = argv[index + 1];

        return CH_SUCCESSFUL;
    }

    return CH_FAILURE;
}