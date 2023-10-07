#ifndef CACHALOG_MESSAGE_H_
#define CACHALOG_MESSAGE_H_

#include "cachalog_config/cachalog_config.h"

typedef struct ch_message_t
{
    struct ch_message_t * next;

    char text[4096];
} ch_message_t;

#endif