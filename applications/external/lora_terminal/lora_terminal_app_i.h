#pragma once

#include "lora_terminal_app.h"
#include "applications/external/lora_terminal/scenes/lora_terminal_scene.h"
#include "lora_terminal_custom_event.h"
#include "lora_terminal_uart.h"

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/text_box.h>
#include <gui/modules/variable_item_list.h>
#include "lora_text_input.h"

#define NUM_MENU_ITEMS (4)

#define lora_terminal_TEXT_BOX_STORE_SIZE (4096)
#define lora_terminal_TEXT_INPUT_STORE_SIZE (512)
#define UART_CH (FuriHalUartIdUSART1)

struct lora_terminalApp {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;

    char text_input_store[lora_terminal_TEXT_INPUT_STORE_SIZE + 1];
    FuriString* text_box_store;
    size_t text_box_store_strlen;
    TextBox* text_box;
    lora_textInput* text_input;

    VariableItemList* var_item_list;

    lora_terminalUart* uart;
    int selected_menu_index;
    int selected_option_index[NUM_MENU_ITEMS];
    const char* selected_tx_string;
    bool is_command;
    bool is_custom_tx_string;
    bool focus_console_start;
    bool show_stopscan_tip;
    int BAUDRATE;
};

typedef enum {
    lora_terminalAppViewVarItemList,
    lora_terminalAppViewConsoleOutput,
    lora_terminalAppViewTextInput,
} lora_terminalAppView;
