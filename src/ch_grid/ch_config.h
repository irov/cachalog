#ifndef CACHALOT_CONFIG_H_
#define CACHALOT_CONFIG_H_

#include "hb_config/hb_config.h"

#include <stdint.h>
#include <stddef.h>

#define CH_VERSION_MAJOR 0
#define CH_VERSION_MINOR 1
#define CH_VERSION_PATCH 0

typedef enum ch_error_code_e
{
    CH_ERROR_OK,
    CH_ERROR_INTERNAL,
    CH_ERROR_ALREADY_EXIST,
    CH_ERROR_NOT_FOUND,
    CH_ERROR_BAD_ARGUMENTS,
} ch_error_code_e;

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

#endif