#ifndef CACHALOG_MATH_H_
#define CACHALOG_MATH_H_

#include "cachalog_config/cachalog_config.h"

uint32_t ch_nearest_pow2( uint32_t _value );
uint32_t ch_log2( uint32_t _value );
uint32_t ch_pow2( uint32_t _exponent );
uint32_t ch_clamp( uint32_t _l, uint32_t _r, uint32_t _value );
ch_size_t ch_clampz( ch_size_t _l, ch_size_t _r, ch_size_t _value );

#endif
