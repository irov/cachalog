#include "cachalog_log_file.h"

#include "cachalog_memory/cachalog_memory.h"
#include "cachalog_utils/cachalog_date.h"

#include <stdio.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

//////////////////////////////////////////////////////////////////////////
typedef struct ch_log_file_handle_t
{
    FILE * f;
}ch_log_file_handle_t;
//////////////////////////////////////////////////////////////////////////
static void __ch_log_file_observer( const char * _category, ch_log_level_t _level, const char * _file, uint32_t _line, const char * _message, void * _ud )
{
    CH_UNUSED( _file );
    CH_UNUSED( _line );

    ch_log_file_handle_t * handle = (ch_log_file_handle_t *)_ud;

    const char * ls = ch_log_get_level_string( _level );

    ch_date_t d;
    ch_date( &d );

#ifdef CH_DEBUG
    fprintf( handle->f, "%s [%u.%u.%u][%u:%u:%u][%u] (%s) [%s:%u]: %s\n", ls, d.year, d.mon, d.mday, d.hour, d.min, d.sec, d.msec, _category, _file, _line, _message );
#else
    fprintf( handle->f, "%s [%u.%u.%u][%u:%u:%u][%u] (%s): %s\n", ls, d.year, d.mon, d.mday, d.hour, d.min, d.sec, d.msec, _category, _message );
#endif

    fflush( handle->f );
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_log_file_initialize( const char * _file )
{
    if( _file == CH_NULLPTR )
    {
        return CH_SUCCESSFUL;
    }

    FILE * f = fopen( _file, "wb" );

    if( f == CH_NULLPTR )
    {
        return CH_FAILURE;
    }

    ch_log_file_handle_t * handle = CH_NEW( ch_log_file_handle_t );
    handle->f = f;

    if( ch_log_add_observer( CH_NULLPTR, CH_LOG_VERBOSE, &__ch_log_file_observer, handle ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void ch_log_file_finalize()
{
    ch_log_file_handle_t * handle;
    if( ch_log_remove_observer( &__ch_log_file_observer, &handle ) == CH_FAILURE )
    {
        return;
    }
    
    fclose( handle->f );
    handle->f = CH_NULLPTR;

    CH_DELETE( handle );
}
//////////////////////////////////////////////////////////////////////////