#pragma once

#include "furi_hal.h"

#define RX_BUF_SIZE (320)

typedef struct lora_terminalUart lora_terminalUart;

void lora_terminal_uart_set_handle_rx_data_cb(
    lora_terminalUart* uart,
    void (*handle_rx_data_cb)(uint8_t* buf, size_t len, void* context));
void lora_terminal_uart_tx(uint8_t* data, size_t len);
lora_terminalUart* lora_terminal_uart_init(lora_terminalApp* app);
void lora_terminal_uart_free(lora_terminalUart* uart);
