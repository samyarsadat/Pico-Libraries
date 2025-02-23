#include <stdio.h>
#include "pico/stdlib.h"

#include <uxr/client/profile/transport/custom/custom_transport.h>


bool pico_serial_transport_open(struct uxrCustomTransport *transport)
{
    // Ensure that stdio_init_all is only called once on the runtime
    // We initialize STDIO in our own program anyway, so this is not needed.
    /*static bool require_init = true;
    
    if(require_init)
    {
        stdio_init_all();
        require_init = false;
    }*/

    // Don't check for USB connection, as some programs might be using
    // standard UART pins and not UART over USB. In such cases, this
    // function would return false and microros wouldn't proceed.
    //return stdio_usb_connected();
    return true;
}

bool pico_serial_transport_close(struct uxrCustomTransport *transport)
{
    return true;
}

size_t pico_serial_transport_write(struct uxrCustomTransport *transport, uint8_t *buf, size_t len, uint8_t *errcode)
{
    for (size_t i = 0; i < len; i++)
    {
        if (buf[i] != putchar(buf[i]))
        {
            *errcode = 1;
            return i;
        }
    }

    return len;
}

size_t pico_serial_transport_read(struct uxrCustomTransport *transport, uint8_t *buf, size_t len, int timeout, uint8_t *errcode)
{
    uint64_t start_time_us = time_us_64();
    
    for (size_t i = 0; i < len; i++)
    {
        int64_t elapsed_time_us = timeout * 1000 - (time_us_64() - start_time_us);
        
        if (elapsed_time_us < 0)
        {
            *errcode = 1;
            return i;
        }

        int character = getchar_timeout_us(elapsed_time_us);
        
        if (character == PICO_ERROR_TIMEOUT)
        {
            *errcode = 1;
            return i;
        }
        
        buf[i] = character;
    }
    
    return len;
}