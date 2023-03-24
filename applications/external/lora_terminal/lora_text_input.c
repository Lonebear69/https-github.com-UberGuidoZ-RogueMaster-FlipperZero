#include "lora_text_input.h"
#include <gui/elements.h>
#include "lora_terminal_icons.h"
#include <furi.h>

struct lora_textInput {
    View* view;
    FuriTimer* timer;
};

typedef struct {
    const char text;
    const uint8_t x;
    const uint8_t y;
} lora_textInputKey;

typedef struct {
    const char* header;
    char* text_buffer;
    size_t text_buffer_size;
    bool clear_default_text;

    lora_textInputCallback callback;
    void* callback_context;

    uint8_t selected_row;
    uint8_t selected_column;

    lora_textInputValidatorCallback validator_callback;
    void* validator_callback_context;
    FuriString* validator_text;
    bool valadator_message_visible;
} lora_textInputModel;

static const uint8_t keyboard_origin_x = 1;
static const uint8_t keyboard_origin_y = 29;
static const uint8_t keyboard_row_count = 4;

#define ENTER_KEY '\n'
#define BACKSPACE_KEY '\b'

static const lora_textInputKey keyboard_keys_row_1[] = {
    {'{', 1, 0},
    {'(', 9, 0},
    {'[', 17, 0},
    {'|', 25, 0},
    {'@', 33, 0},
    {'&', 41, 0},
    {'#', 49, 0},
    {';', 57, 0},
    {'^', 65, 0},
    {'*', 73, 0},
    {'`', 81, 0},
    {'"', 89, 0},
    {'~', 97, 0},
    {'\'', 105, 0},
    {'.', 113, 0},
    {'/', 120, 0},
};

static const lora_textInputKey keyboard_keys_row_2[] = {
    {'Q', 1, 10},
    {'W', 9, 10},
    {'E', 17, 10},
    {'R', 25, 10},
    {'T', 33, 10},
    {'Y', 41, 10},
    {'U', 49, 10},
    {'I', 57, 10},
    {'O', 65, 10},
    {'P', 73, 10},
    {'0', 81, 10},
    {'1', 89, 10},
    {'2', 97, 10},
    {'3', 105, 10},
    {'=', 113, 10},
    {'-', 120, 10},
};

static const lora_textInputKey keyboard_keys_row_3[] = {
    {'A', 1, 21},
    {'S', 9, 21},
    {'D', 18, 21},
    {'F', 25, 21},
    {'G', 33, 21},
    {'H', 41, 21},
    {'J', 49, 21},
    {'K', 57, 21},
    {'L', 65, 21},
    {BACKSPACE_KEY, 72, 13},
    {'4', 89, 21},
    {'5', 97, 21},
    {'6', 105, 21},
    {'$', 113, 21},
    {'%', 120, 21},

};

static const lora_textInputKey keyboard_keys_row_4[] = {
    {'Z', 1, 33},
    {'X', 9, 33},
    {'C', 18, 33},
    {'V', 25, 33},
    {'B', 33, 33},
    {'N', 41, 33},
    {'M', 49, 33},
    {'_', 57, 33},
    {ENTER_KEY, 64, 24},
    {'7', 89, 33},
    {'8', 97, 33},
    {'9', 105, 33},
    {'!', 113, 33},
    {'+', 120, 33},
};

static uint8_t get_row_size(uint8_t row_index) {
    uint8_t row_size = 0;

    switch(row_index + 1) {
    case 1:
        row_size = sizeof(keyboard_keys_row_1) / sizeof(lora_textInputKey);
        break;
    case 2:
        row_size = sizeof(keyboard_keys_row_2) / sizeof(lora_textInputKey);
        break;
    case 3:
        row_size = sizeof(keyboard_keys_row_3) / sizeof(lora_textInputKey);
        break;
    case 4:
        row_size = sizeof(keyboard_keys_row_4) / sizeof(lora_textInputKey);
        break;
    }

    return row_size;
}

static const lora_textInputKey* get_row(uint8_t row_index) {
    const lora_textInputKey* row = NULL;

    switch(row_index + 1) {
    case 1:
        row = keyboard_keys_row_1;
        break;
    case 2:
        row = keyboard_keys_row_2;
        break;
    case 3:
        row = keyboard_keys_row_3;
        break;
    case 4:
        row = keyboard_keys_row_4;
        break;
    }

    return row;
}

static char get_selected_char(lora_textInputModel* model) {
    return get_row(model->selected_row)[model->selected_column].text;
}

static bool char_is_lowercase(char letter) {
    return (letter >= 0x61 && letter <= 0x7A);
}

