#include "cachalog_log_console.h"

#include <stdio.h>

//////////////////////////////////////////////////////////////////////////
static void __ch_log_observer( const char * _category, ch_log_level_t _level, const char * _file, uint32_t _line, const char * _message, void * _ud )
{
    CH_UNUSED( _ud );

    const char * ls = ch_log_get_level_string( _level );

    printf( "%s [%s:%u] %s: %s\n", ls, _file, _line, _category, _message );
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_log_console_initialize()
{
    if( ch_log_add_observer( CH_NULLPTR, CH_LOG_ALL, &__ch_log_observer, CH_NULLPTR ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void ch_log_console_finalize()
{
    ch_log_remove_observer( &__ch_log_observer, CH_NULLPTR );
}
//////////////////////////////////////////////////////////////////////////