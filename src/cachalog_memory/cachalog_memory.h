#ifndef CACHALOG_MEMORY_H_
#define CACHALOG_MEMORY_H_

#include "cachalog_config/cachalog_config.h"

typedef void * (*ch_memory_alloc_t)(ch_size_t _size, void * _ud);
typedef void * (*ch_memory_realloc_t)(void * _ptr, ch_size_t _size, void * _ud);
typedef void(*ch_memory_free_t)(const void * _ptr, void * _ud);

void ch_memory_initialize( ch_memory_alloc_t _alloc, ch_memory_realloc_t _realloc, ch_memory_free_t _free, void * _ud );

void * ch_memory_alloc( ch_size_t _size );
void * ch_memory_realloc( void * _ptr, ch_size_t _size );
void ch_memory_free( const void * _ptr );

#ifndef CH_ALLOC
#define CH_ALLOC(S) ch_memory_alloc((S))
#endif

#ifndef CH_REALLOC
#define CH_REALLOC(P, S) ch_memory_realloc((P), (S))
#endif

#ifndef CH_FREE
#define CH_FREE(P) ch_memory_free((P))
#endif

#ifndef CH_NEW
#define CH_NEW(TYPE) ((TYPE*)ch_memory_alloc(sizeof(TYPE)))
#endif

#ifndef CH_NEWN
#define CH_NEWN(TYPE, N) ((TYPE*)ch_memory_alloc(sizeof(TYPE) * N))
#endif

#ifndef CH_DELETE
#define CH_DELETE(P) ch_memory_free((P))
#endif

#ifndef CH_DELETEN
#define CH_DELETEN(P) ch_memory_free((P))
#endif

#endif
