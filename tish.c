#include <tish.h>
#include <xlog.h>

int tish_start_receive(tish_t *spec)
{
    return uart_rx_enable(spec->uart_spec->uart, spec->recv_buf, spec->uart_spec->buffer_length, spec->uart_spec->receive_timeout);
}

void tish_irq(const struct device *dev, struct uart_event *evt, void *data)
{
    tish_t *spec = data;

    if (dev != spec->uart_spec->uart)
        return;

    switch (evt->type)
    {
    case UART_RX_RDY:
        struct uart_event_rx *rx = &evt->data.rx;
        if (rx->len)
        {
            size_t written;
            SURE(k_pipe_put, &spec->pipe, rx->buf + rx->offset, rx->len, &written, rx->len, K_NO_WAIT);
        }
        break;

    case UART_RX_BUF_REQUEST:
        __ASSERT(spec->ovfw_assigned == false, "UART_RX_BUF_REQUEST, ovfw_buf already assigned");

        struct uart_event_rx_buf *rx_buf = &evt->data.rx_buf;
        rx_buf->buf = spec->ovfw_buf;
        spec->ovfw_assigned = 1;
        break;

    case UART_RX_BUF_RELEASED:
        spec->ovfw_assigned = 0;
        break;

    case UART_RX_DISABLED:
        SURE(tish_start_receive, spec);
        break;

    case UART_TX_DONE:
        break;

    case UART_TX_ABORTED:
        break;

    case UART_RX_STOPPED:
        break;

    default:
        break;
    }
}

int tish_init(tish_t *spec, const uart_spec_t *uart_spec)
{
    spec->uart_spec = uart_spec;
    SURE(uart_callback_set, spec->uart_spec->uart, tish_irq, spec);

    spec->recv_buf = k_malloc(spec->uart_spec->buffer_length);
    spec->ovfw_buf = k_malloc(spec->uart_spec->buffer_length);
    spec->pipe_buf = k_malloc(spec->uart_spec->buffer_length);
    k_pipe_init(&spec->pipe, spec->pipe_buf, spec->uart_spec->buffer_length);
    spec->ovfw_assigned = false;
    spec->ready = true;
    
    return tish_start_receive(spec);
}