static char char_to_uppercase(const char letter) {
    switch(letter) {
    case '_':
        return 0x20;
        break;
    case '(':
        return 0x29;
        break;
    case '{':
        return 0x7d;
        break;
    case '[':
        return 0x5d;
        break;
    case '/':
        return 0x5c;
        break;
    case ';':
        return 0x3a;
        break;
    case '.':
        return 0x2c;
        break;
    case '!':
        return 0x3f;
        break;
    case '<':
        return 0x3e;
        break;
    }
    if(isalpha(letter)) {
        return (letter - 0x20);
    } else {
        return letter;
    }
}

static void lora_text_input_backspace_cb(lora_textInputModel* model) {
    uint8_t text_length = model->clear_default_text ? 1 : strlen(model->text_buffer);
    if(text_length > 0) {
        model->text_buffer[text_length - 1] = 0;
    }
}

static void lora_text_input_view_draw_callback(Canvas* canvas, void* _model) {
    lora_textInputModel* model = _model;
    uint8_t text_length = model->text_buffer ? strlen(model->text_buffer) : 0;
    uint8_t needed_string_width = canvas_width(canvas) - 8;
    uint8_t start_pos = 4;

    const char* text = model->text_buffer;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    canvas_draw_str(canvas, 2, 7, model->header);
    elements_slightly_rounded_frame(canvas, 1, 8, 126, 12);

    if(canvas_string_width(canvas, text) > needed_string_width) {
        canvas_draw_str(canvas, start_pos, 17, "...");
        start_pos += 6;
        needed_string_width -= 8;
    }

    while(text != 0 && canvas_string_width(canvas, text) > needed_string_width) {
        text++;
    }

    if(model->clear_default_text) {
        elements_slightly_rounded_box(
            canvas, start_pos - 1, 14, canvas_string_width(canvas, text) + 2, 10);
        canvas_set_color(canvas, ColorWhite);
    } else {
        canvas_draw_str(canvas, start_pos + canvas_string_width(canvas, text) + 1, 18, "|");
        canvas_draw_str(canvas, start_pos + canvas_string_width(canvas, text) + 2, 18, "|");
    }
    canvas_draw_str(canvas, start_pos, 17, text);

    canvas_set_font(canvas, FontKeyboard);

    for(uint8_t row = 0; row <= keyboard_row_count; row++) {
        const uint8_t column_count = get_row_size(row);
        const lora_textInputKey* keys = get_row(row);

        for(size_t column = 0; column < column_count; column++) {
            if(keys[column].text == ENTER_KEY) {
                canvas_set_color(canvas, ColorBlack);
                if(model->selected_row == row && model->selected_column == column) {
                    canvas_draw_icon(
                        canvas,
                        keyboard_origin_x + keys[column].x,
                        keyboard_origin_y + keys[column].y,
                        &I_KeySaveSelected_24x11);
                } else {
                    canvas_draw_icon(
                        canvas,
                        keyboard_origin_x + keys[column].x,
                        keyboard_origin_y + keys[column].y,
                        &I_KeySave_24x11);
                }
            } else if(keys[column].text == BACKSPACE_KEY) {
                canvas_set_color(canvas, ColorBlack);
                if(model->selected_row == row && model->selected_column == column) {
                    canvas_draw_icon(
                        canvas,
                        keyboard_origin_x + keys[column].x,
                        keyboard_origin_y + keys[column].y,
                        &I_KeyBackspaceSelected_16x9);
                } else {
                    canvas_draw_icon(
                        canvas,
                        keyboard_origin_x + keys[column].x,
                        keyboard_origin_y + keys[column].y,
                        &I_KeyBackspace_16x9);
                }
            } else {
                if(model->selected_row == row && model->selected_column == column) {
                    canvas_set_color(canvas, ColorBlack);
                    canvas_draw_box(
                        canvas,
                        keyboard_origin_x + keys[column].x - 1,
                        keyboard_origin_y + keys[column].y - 8,
                        7,
                        10);
                    canvas_set_color(canvas, ColorWhite);
                } else {
                    canvas_set_color(canvas, ColorBlack);
                }

                if(model->clear_default_text ||
                   (text_length == 0 && char_is_lowercase(keys[column].text))) {
                    canvas_draw_glyph(
                        canvas,
                        keyboard_origin_x + keys[column].x,
                        keyboard_origin_y + keys[column].y,
                        //char_to_uppercase(keys[column].text));
                        keys[column].text);
                } else {
                    canvas_draw_glyph(
                        canvas,
                        keyboard_origin_x + keys[column].x,
                        keyboard_origin_y + keys[column].y,
                        keys[column].text);
                }
            }
        }
    }
    if(model->valadator_message_visible) {
        canvas_set_font(canvas, FontSecondary);
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_box(canvas, 8, 10, 110, 48);
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_icon(canvas, 10, 14, &I_WarningDolphin_45x42);
        canvas_draw_rframe(canvas, 8, 8, 112, 50, 3);
        canvas_draw_rframe(canvas, 9, 9, 110, 48, 2);
        elements_multiline_text(canvas, 62, 20, furi_string_get_cstr(model->validator_text));
        canvas_set_font(canvas, FontKeyboard);
    }
}

