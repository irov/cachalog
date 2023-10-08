#ifndef CACHALOG_RING_H_
#define CACHALOG_RING_H_

#include "cachalog_config/cachalog_config.h"

#define CH_RING_DECLARE( TYPE )\
    struct TYPE * next;\
    struct TYPE * prev

#define CH_RING_INIT( HEAD )\
    (HEAD)->next = (HEAD);\
    (HEAD)->prev = (HEAD);
    
#define CH_RING_IS_EMPTY( HEAD )\
    ((HEAD)->next == (HEAD))

#define CH_RING_PUSH_BACK( HEAD, NODE )\
    (NODE)->next = (HEAD);\
    (NODE)->prev = (HEAD)->prev;\
    (HEAD)->prev->next = (NODE);\
    (HEAD)->prev = (NODE);

#define CH_RING_PUSH_FRONT( HEAD, NODE )\
    (NODE)->next = (HEAD)->next;\
    (NODE)->prev = (HEAD);\
    (HEAD)->next->prev = (NODE);\
    (HEAD)->next = (NODE);

#define CH_RING_POP_BACK( HEAD )\
    (HEAD)->prev->prev->next = (HEAD);\
    (HEAD)->prev = (HEAD)->prev->prev;

#define CH_RING_POP_FRONT( HEAD )\
    (HEAD)->next->next->prev = (HEAD);\
    (HEAD)->next = (HEAD)->next->next;

#define CH_RING_GET_BACK( HEAD )\
    (HEAD)->prev

#define CH_RING_GET_FRONT( HEAD )\
    (HEAD)->next

#define CH_RING_GET_NEXT( NODE )\
    (NODE)->next

#define CH_RING_GET_PREV( NODE )\
    (NODE)->prev

#define CH_RING_REMOVE( NODE )\
    (NODE)->prev->next = (NODE)->next;\
    (NODE)->next->prev = (NODE)->prev;

#endif