#include "cachalog_rand.h"

static inline uint64_t ror64( uint64_t v, uint64_t r )
{
    return (v >> r) | (v << (64 - r));
}

uint64_t ch_rand64( uint64_t * const _seed )
{
    const uint64_t prime = 0x9FB21C651E98DF25L;

    uint64_t v = *_seed;

    v ^= ror64( v, 49 ) ^ ror64( v, 24 );
    v *= prime;
    v ^= v >> 28;
    v *= prime;
    v ^= v >> 28;

    *_seed = v;

    return v;
}