#ifndef __tish_h__
#define __tish_h__

#include <xuart.h>

typedef struct
{
    const uart_spec_t *uart_spec;

    void *recv_buf;
    void *ovfw_buf;
    void *pipe_buf;
    bool ovfw_assigned;
    bool ready;
    
    k_pipe_t pipe;
} tish_t;

int tish_init(tish_t *spec, const uart_spec_t *uart_spec_t/*, tish_line_callback_t */);
typedef void (*tish_line_callback_t)(char *line);

#endif