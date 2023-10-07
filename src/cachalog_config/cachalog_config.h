#ifndef CACHALOG_CONFIG_H_
#define CACHALOG_CONFIG_H_

#include <stdint.h>
#include <stddef.h>

#define CH_VERSION_MAJOR 0
#define CH_VERSION_MINOR 1
#define CH_VERSION_PATCH 0

#ifndef NDEBUG
#define CH_DEBUG
#endif

typedef enum ch_bool_e
{
    CH_FALSE,
    CH_TRUE,
} ch_bool_e;

typedef uint32_t ch_bool_t;
typedef size_t ch_size_t;

static const ch_size_t CH_UNKNOWN_STRING_SIZE = (~0U);

typedef enum ch_result_e
{
    CH_SUCCESSFUL,
    CH_FAILURE,
} ch_result_e;

typedef enum ch_error_code_e
{
    CH_ERROR_OK,
    CH_ERROR_INTERNAL,
    CH_ERROR_ALREADY_EXIST,
    CH_ERROR_NOT_FOUND,
    CH_ERROR_BAD_ARGUMENTS,
} ch_error_code_e;

typedef ch_result_e ch_result_t;
typedef ch_error_code_e ch_error_code_t;

typedef enum ch_http_code_e
{
    CH_HTTP_OK = 200,
    CH_HTTP_NOCONTENT = 204,
    CH_HTTP_MOVEPERM = 301,
    CH_HTTP_MOVETEMP = 302,
    CH_HTTP_NOTMODIFIED = 304,
    CH_HTTP_BADREQUEST = 400,
    CH_HTTP_NOTFOUND = 404,
    CH_HTTP_BADMETHOD = 405,
    CH_HTTP_ENTITYTOOLARGE = 413,
    CH_HTTP_EXPECTATIONFAILED = 417,
    CH_HTTP_INTERNAL = 500,
    CH_HTTP_NOTIMPLEMENTED = 501,
    CH_HTTP_SERVUNAVAIL = 503,
} ch_http_code_e;

typedef ch_http_code_e ch_http_code_t;

typedef uint8_t ch_byte_t;

typedef uint64_t ch_time_t;

typedef struct ch_sha1_t
{
    ch_byte_t value[20];
} ch_sha1_t;

#ifndef CH_DATA_MAX_SIZE
#define CH_DATA_MAX_SIZE 10240
#endif

typedef char ch_source_t[CH_DATA_MAX_SIZE];
typedef ch_byte_t ch_data_t[CH_DATA_MAX_SIZE];

#ifndef CH_UNUSED
#define CH_UNUSED(X) (void)(X)
#endif

#ifndef CH_NULLPTR
#define CH_NULLPTR ((void*)0)
#endif

#define CH_MEMOFFSET(P, I) ((void *)((uint8_t *)(P) + I))
#define CH_TMEMOFFSET(T, P, I) ((T)((uint8_t *)(P) + I))

#ifndef CH_MAX_PATH
#define CH_MAX_PATH 260
#endif

#ifndef CH_MAX_URI
#define CH_MAX_URI 2048
#endif

#if defined(WIN32)
#   define CH_PLATFORM_WINDOWS
#elif defined(__linux__) && !defined(__ANDROID__)
#   define CH_PLATFORM_LINUX
#elif defined(__APPLE__)
#   include "TargetConditionals.h"
#   if TARGET_OS_OSX
#       define CH_PLATFORM_OSX
#   else
#       error "unsuport apple platform"
#   endif
#else
#   error "undefine platform"
#endif

#ifndef CH_CODE_FILE
#define CH_CODE_FILE __FILE__
#endif

#ifndef CH_CODE_LINE
#define CH_CODE_LINE __LINE__
#endif

#endif