static void
    lora_text_input_handle_up(lora_textInput* lora_text_input, lora_textInputModel* model) {
    UNUSED(lora_text_input);
    if(model->selected_row > 0) {
        model->selected_row--;
        if(model->selected_column > get_row_size(model->selected_row) - 6) {
            model->selected_column = model->selected_column + 1;
        }
    }
}

static void
    lora_text_input_handle_down(lora_textInput* lora_text_input, lora_textInputModel* model) {
    UNUSED(lora_text_input);
    if(model->selected_row < keyboard_row_count - 1) {
        model->selected_row++;
        if(model->selected_column > get_row_size(model->selected_row) - 4) {
            model->selected_column = model->selected_column - 1;
        }
    }
}

static void
    lora_text_input_handle_left(lora_textInput* lora_text_input, lora_textInputModel* model) {
    UNUSED(lora_text_input);
    if(model->selected_column > 0) {
        model->selected_column--;
    } else {
        model->selected_column = get_row_size(model->selected_row) - 1;
    }
}

static void
    lora_text_input_handle_right(lora_textInput* lora_text_input, lora_textInputModel* model) {
    UNUSED(lora_text_input);
    if(model->selected_column < get_row_size(model->selected_row) - 1) {
        model->selected_column++;
    } else {
        model->selected_column = 0;
    }
}

static void lora_text_input_handle_ok(
    lora_textInput* lora_text_input,
    lora_textInputModel* model,
    bool shift) {
    char selected = get_selected_char(model);
    uint8_t text_length = strlen(model->text_buffer);

    if(shift) {
        selected = char_to_uppercase(selected);
    }

    if(selected == ENTER_KEY) {
        if(model->validator_callback &&
           (!model->validator_callback(
               model->text_buffer, model->validator_text, model->validator_callback_context))) {
            model->valadator_message_visible = true;
            furi_timer_start(lora_text_input->timer, furi_kernel_get_tick_frequency() * 4);
        } else if(model->callback != 0 && text_length > 0) {
            model->callback(model->callback_context);
        }
    } else if(selected == BACKSPACE_KEY) {
        lora_text_input_backspace_cb(model);
    } else {
        if(model->clear_default_text) {
            text_length = 0;
        }
        if(text_length < (model->text_buffer_size - 1)) {
            if(text_length == 0 && char_is_lowercase(selected)) {
                //selected = char_to_uppercase(selected);
            }
            model->text_buffer[text_length] = selected;
            model->text_buffer[text_length + 1] = 0;
        }
    }
    model->clear_default_text = false;
}

static bool lora_text_input_view_input_callback(InputEvent* event, void* context) {
    lora_textInput* lora_text_input = context;
    furi_assert(lora_text_input);

    bool consumed = false;

    // Acquire model
    lora_textInputModel* model = view_get_model(lora_text_input->view);

    if((!(event->type == InputTypePress) && !(event->type == InputTypeRelease)) &&
       model->valadator_message_visible) {
        model->valadator_message_visible = false;
        consumed = true;
    } else if(event->type == InputTypeShort) {
        consumed = true;
        switch(event->key) {
        case InputKeyUp:
            lora_text_input_handle_up(lora_text_input, model);
            break;
        case InputKeyDown:
            lora_text_input_handle_down(lora_text_input, model);
            break;
        case InputKeyLeft:
            lora_text_input_handle_left(lora_text_input, model);
            break;
        case InputKeyRight:
            lora_text_input_handle_right(lora_text_input, model);
            break;
        case InputKeyOk:
            lora_text_input_handle_ok(lora_text_input, model, false);
            break;
        default:
            consumed = false;
            break;
        }
    } else if(event->type == InputTypeLong) {
        consumed = true;
        switch(event->key) {
        case InputKeyUp:
            lora_text_input_handle_up(lora_text_input, model);
            break;
        case InputKeyDown:
            lora_text_input_handle_down(lora_text_input, model);
            break;
        case InputKeyLeft:
            lora_text_input_handle_left(lora_text_input, model);
            break;
        case InputKeyRight:
            lora_text_input_handle_right(lora_text_input, model);
            break;
        case InputKeyOk:
            lora_text_input_handle_ok(lora_text_input, model, true);
            break;
        case InputKeyBack:
            lora_text_input_backspace_cb(model);
            break;
        default:
            consumed = false;
            break;
        }
    } else if(event->type == InputTypeRepeat) {
        consumed = true;
        switch(event->key) {
        case InputKeyUp:
            lora_text_input_handle_up(lora_text_input, model);
            break;
        case InputKeyDown:
            lora_text_input_handle_down(lora_text_input, model);
            break;
        case InputKeyLeft:
            lora_text_input_handle_left(lora_text_input, model);
            break;
        case InputKeyRight:
            lora_text_input_handle_right(lora_text_input, model);
            break;
        case InputKeyBack:
            lora_text_input_backspace_cb(model);
            break;
        default:
            consumed = false;
            break;
        }
    }

    // Commit model
    view_commit_model(lora_text_input->view, consumed);

    return consumed;
}

