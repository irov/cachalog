#ifndef CACHALOG_FILE_H_
#define CACHALOG_FILE_H_

#include "cachalog_config/cachalog_config.h"

ch_result_t ch_file_read( const char * _path, void * _buffer, ch_size_t _capacity, ch_size_t * const _size );
ch_result_t ch_file_read_text( const char * _path, char * _buffer, ch_size_t _capacity, ch_size_t * const _size );

#endif