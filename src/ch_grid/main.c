#include "ch_config.h"

#include "ch_service.h"

#include "hb_thread/hb_thread.h"
#include "hb_mutex/hb_mutex.h"
#include "hb_memory/hb_memory.h"

#include "hb_log/hb_log.h"
#include "hb_log_stderr/hb_log_stderr.h"
#include "hb_log_file/hb_log_file.h"

#include "hb_json/hb_json.h"
#include "hb_http/hb_http.h"

#include "hb_utils/hb_getopt.h"
#include "hb_utils/hb_getenv.h"
#include "hb_utils/hb_clock.h"
#include "hb_utils/hb_date.h"
#include "hb_utils/hb_file.h"
#include "hb_utils/hb_time.h"

#include "evhttp.h"
#include "event2/thread.h"

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
#ifndef CH_GRID_REQUEST_DATA_MAX_SIZE
#define CH_GRID_REQUEST_DATA_MAX_SIZE 102400
#endif
//////////////////////////////////////////////////////////////////////////
#ifndef CH_GRID_RESPONSE_DATA_MAX_SIZE
#define CH_GRID_RESPONSE_DATA_MAX_SIZE 1048576
#endif
//////////////////////////////////////////////////////////////////////////
typedef struct ch_grid_config_t
{
    char name[32 + 1];
    char token[32 + 1];

    char grid_uri[HB_MAX_URI];
    uint16_t grid_port;

    uint32_t max_thread;
    uint32_t max_record;
    uint64_t max_time;

    char log_file[HB_MAX_PATH];
} ch_grid_config_t;
//////////////////////////////////////////////////////////////////////////
typedef struct ch_grid_process_handle_t
{
    evutil_socket_t * ev_socket;
    hb_mutex_handle_t * mutex_ev_socket;

    const ch_grid_config_t * config;

    hb_thread_handle_t * thread;
    ch_service_t * service;

    char request_data[CH_GRID_REQUEST_DATA_MAX_SIZE];
    char response_data[CH_GRID_RESPONSE_DATA_MAX_SIZE];
} ch_grid_process_handle_t;
//////////////////////////////////////////////////////////////////////////
typedef ch_http_code_t( *ch_request_func_t )(ch_service_t * _service, const char * _project, const hb_json_handle_t * _json, char * _response, hb_size_t _capacity, hb_size_t * const _size, char * const _reason);
//////////////////////////////////////////////////////////////////////////
typedef struct ch_grid_cmd_inittab_t
{
    const char * name;
    ch_request_func_t request;
} ch_grid_cmd_inittab_t;
//////////////////////////////////////////////////////////////////////////
extern ch_http_code_t ch_grid_request_insert( ch_service_t * _service, const char * _project, const hb_json_handle_t * _json, char * _response, hb_size_t _capacity, hb_size_t * const _size, char * const _reason );
extern ch_http_code_t ch_grid_request_select( ch_service_t * _service, const char * _project, const hb_json_handle_t * _json, char * _response, hb_size_t _capacity, hb_size_t * const _size, char * const _reason );
//////////////////////////////////////////////////////////////////////////
static ch_grid_cmd_inittab_t grid_cmds[] =
{
    {"insert", &ch_grid_request_insert},
    {"select", &ch_grid_request_select},
};
//////////////////////////////////////////////////////////////////////////
static void __ch_grid_request( struct evhttp_request * _request, void * _ud )
{
    const char * host = evhttp_request_get_host( _request );
    HB_UNUSED( host );

    const char * uri = evhttp_request_get_uri( _request );

    struct evbuffer * output_buffer = evhttp_request_get_output_buffer( _request );

    if( output_buffer == HB_NULLPTR )
    {
        return;
    }

    struct evkeyvalq * output_headers = evhttp_request_get_output_headers( _request );

    if( output_headers == HB_NULLPTR )
    {
        return;
    }

    evhttp_add_header( output_headers, "Access-Control-Allow-Origin", "*" );
    evhttp_add_header( output_headers, "Access-Control-Allow-Headers", "*" );
    evhttp_add_header( output_headers, "Access-Control-Allow-Methods", "GET,POST,OPTIONS" );
    evhttp_add_header( output_headers, "Content-Type", "application/json" );

    enum evhttp_cmd_type command_type = evhttp_request_get_command( _request );

    if( command_type == EVHTTP_REQ_OPTIONS )
    {
        evhttp_send_reply( _request, HTTP_OK, "", output_buffer );

        return;
    }

    if( hb_http_is_request_json( _request ) == HB_FALSE )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "request must be json", output_buffer );

        return;
    }

    ch_grid_process_handle_t * process = (ch_grid_process_handle_t *)_ud;

    char response_reason[CH_GRID_REASON_MAX_SIZE] = {'\0'};
    ch_http_code_t response_code = CH_HTTP_OK;
    hb_size_t response_data_size = 0;

    HB_LOG_MESSAGE_INFO( "grid", "request uri: %s"
        , uri
    );

    char token[CH_TOKEN_SIZE + 1 + 1] = {'\0'};
    char project[CH_PROJECT_MAXLEN + 1 + 1] = {'\0'};
    char cmd_name[CH_CMD_MAXLEN + 1 + 1] = {'\0'};
    int32_t count = sscanf( uri, "/%" HB_PP_STRINGIZE( CH_TOKEN_SIZE ) "[^'/']/%" HB_PP_STRINGIZE( CH_PROJECT_MAXLEN ) "[^'/']/%" HB_PP_STRINGIZE( CH_CMD_MAXLEN ) "[^ '/']", token, project, cmd_name );

    if( count != 3 )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "incorrect url", output_buffer );

        return;
    }

    const ch_grid_config_t * config = process->config;

    if( strlen( token ) != CH_TOKEN_SIZE || strcmp( config->token, token ) != 0 )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "bad token", output_buffer );

        return;
    }

    if( strlen( project ) < CH_PROJECT_MINLEN || strlen( project ) > CH_PROJECT_MAXLEN )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "bad project", output_buffer );

        return;
    }

    if( strlen( cmd_name ) < 1 || strlen( cmd_name ) > CH_CMD_MAXLEN )
    {
        evhttp_send_reply( _request, HTTP_BADREQUEST, "bad cmd", output_buffer );

        return;
    }

    HB_LOG_MESSAGE_INFO( "grid", "request project: %s cmd: %s"
        , project
        , cmd_name
    );

    ch_service_t * service = process->service;

    hb_bool_t cmd_found = HB_FALSE;

    for( ch_grid_cmd_inittab_t
        * cmd_inittab = grid_cmds,
        *cmd_inittab_end = grid_cmds + sizeof( grid_cmds ) / sizeof( grid_cmds[0] );
        cmd_inittab != cmd_inittab_end;
        ++cmd_inittab )
    {
        if( strcmp( cmd_inittab->name, cmd_name ) != 0 )
        {
            continue;
        }

        hb_json_handle_t * json_handle;
        if( hb_http_get_request_json( _request, process->request_data, sizeof( process->request_data ), &json_handle ) == HB_FAILURE )
        {
            evhttp_send_reply( _request, HTTP_BADREQUEST, "invalid get request json", output_buffer );

            return;
        }

        response_code = (*cmd_inittab->request)(service, project, json_handle, process->response_data, sizeof( process->response_data ), &response_data_size, response_reason);

        hb_json_free( json_handle );

        cmd_found = HB_TRUE;

        break;
    }

    if( cmd_found == HB_FALSE )
    {
        snprintf( response_reason, CH_GRID_REASON_MAX_SIZE, "invalid found cmd '%s'", cmd_name );

        evhttp_send_reply( _request, HTTP_BADMETHOD, response_reason, output_buffer );

        return;
    }

    HB_LOG_MESSAGE_INFO( "grid", "response code: %u reason: %s data: %.*s"
        , response_code
        , response_reason
        , response_data_size
        , process->response_data
    );

    if( evbuffer_add( output_buffer, process->response_data, response_data_size ) != 0 )
    {
        evhttp_send_reply( _request, HTTP_INTERNAL, "invalid add response to buffer", output_buffer );

        return;
    }

    char response_data_size_str[16] = {'\0'};
    snprintf( response_data_size_str, sizeof( response_data_size_str ), "%zu", response_data_size );

    evhttp_add_header( output_headers, "Content-Length", response_data_size_str );

    evhttp_send_reply( _request, response_code, response_reason, output_buffer );
}
//////////////////////////////////////////////////////////////////////////
static void __ch_ev_accept_socket( ch_grid_process_handle_t * _handle, struct evhttp * _server )
{
    if( *_handle->ev_socket == -1 )
    {
        const ch_grid_config_t * config = _handle->config;

        struct evhttp_bound_socket * bound_socket = evhttp_bind_socket_with_handle( _server, config->grid_uri, config->grid_port );

        if( bound_socket == HB_NULLPTR )
        {
            int e = errno;
            const char * err = strerror( errno );

            HB_LOG_MESSAGE_ERROR( "grid", "grid '%s' invalid bind socket '%s:%u' error: %s [%d]"
                , config->name
                , config->grid_uri
                , config->grid_port
                , err
                , e
            );

            return;
        }

        *_handle->ev_socket = evhttp_bound_socket_get_fd( bound_socket );
    }
    else
    {
        evhttp_accept_socket( _server, *_handle->ev_socket );
    }
}
//////////////////////////////////////////////////////////////////////////
static void __ch_ev_thread_base( void * _ud )
{
    HB_UNUSED( _ud );

    ch_grid_process_handle_t * handle = (ch_grid_process_handle_t *)_ud;

    struct event_base * base = event_base_new();
    HB_UNUSED( base );

    struct evhttp * http_server = evhttp_new( base );
    HB_UNUSED( http_server );

    evhttp_set_allowed_methods( http_server, EVHTTP_REQ_POST | EVHTTP_REQ_OPTIONS );

    evhttp_set_gencb( http_server, &__ch_grid_request, handle );

    hb_mutex_lock( handle->mutex_ev_socket );

    __ch_ev_accept_socket( handle, http_server );

    hb_mutex_unlock( handle->mutex_ev_socket );

    event_base_dispatch( base );

    evhttp_free( http_server );
}
//////////////////////////////////////////////////////////////////////////
static void * __ch_memory_alloc( hb_size_t _size, void * _ud )
{
    HB_UNUSED( _ud );

    void * ptr = malloc( _size );

    return ptr;
}
//////////////////////////////////////////////////////////////////////////
static void * __ch_memory_realloc( void * _ptr, hb_size_t _size, void * _ud )
{
    HB_UNUSED( _ud );

    void * ptr = realloc( _ptr, _size );

    return ptr;
}
//////////////////////////////////////////////////////////////////////////
static void __ch_memory_free( const void * _ptr, void * _ud )
{
    HB_UNUSED( _ud );

    free( (void *)_ptr );
}
//////////////////////////////////////////////////////////////////////////
static void __event_log( int _severity, const char * _msg )
{
    switch( _severity )
    {
    case EVENT_LOG_MSG:
        {
            hb_log_message( "libevent", HB_LOG_INFO, HB_NULLPTR, 0, "%s", _msg );
        }break;
    case EVENT_LOG_WARN:
        {
            hb_log_message( "libevent", HB_LOG_WARNING, HB_NULLPTR, 0, "%s", _msg );
        }break;
    case EVENT_LOG_ERR:
        {
            hb_log_message( "libevent", HB_LOG_ERROR, HB_NULLPTR, 0, "%s", _msg );
        }break;
    }
}
//////////////////////////////////////////////////////////////////////////
static void __event_fatal( int err )
{
    HB_LOG_MESSAGE_ERROR( "grid", "libevent fatal error: %d"
        , err
    );
}
//////////////////////////////////////////////////////////////////////////
static void __ch_grid_config_u16( int _argc, char * _argv[], const char * _env, const char * _key, uint16_t * const _value, uint16_t _default )
{
    int64_t arg = -1;
    hb_getopti( _argc, _argv, _key, &arg );

    if( arg != -1 )
    {
        *_value = (uint16_t)arg;
    }
    else
    {
        int64_t env = -1;
        if( hb_getenvi( _env, &env ) == HB_SUCCESSFUL )
        {
            *_value = (uint16_t)env;
        }
        else
        {
            *_value = _default;
        }
    }
}
//////////////////////////////////////////////////////////////////////////
static void __ch_grid_config_u32( int _argc, char * _argv[], const char * _env, const char * _key, uint32_t * const _value, uint32_t _default )
{
    int64_t arg = -1;
    hb_getopti( _argc, _argv, _key, &arg );

    if( arg != -1 )
    {
        *_value = (uint32_t)arg;
    }
    else
    {
        int64_t env = -1;
        if( hb_getenvi( _env, &env ) == HB_SUCCESSFUL )
        {
            *_value = (uint32_t)env;
        }
        else
        {
            *_value = _default;
        }
    }
}
//////////////////////////////////////////////////////////////////////////
static void __ch_grid_config_u64( int _argc, char * _argv[], const char * _env, const char * _key, uint64_t * const _value, uint64_t _default )
{
    int64_t arg = -1;
    hb_getopti( _argc, _argv, _key, &arg );

    if( arg != -1 )
    {
        *_value = (uint64_t)arg;
    }
    else
    {
        int64_t env = -1;
        if( hb_getenvi( _env, &env ) == HB_SUCCESSFUL )
        {
            *_value = (uint64_t)env;
        }
        else
        {
            *_value = _default;
        }
    }
}
//////////////////////////////////////////////////////////////////////////
static void __ch_grid_config_string( int _argc, char * _argv[], const char * _env, const char * _key, char * _value, hb_size_t _capacity, const char * _default )
{
    const char * arg = HB_NULLPTR;
    hb_getopt( _argc, _argv, _key, &arg );

    if( arg != HB_NULLPTR )
    {
        strncpy( _value, arg, _capacity );
    }
    else
    {
        if( hb_getenv( _env, _value, _capacity ) == HB_SUCCESSFUL )
        {
            //Empty;
        }
        else
        {
            strncpy( _value, _default, _capacity );
        }        
    }
}
//////////////////////////////////////////////////////////////////////////
int main( int _argc, char * _argv[] )
{
    HB_UNUSED( _argc );
    HB_UNUSED( _argv );

    hb_memory_initialize( &__ch_memory_alloc, &__ch_memory_realloc, &__ch_memory_free, HB_NULLPTR );

    hb_log_initialize();

    if( hb_log_stderr_initialize() == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

#ifdef HB_PLATFORM_WINDOWS
    const WORD wVersionRequested = MAKEWORD( 2, 2 );

    WSADATA wsaData;
    int32_t err = WSAStartup( wVersionRequested, &wsaData );

    if( err != 0 )
    {
        HB_LOG_MESSAGE_CRITICAL( "grid", "invalid initialize winsock" );

        return EXIT_FAILURE;
    }
#endif

    const char * config_file = HB_NULLPTR;
    hb_getopt( _argc, _argv, "--config", &config_file );

    ch_grid_config_t * config = HB_NEW( ch_grid_config_t );

    __ch_grid_config_u32( _argc, _argv, "CACHALOT__MAX_THREAD", "--max_thread", &config->max_thread, 16 );
    __ch_grid_config_u32( _argc, _argv, "CACHALOT__MAX_RECORD", "--max_record", &config->max_record, 10000 );
    __ch_grid_config_u64( _argc, _argv, "CACHALOT__MAX_TIME", "--max_time", &config->max_time, HB_TIME_SECONDS_IN_WEEK );
    __ch_grid_config_string( _argc, _argv, "CACHALOT__GRID_URL", "--grid_uri", config->grid_uri, sizeof( config->grid_uri ), "127.0.0.1" );
    __ch_grid_config_u16( _argc, _argv, "CACHALOT__GRID_PORT", "--grid_port", &config->grid_port, 5555 );
    __ch_grid_config_string( _argc, _argv, "CACHALOT__TOKEN", "--token", config->token, sizeof( config->token ), "" );
    __ch_grid_config_string( _argc, _argv, "CACHALOT__NAME", "--name", config->name, sizeof( config->name ), "hb" );

    char default_log_file[HB_MAX_PATH];

#ifndef HB_DEBUG
    strcpy( default_log_file, "" );
#else
    hb_date_t date;
    hb_date( &date );

    sprintf( default_log_file, "log_grid_%u_%u_%u_%u_%u_%u.log"
        , date.year
        , date.mon
        , date.mday
        , date.hour
        , date.min
        , date.sec );
#endif

    __ch_grid_config_string( _argc, _argv, "CACHALOT__LOG_FILE", "--log_file", config->log_file, sizeof( config->log_file ), default_log_file );

    if( config_file != HB_NULLPTR )
    {
        char config_buffer[HB_DATA_MAX_SIZE];
        hb_size_t config_size;
        if( hb_file_read_text( config_file, config_buffer, HB_DATA_MAX_SIZE, &config_size ) == HB_FAILURE )
        {
            HB_LOG_MESSAGE_CRITICAL( "grid", "config file '%s' invalid read"
                , config_file
            );

            return EXIT_FAILURE;
        }

        uint8_t config_pool[HB_DATA_MAX_SIZE];

        hb_json_handle_t * json_handle;
        if( hb_json_create( config_buffer, config_size, config_pool, HB_DATA_MAX_SIZE, &json_handle ) == HB_FAILURE )
        {
            HB_LOG_MESSAGE_CRITICAL( "grid", "config file '%s' wrong json"
                , config_file
            );

            return EXIT_FAILURE;
        }

        hb_json_copy_field_string( json_handle, "grid_uri", config->grid_uri, sizeof( config->grid_uri ) );
        hb_json_get_field_uint16( json_handle, "grid_port", &config->grid_port );

        hb_json_copy_field_string( json_handle, "token", config->token, sizeof( config->token ) );

        hb_json_get_field_uint32( json_handle, "max_thread", &config->max_thread );
        hb_json_get_field_uint32( json_handle, "max_record", &config->max_record );
        hb_json_get_field_uint64( json_handle, "max_time", &config->max_time );

        hb_json_copy_field_string( json_handle, "name", config->name, sizeof( config->name ) );
        hb_json_copy_field_string( json_handle, "log_file", config->log_file, sizeof( config->log_file ) );

        hb_json_free( json_handle );
    }

    if( strlen( config->token ) != CH_TOKEN_SIZE )
    {
        HB_LOG_MESSAGE_CRITICAL( "grid", "invalid token size '%s' [%d]"
            , config->token
            , CH_TOKEN_SIZE
        );

        return EXIT_FAILURE;
    }

    if( strcmp( config->log_file, "" ) != 0 )
    {
        if( hb_log_file_initialize( config->log_file ) == HB_FAILURE )
        {
            HB_LOG_MESSAGE_WARNING( "grid", "grid '%s' invalid initialize [log] file '%s'"
                , config->name
                , config->log_file
            );
        }
    }

    HB_LOG_MESSAGE_INFO( "grid", "start grid with config:" );
    HB_LOG_MESSAGE_INFO( "grid", "------------------------------------" );
    HB_LOG_MESSAGE_INFO( "grid", "name: %s", config->name );
    HB_LOG_MESSAGE_INFO( "grid", "token: %s", config->token );
    HB_LOG_MESSAGE_INFO( "grid", "grid_uri: %s", config->grid_uri );
    HB_LOG_MESSAGE_INFO( "grid", "grid_port: %u", config->grid_port );
    HB_LOG_MESSAGE_INFO( "grid", "max_thread: %u", config->max_thread );
    HB_LOG_MESSAGE_INFO( "grid", "max_record: %u", config->max_record );
    HB_LOG_MESSAGE_INFO( "grid", "max_time: %" PRIu64 "", config->max_time );
    HB_LOG_MESSAGE_INFO( "grid", "log_file: %s", config->log_file );
    HB_LOG_MESSAGE_INFO( "grid", "------------------------------------" );

    ch_service_t * service;
    if( ch_service_create( &service, config->max_record, config->max_time ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "grid", "grid '%s' invalid create service"
            , config->name
        );

        return EXIT_FAILURE;
    }

    event_enable_debug_logging( EVENT_DBG_ALL );
    event_set_log_callback( &__event_log );
    event_set_fatal_callback( &__event_fatal );

    hb_mutex_handle_t * mutex_ev_socket;
    if( hb_mutex_create( &mutex_ev_socket ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "grid", "grid '%s' invalid create mutex"
            , config->name
        );

        return EXIT_FAILURE;
    }

    ch_grid_process_handle_t * process_handles = HB_NEWN( ch_grid_process_handle_t, config->max_thread );

    evutil_socket_t ev_socket = -1;
    for( uint32_t i = 0; i != config->max_thread; ++i )
    {
        ch_grid_process_handle_t * process_handle = process_handles + i;

        process_handle->ev_socket = &ev_socket;
        process_handle->mutex_ev_socket = mutex_ev_socket;

        process_handle->config = config;
        process_handle->service = service;

        process_handle->thread = HB_NULLPTR;

        hb_thread_handle_t * thread;
        if( hb_thread_create( &__ch_ev_thread_base, process_handle, &thread ) == HB_FAILURE )
        {
            HB_LOG_MESSAGE_ERROR( "grid", "grid '%s' invalid create thread"
                , config->name
            );

            return EXIT_FAILURE;
        }

        process_handle->thread = thread;
    }

    HB_LOG_MESSAGE_INFO( "grid", "ready.." );

    HB_LOG_MESSAGE_INFO( "grid", "------------------------------------" );

    for( uint32_t i = 0; i != config->max_thread; ++i )
    {
        ch_grid_process_handle_t * process_handle = process_handles + i;

        if( process_handle->thread != HB_NULLPTR )
        {
            hb_thread_join( process_handle->thread );
        }
    }

    hb_mutex_destroy( mutex_ev_socket );
    mutex_ev_socket = HB_NULLPTR;

    for( uint32_t i = 0; i != config->max_thread; ++i )
    {
        ch_grid_process_handle_t * process_handle = process_handles + i;

        if( process_handle->thread != HB_NULLPTR )
        {
            hb_thread_destroy( process_handle->thread );
        }
    }

    HB_DELETEN( process_handles );

    HB_DELETE( config );

#ifdef HB_PLATFORM_WINDOWS
    WSACleanup();
#endif

    hb_log_stderr_finalize();

#ifdef HB_DEBUG
    hb_log_file_finalize();
#endif

    hb_log_finalize();

    return EXIT_SUCCESS;
}