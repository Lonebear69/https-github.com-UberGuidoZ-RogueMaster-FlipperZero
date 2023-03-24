#include "lora_terminal_scene.h"

// Generate scene on_enter handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_enter,
void (*const lora_terminal_scene_on_enter_handlers[])(void*) = {
#include "lora_terminal_scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_event handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_event,
bool (*const lora_terminal_scene_on_event_handlers[])(void* context, SceneManagerEvent event) = {
#include "lora_terminal_scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_exit handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_exit,
void (*const lora_terminal_scene_on_exit_handlers[])(void* context) = {
#include "lora_terminal_scene_config.h"
};
#undef ADD_SCENE

// Initialize scene handlers configuration structure
const SceneManagerHandlers lora_terminal_scene_handlers = {
    .on_enter_handlers = lora_terminal_scene_on_enter_handlers,
    .on_event_handlers = lora_terminal_scene_on_event_handlers,
    .on_exit_handlers = lora_terminal_scene_on_exit_handlers,
    .scene_num = lora_terminalSceneNum,
};
