#include "uros_freertos_abstract_lib/internal/pico_uart_transport.h"
#include <uxr/client/profile/transport/custom/custom_transport.h>
#include "pico/stdlib.h"
#include "pico/stdio.h"
#include <stdio.h>


bool pico_serial_transport_open(struct uxrCustomTransport* transport) {
    (void) transport;

    #ifndef UROS_TRANSPORT_NO_USB_CHECK
    return stdio_usb_connected();
    #else
    return true;
    #endif
}

bool pico_serial_transport_close(struct uxrCustomTransport* transport) {
    (void) transport;
    return true;
}

size_t pico_serial_transport_write(struct uxrCustomTransport* transport, const uint8_t* buf, size_t len, uint8_t* errcode) {
    (void) transport; (void) errcode;
    return stdio_put_string((const char*) buf, len, false, false);
}

size_t pico_serial_transport_read(struct uxrCustomTransport* transport, uint8_t* buf, size_t len, int timeout, uint8_t* errcode) {
    (void) transport;
    uint64_t start_time_us = time_us_64();
    
    for (size_t i = 0; i < len; i++) {
        int64_t elapsed_time_us = timeout * 1000 - (time_us_64() - start_time_us);
        
        if (elapsed_time_us < 0) {
            *errcode = 1;
            return i;
        }

        int character = stdio_getchar_timeout_us(elapsed_time_us);
        
        if (character == PICO_ERROR_TIMEOUT) {
            *errcode = 1;
            return i;
        }
        
        buf[i] = character;
    }
    
    return len;
}