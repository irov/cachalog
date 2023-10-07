#ifndef CACHALOG_LOG_FILE_H_
#define CACHALOG_LOG_FILE_H_

#include "cachalog_log/cachalog_log.h"

ch_result_t ch_log_file_initialize( const char * _path );
void ch_log_file_finalize();

#endif
