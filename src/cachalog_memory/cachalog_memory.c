#include "cachalog_memory.h"

//////////////////////////////////////////////////////////////////////////
static ch_memory_alloc_t g_alloc = CH_NULLPTR;
static ch_memory_realloc_t g_realloc = CH_NULLPTR;
static ch_memory_free_t g_free = CH_NULLPTR;
static void * g_ud = CH_NULLPTR;
//////////////////////////////////////////////////////////////////////////
void ch_memory_initialize( ch_memory_alloc_t _alloc, ch_memory_realloc_t _realloc, ch_memory_free_t _free, void * _ud )
{
    g_alloc = _alloc;
    g_realloc = _realloc;
    g_free = _free;
    g_ud = _ud;
}
//////////////////////////////////////////////////////////////////////////
void * ch_memory_alloc( ch_size_t _size )
{
    void * ptr = g_alloc( _size, g_ud );

    return ptr;
}
//////////////////////////////////////////////////////////////////////////
void * ch_memory_realloc( void * _ptr, ch_size_t _size )
{
    void * ptr = g_realloc( _ptr, _size, g_ud );

    return ptr;
}
//////////////////////////////////////////////////////////////////////////
void ch_memory_free( const void * _ptr )
{
    g_free( _ptr, g_ud );
}
//////////////////////////////////////////////////////////////////////////