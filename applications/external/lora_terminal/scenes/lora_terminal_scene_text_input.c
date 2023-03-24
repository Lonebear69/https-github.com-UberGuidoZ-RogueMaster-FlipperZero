#include "applications/external/lora_terminal/lora_terminal_app_i.h"

void lora_terminal_scene_text_input_callback(void* context) {
    lora_terminalApp* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, lora_terminalEventStartConsole);
}

void lora_terminal_scene_text_input_on_enter(void* context) {
    lora_terminalApp* app = context;

    if(false == app->is_custom_tx_string) {
        // Fill text input with selected string so that user can add to it
        size_t length = strlen(app->selected_tx_string);
        furi_assert(length < lora_terminal_TEXT_INPUT_STORE_SIZE);
        bzero(app->text_input_store, lora_terminal_TEXT_INPUT_STORE_SIZE);
        strncpy(app->text_input_store, app->selected_tx_string, length);

        // Add space - because flipper keyboard currently doesn't have a space
        //app->text_input_store[length] = ' ';
        app->text_input_store[length + 1] = '\0';
        app->is_custom_tx_string = true;
    }

    // Setup view
    lora_textInput* text_input = app->text_input;
    // Add help message to header
    lora_text_input_set_header_text(text_input, "Send command to UART");
    lora_text_input_set_result_callback(
        text_input,
        lora_terminal_scene_text_input_callback,
        app,
        app->text_input_store,
        lora_terminal_TEXT_INPUT_STORE_SIZE,
        false);

    view_dispatcher_switch_to_view(app->view_dispatcher, lora_terminalAppViewTextInput);
}

bool lora_terminal_scene_text_input_on_event(void* context, SceneManagerEvent event) {
    lora_terminalApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == lora_terminalEventStartConsole) {
            // Point to custom string to send
            app->selected_tx_string = app->text_input_store;
            scene_manager_next_scene(app->scene_manager, lora_terminalAppViewConsoleOutput);
            consumed = true;
        }
    }

    return consumed;
}

void lora_terminal_scene_text_input_on_exit(void* context) {
    lora_terminalApp* app = context;

    lora_text_input_reset(app->text_input);
}
