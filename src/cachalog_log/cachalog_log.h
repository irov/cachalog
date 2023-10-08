#ifndef CACHALOG_LOG_H_
#define CACHALOG_LOG_H_

#include "cachalog_config/cachalog_config.h"

typedef enum ch_log_level_e
{
    CH_LOG_VERBOSE,
    CH_LOG_INFO,
    CH_LOG_WARNING,
    CH_LOG_ERROR,
    CH_LOG_CRITICAL,
} ch_log_level_e;

typedef ch_log_level_e ch_log_level_t;

ch_result_t ch_log_initialize();
void ch_log_finalize();

const char * ch_log_get_level_string( ch_log_level_t _level );

typedef void(*ch_log_observer_t)(const char * _category, ch_log_level_t _level, const char * _file, uint32_t _line, const char * _message, void * _ud);

ch_result_t ch_log_add_observer( const char * _category, ch_log_level_t _level, ch_log_observer_t _observer, void * _ud );
ch_result_t ch_log_remove_observer( ch_log_observer_t _observer, void ** _ud );

void ch_log_message( const char * _category, ch_log_level_t _level, const char * _file, uint32_t _line, const char * _format, ... );

#define CH_LOG_MESSAGE_INFO(category, ...)\
    ch_log_message(category, CH_LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)

#define CH_LOG_MESSAGE_WARNING(category, ...)\
    ch_log_message(category, CH_LOG_WARNING, __FILE__, __LINE__, __VA_ARGS__)

#define CH_LOG_MESSAGE_ERROR(category, ...)\
    ch_log_message(category, CH_LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)

#define CH_LOG_MESSAGE_CRITICAL(category, ...)\
    ch_log_message(category, CH_LOG_CRITICAL, __FILE__, __LINE__, __VA_ARGS__)

#endif
