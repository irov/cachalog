#include "cachalog_mutex.h"

#include "cachalog_memory/cachalog_memory.h"
#include "cachalog_log/cachalog_log.h"

#include "cachalog_platform/cachalog_platform_windows.h"

//////////////////////////////////////////////////////////////////////////
typedef struct ch_mutex_handle_t
{
    CRITICAL_SECTION section;
}ch_mutex_handle_t;
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_mutex_create( ch_mutex_handle_t ** _handle )
{
    ch_mutex_handle_t * handle = CH_NEW( ch_mutex_handle_t );

    InitializeCriticalSection( &handle->section );

    *_handle = handle;

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void ch_mutex_destroy( ch_mutex_handle_t * _handle )
{
    DeleteCriticalSection( &_handle->section );

    CH_DELETE( _handle );
}
//////////////////////////////////////////////////////////////////////////
ch_bool_t ch_mutex_try_lock( ch_mutex_handle_t * _handle )
{
    if( TryEnterCriticalSection( &_handle->section ) == FALSE )
    {
        return CH_FALSE;
    }

    return CH_TRUE;
}
//////////////////////////////////////////////////////////////////////////
void ch_mutex_lock( ch_mutex_handle_t * _handle )
{
    EnterCriticalSection( &_handle->section );
}
//////////////////////////////////////////////////////////////////////////
void ch_mutex_unlock( ch_mutex_handle_t * _handle )
{
    LeaveCriticalSection( &_handle->section );
}
//////////////////////////////////////////////////////////////////////////