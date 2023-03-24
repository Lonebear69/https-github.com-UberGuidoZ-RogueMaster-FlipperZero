#include "lora_terminal_app_i.h"

#include <furi.h>
#include <furi_hal.h>

static bool lora_terminal_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    lora_terminalApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool lora_terminal_app_back_event_callback(void* context) {
    furi_assert(context);
    lora_terminalApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void lora_terminal_app_tick_event_callback(void* context) {
    furi_assert(context);
    lora_terminalApp* app = context;
    scene_manager_handle_tick_event(app->scene_manager);
}

lora_terminalApp* lora_terminal_app_alloc() {
    lora_terminalApp* app = malloc(sizeof(lora_terminalApp));

    app->gui = furi_record_open(RECORD_GUI);

    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&lora_terminal_scene_handlers, app);
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, lora_terminal_app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, lora_terminal_app_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        app->view_dispatcher, lora_terminal_app_tick_event_callback, 100);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    app->var_item_list = variable_item_list_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        lora_terminalAppViewVarItemList,
        variable_item_list_get_view(app->var_item_list));

    for(int i = 0; i < NUM_MENU_ITEMS; ++i) {
        app->selected_option_index[i] = 0;
    }

    app->text_box = text_box_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, lora_terminalAppViewConsoleOutput, text_box_get_view(app->text_box));
    app->text_box_store = furi_string_alloc();
    furi_string_reserve(app->text_box_store, lora_terminal_TEXT_BOX_STORE_SIZE);

    app->text_input = lora_text_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        lora_terminalAppViewTextInput,
        lora_text_input_get_view(app->text_input));

    scene_manager_next_scene(app->scene_manager, lora_terminalSceneStart);

    return app;
}

void lora_terminal_app_free(lora_terminalApp* app) {
    furi_assert(app);

    // Views
    view_dispatcher_remove_view(app->view_dispatcher, lora_terminalAppViewVarItemList);
    view_dispatcher_remove_view(app->view_dispatcher, lora_terminalAppViewConsoleOutput);
    view_dispatcher_remove_view(app->view_dispatcher, lora_terminalAppViewTextInput);
    text_box_free(app->text_box);
    furi_string_free(app->text_box_store);
    lora_text_input_free(app->text_input);

    // View dispatcher
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    lora_terminal_uart_free(app->uart);

    // Close records
    furi_record_close(RECORD_GUI);

    free(app);
}

int32_t lora_terminal_app(void* p) {
    UNUSED(p);
    lora_terminalApp* lora_terminal_app = lora_terminal_app_alloc();

    lora_terminal_app->uart = lora_terminal_uart_init(lora_terminal_app);

    view_dispatcher_run(lora_terminal_app->view_dispatcher);

    lora_terminal_app_free(lora_terminal_app);

    return 0;
}
