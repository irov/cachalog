#ifndef CACHALOG_THREAD_H_
#define CACHALOG_THREAD_H_

#include "cachalog_config/cachalog_config.h"

typedef struct ch_thread_handle_t ch_thread_handle_t;

typedef void(*ch_thread_function_t)(void * _ud);

ch_result_t ch_thread_create( ch_thread_function_t _function, void * _ud, ch_thread_handle_t ** _handle );
void ch_thread_join( ch_thread_handle_t * _handle );
void ch_thread_destroy( ch_thread_handle_t * _handle );

#endif
