#include "cachalog_thread.h"

#include "cachalog_memory/cachalog_memory.h"
#include "cachalog_log/cachalog_log.h"

#include <unistd.h>
#include <pthread.h>
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
    pthread_t id;
    ch_thread_proxy_t * proxy;
}ch_thread_handle_t;
//////////////////////////////////////////////////////////////////////////
static void * __ch_thread_proxy( void * _ud )
{
    ch_thread_proxy_t * proxy = (ch_thread_proxy_t *)(_ud);

    (*proxy->function)(proxy->ud);

    pthread_exit( CH_NULLPTR );
}
//////////////////////////////////////////////////////////////////////////
ch_result_t ch_thread_create( ch_thread_function_t _function, void * _ud, ch_thread_handle_t ** _handle )
{
    ch_thread_proxy_t * proxy = CH_NEW( ch_thread_proxy_t );
    proxy->function = _function;
    proxy->ud = _ud;

    pthread_t id;
    int err = pthread_create( &id, CH_NULLPTR, &__ch_thread_proxy, proxy );

    if( err != 0 )
    {
        CH_LOG_MESSAGE_ERROR( "thread", "invalid create pthread errno: %d"
            , errno
        );

        return CH_FAILURE;
    }
    
    ch_thread_handle_t * handle = CH_NEW( ch_thread_handle_t );

    handle->id = id;
    handle->proxy = proxy;

    *_handle = handle;

    return CH_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void ch_thread_join( ch_thread_handle_t * _handle )
{
    pthread_t id = _handle->id;

    pthread_join( id, CH_NULLPTR );
}
//////////////////////////////////////////////////////////////////////////
void ch_thread_destroy( ch_thread_handle_t * _handle )
{
    pthread_t id = _handle->id;

    pthread_cancel( id );
    pthread_join( id, CH_NULLPTR );

    CH_DELETE( _handle->proxy );
    CH_DELETE( _handle );
}
//////////////////////////////////////////////////////////////////////////