void lora_text_input_timer_callback(void* context) {
    furi_assert(context);
    lora_textInput* lora_text_input = context;

    with_view_model(
        lora_text_input->view,
        lora_textInputModel * model,
        { model->valadator_message_visible = false; },
        true);
}

lora_textInput* lora_text_input_alloc() {
    lora_textInput* lora_text_input = malloc(sizeof(lora_textInput));
    lora_text_input->view = view_alloc();
    view_set_context(lora_text_input->view, lora_text_input);
    view_allocate_model(lora_text_input->view, ViewModelTypeLocking, sizeof(lora_textInputModel));
    view_set_draw_callback(lora_text_input->view, lora_text_input_view_draw_callback);
    view_set_input_callback(lora_text_input->view, lora_text_input_view_input_callback);

    lora_text_input->timer =
        furi_timer_alloc(lora_text_input_timer_callback, FuriTimerTypeOnce, lora_text_input);

    with_view_model(
        lora_text_input->view,
        lora_textInputModel * model,
        { model->validator_text = furi_string_alloc(); },
        false);

    lora_text_input_reset(lora_text_input);

    return lora_text_input;
}

void lora_text_input_free(lora_textInput* lora_text_input) {
    furi_assert(lora_text_input);
    with_view_model(
        lora_text_input->view,
        lora_textInputModel * model,
        { furi_string_free(model->validator_text); },
        false);

    // Send stop command
    furi_timer_stop(lora_text_input->timer);
    // Release allocated memory
    furi_timer_free(lora_text_input->timer);

    view_free(lora_text_input->view);

    free(lora_text_input);
}

void lora_text_input_reset(lora_textInput* lora_text_input) {
    furi_assert(lora_text_input);
    with_view_model(
        lora_text_input->view,
        lora_textInputModel * model,
        {
            model->text_buffer_size = 0;
            model->header = "";
            model->selected_row = 0;
            model->selected_column = 0;
            model->clear_default_text = false;
            model->text_buffer = NULL;
            model->text_buffer_size = 0;
            model->callback = NULL;
            model->callback_context = NULL;
            model->validator_callback = NULL;
            model->validator_callback_context = NULL;
            furi_string_reset(model->validator_text);
            model->valadator_message_visible = false;
        },
        true);
}

View* lora_text_input_get_view(lora_textInput* lora_text_input) {
    furi_assert(lora_text_input);
    return lora_text_input->view;
}

void lora_text_input_set_result_callback(
    lora_textInput* lora_text_input,
    lora_textInputCallback callback,
    void* callback_context,
    char* text_buffer,
    size_t text_buffer_size,
    bool clear_default_text) {
    with_view_model(
        lora_text_input->view,
        lora_textInputModel * model,
        {
            model->callback = callback;
            model->callback_context = callback_context;
            model->text_buffer = text_buffer;
            model->text_buffer_size = text_buffer_size;
            model->clear_default_text = clear_default_text;
            if(text_buffer && text_buffer[0] != '\0') {
                // Set focus on Save
                model->selected_row = 2;
                model->selected_column = 8;
            }
        },
        true);
}

void lora_text_input_set_validator(
    lora_textInput* lora_text_input,
    lora_textInputValidatorCallback callback,
    void* callback_context) {
    with_view_model(
        lora_text_input->view,
        lora_textInputModel * model,
        {
            model->validator_callback = callback;
            model->validator_callback_context = callback_context;
        },
        true);
}

lora_textInputValidatorCallback
    lora_text_input_get_validator_callback(lora_textInput* lora_text_input) {
    lora_textInputValidatorCallback validator_callback = NULL;
    with_view_model(
        lora_text_input->view,
        lora_textInputModel * model,
        { validator_callback = model->validator_callback; },
        false);
    return validator_callback;
}

void* lora_text_input_get_validator_callback_context(lora_textInput* lora_text_input) {
    void* validator_callback_context = NULL;
    with_view_model(
        lora_text_input->view,
        lora_textInputModel * model,
        { validator_callback_context = model->validator_callback_context; },
        false);
    return validator_callback_context;
}

void lora_text_input_set_header_text(lora_textInput* lora_text_input, const char* text) {
    with_view_model(
        lora_text_input->view, lora_textInputModel * model, { model->header = text; }, true);
}
