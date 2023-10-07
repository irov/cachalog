#ifndef CACHALOG_MUTEX_H_
#define CACHALOG_MUTEX_H_

#include "cachalog_config/cachalog_config.h"

typedef struct ch_mutex_handle_t ch_mutex_handle_t;

ch_result_t ch_mutex_create( ch_mutex_handle_t ** _handle );
void ch_mutex_destroy( ch_mutex_handle_t * _handle );

ch_bool_t ch_mutex_try_lock( ch_mutex_handle_t * _handle );
void ch_mutex_lock( ch_mutex_handle_t * _handle );
void ch_mutex_unlock( ch_mutex_handle_t * _handle );

#endif
