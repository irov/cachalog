#include "cachalog_file.h"
#include "cachalog_log/cachalog_log.h"

#include <stdio.h>

//////////////////////////////////////////////////////////////////////////
ch_result_t ch_file_read( const char * _path, void * _buffer, ch_size_t _capacity, ch_size_t * const _size )
{
    FILE * f = fopen( _path, "rb" );

    if( f == CH_NULLPTR )
    {
        CH_LOG_MESSAGE_CRITICAL( "file", "file '%s' not found"
            , _path
        );

        return CH_FAILURE;
    }

    fseek( f, 0L, SEEK_END );
    long sz = ftell( f );
    rewind( f );

    if( sz > (long)_capacity )
    {
        CH_LOG_MESSAGE_CRITICAL( "file", "file '%s' very large [%d]"
            , _path
            , _capacity
        );

        fclose( f );

        return CH_FAILURE;
    }

    ch_size_t r = fread( _buffer, sz, 1, f );

    fclose( f );

    if( r != 1 )
    {
        CH_LOG_MESSAGE_CRITICAL( "file", "file '%s' invalid read [%zu bytes]"
            , _path
            , sz
        );

        return CH_FAILURE;
    }

    if( _size != CH_NULLPTR )
    {
        *_size = sz;
    }

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_file_read_text( const char * _path, char * _buffer, ch_size_t _capacity, ch_size_t * const _size )
{
    ch_size_t size;
    if( ch_file_read( _path, _buffer, _capacity, &size ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    _buffer[size] = '\0';
    
    if( _size != CH_NULLPTR )
    {
        *_size = size;
    }

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////