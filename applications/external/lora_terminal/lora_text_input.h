#pragma once

#include <gui/view.h>
#include "lora_validators.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Text input anonymous structure */
typedef struct lora_textInput lora_textInput;
typedef void (*lora_textInputCallback)(void* context);
typedef bool (*lora_textInputValidatorCallback)(const char* text, FuriString* error, void* context);

/** Allocate and initialize text input 
 * 
 * This text input is used to enter string
 *
 * @return     lora_textInput instance
 */
lora_textInput* lora_text_input_alloc();

/** Deinitialize and free text input
 *
 * @param      lora_text_input  lora_textInput instance
 */
void lora_text_input_free(lora_textInput* lora_text_input);

/** Clean text input view Note: this function does not free memory
 *
 * @param      lora_text_input  Text input instance
 */
void lora_text_input_reset(lora_textInput* lora_text_input);

/** Get text input view
 *
 * @param      lora_text_input  lora_textInput instance
 *
 * @return     View instance that can be used for embedding
 */
View* lora_text_input_get_view(lora_textInput* lora_text_input);

/** Set text input result callback
 *
 * @param      lora_text_input          lora_textInput instance
 * @param      callback            callback fn
 * @param      callback_context    callback context
 * @param      text_buffer         pointer to YOUR text buffer, that we going
 *                                 to modify
 * @param      text_buffer_size    YOUR text buffer size in bytes. Max string
 *                                 length will be text_buffer_size-1.
 * @param      clear_default_text  clear text from text_buffer on first OK
 *                                 event
 */
void lora_text_input_set_result_callback(
    lora_textInput* lora_text_input,
    lora_textInputCallback callback,
    void* callback_context,
    char* text_buffer,
    size_t text_buffer_size,
    bool clear_default_text);

void lora_text_input_set_validator(
    lora_textInput* lora_text_input,
    lora_textInputValidatorCallback callback,
    void* callback_context);

lora_textInputValidatorCallback
    lora_text_input_get_validator_callback(lora_textInput* lora_text_input);

void* lora_text_input_get_validator_callback_context(lora_textInput* lora_text_input);

/** Set text input header text
 *
 * @param      lora_text_input  lora_textInput instance
 * @param      text        text to be shown
 */
void lora_text_input_set_header_text(lora_textInput* lora_text_input, const char* text);

#ifdef __cplusplus
}
#endif
