#include "cachalog_log.h"

#include "cachalog_memory/cachalog_memory.h"
#include "cachalog_mutex/cachalog_mutex.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifndef CH_LOG_MAX_OBSERVER
#define CH_LOG_MAX_OBSERVER 16
#endif

#ifndef CH_LOG_MAX_MESSAGE_SIZE
#define CH_LOG_MAX_MESSAGE_SIZE 2048
#endif

//////////////////////////////////////////////////////////////////////////
typedef struct ch_log_service_observer_desc_t
{
    ch_log_level_t level;
    char category[32];
    ch_log_observer_t observer;
    void * ud;
}ch_log_service_observer_desc_t;
//////////////////////////////////////////////////////////////////////////
typedef struct ch_log_service_handle_t
{
    uint32_t observer_count;
    ch_log_service_observer_desc_t observers[CH_LOG_MAX_OBSERVER];

    ch_mutex_handle_t * mutex;
}ch_log_service_handle_t;
//////////////////////////////////////////////////////////////////////////
static ch_log_service_handle_t * g_log_service_handle = CH_NULLPTR;
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_log_initialize()
{
    ch_log_service_handle_t * handle = CH_NEW( ch_log_service_handle_t );
    handle->observer_count = 0;

    ch_mutex_handle_t * mutex;
    if( ch_mutex_create( &mutex ) == CH_FAILURE )
    {
        return CH_FAILURE;
    }

    handle->mutex = mutex;

    g_log_service_handle = handle;
    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void ch_log_finalize()
{
    ch_mutex_destroy( g_log_service_handle->mutex );

    CH_DELETE( g_log_service_handle );
    g_log_service_handle = CH_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
const char * ch_log_get_level_string( ch_log_level_t _level )
{
    static const char * ch_log_level_string[] = {"all", "info", "warning", "error", "critical"};

    const char * str = ch_log_level_string[_level];

    return str;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_log_add_observer( const char * _category, ch_log_level_t _level, ch_log_observer_t _observer, void * _ud )
{
    if( g_log_service_handle->observer_count == CH_LOG_MAX_OBSERVER )
    {
        return CH_FAILURE;
    }

    ch_log_service_observer_desc_t desc;
    desc.level = _level;

    if( _category != CH_NULLPTR )
    {
        strcpy( desc.category, _category );
    }
    else
    {
        desc.category[0] = '\0';
    }

    desc.observer = _observer;
    desc.ud = _ud;

    g_log_service_handle->observers[g_log_service_handle->observer_count++] = desc;

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_log_remove_observer( ch_log_observer_t _observer, void ** _ud )
{
    for( int32_t i = 0; i != CH_LOG_MAX_OBSERVER; ++i )
    {
        ch_log_service_observer_desc_t * desc = g_log_service_handle->observers + i;

        if( desc->observer != _observer )
        {
            continue;
        }

        if( _ud != CH_NULLPTR )
        {
            *_ud = desc->ud;
        }

        g_log_service_handle->observers[i] = g_log_service_handle->observers[g_log_service_handle->observer_count - 1];

        return CH_SUCCESSFUL;
    }

    return CH_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
static void __ch_log_message_args( const char * _category, ch_log_level_t _level, const char * _file, uint32_t _line, const char * _message )
{
    uint32_t count = g_log_service_handle->observer_count;

    for( uint32_t i = 0; i != count; ++i )
    {
        ch_log_service_observer_desc_t * desc = g_log_service_handle->observers + i;

        if( desc->level > _level )
        {
            continue;
        }

        if( desc->category[0] != '\0' && strcmp( desc->category, _category ) != 0 )
        {
            continue;
        }

        (*desc->observer)(_category, _level, _file, _line, _message, desc->ud);
    }
}
//////////////////////////////////////////////////////////////////////////
void ch_log_message( const char * _category, ch_log_level_t _level, const char * _file, uint32_t _line, const char * _format, ... )
{
    va_list args;
    va_start( args, _format );

    char message[CH_LOG_MAX_MESSAGE_SIZE];
    int32_t n = vsprintf( message, _format, args );

    va_end( args );

    if( n <= 0 )
    {
        return;
    }

    ch_mutex_lock( g_log_service_handle->mutex );

    __ch_log_message_args( _category, _level, _file, _line, message );

    ch_mutex_unlock( g_log_service_handle->mutex );
}
//////////////////////////////////////////////////////////////////////////