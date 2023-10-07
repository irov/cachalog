#include "cachalog_thread.h"

#include "cachalog_memory/cachalog_memory.h"
#include "cachalog_log/cachalog_log.h"

#include "cachalog_platform/cachalog_platform_windows.h"

#include <process.h>
#include <errno.h>

//////////////////////////////////////////////////////////////////////////
typedef struct ch_thread_proxy_t
{
    ch_thread_function_t function;
    void * ud;
}ch_thread_proxy_t;
//////////////////////////////////////////////////////////////////////////
typedef struct ch_thread_handle_t
{
    uint32_t id;
    HANDLE handle;
    ch_thread_proxy_t * proxy;
}ch_thread_handle_t;
//////////////////////////////////////////////////////////////////////////
static uint32_t __stdcall __ch_thread_proxy( void * _ud )
{
    ch_thread_proxy_t * proxy = (ch_thread_proxy_t *)(_ud);

    (*proxy->function)(proxy->ud);

    return 0;
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_thread_create( ch_thread_function_t _function, void * _ud, ch_thread_handle_t ** _handle )
{
    ch_thread_proxy_t * proxy = CH_NEW( ch_thread_proxy_t );
    proxy->function = _function;
    proxy->ud = _ud;

    uint32_t id;
    uintptr_t th = _beginthreadex( CH_NULLPTR, 0, &__ch_thread_proxy, proxy, 0, &id );

    if( th == -1L )
    {
        CH_LOG_MESSAGE_ERROR( "thread", "invalid _beginthreadex errno: %d"
            , errno
        );

        return CH_FAILURE;
    }

    ch_thread_handle_t * handle = CH_NEW( ch_thread_handle_t );

    handle->id = id;
    handle->handle = (HANDLE)th;
    handle->proxy = proxy;

    *_handle = handle;

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void ch_thread_join( ch_thread_handle_t * _handle )
{
    HANDLE handle = _handle->handle;

    WaitForSingleObject( handle, INFINITE );
}
//////////////////////////////////////////////////////////////////////////
void ch_thread_destroy( ch_thread_handle_t * _handle )
{
    HANDLE handle = _handle->handle;

    CloseHandle( handle );

    CH_DELETE( _handle->proxy );
    CH_DELETE( _handle );
}
//////////////////////////////////////////////////////////////////////////