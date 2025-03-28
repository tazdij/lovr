#include "os.h"
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/dom_pk_codes.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define CANVAS "#canvas"

static struct {
  fn_quit* onQuitRequest;
  fn_visible* onWindowVisible;
  fn_focus* onWindowFocus;
  fn_resize* onWindowResize;
  fn_key* onKeyboardEvent;
  fn_mouse_button* onMouseButton;
  fn_mouse_move* onMouseMove;
  fn_mousewheel_move* onMouseWheelMove;
  bool keyMap[OS_KEY_COUNT];
  bool mouseMap[2];
  os_mouse_mode mouseMode;
  long mouseX;
  long mouseY;
  uint32_t width;
  uint32_t height;
} state;

static const char* onBeforeUnload(int type, const void* unused, void* userdata) {
  if (state.onQuitRequest) {
    state.onQuitRequest();
  }
  return NULL;
}

static EM_BOOL onVisibilityChanged(int type, const EmscriptenVisibilityChangeEvent* visibility, void* userdata) {
  if (state.onWindowVisible) {
    state.onWindowVisible(!visibility->hidden);
    return true;
  }
  return false;
}

static EM_BOOL onFocusChanged(int type, const EmscriptenFocusEvent* data, void* userdata) {
  if (state.onWindowFocus) {
    state.onWindowFocus(type == EMSCRIPTEN_EVENT_FOCUS);
    return true;
  }
  return false;
}

static EM_BOOL onResize(int type, const EmscriptenUiEvent* data, void* userdata) {
  int newWidth, newHeight;
  emscripten_get_canvas_element_size(CANVAS, &newWidth, &newHeight);

  if (state.width != (uint32_t) newWidth || state.height != (uint32_t) newHeight) {
    state.width = newWidth;
    state.height = newHeight;
    if (state.onWindowResize) {
      state.onWindowResize(state.width, state.height);
      return true;
    }
  }

  return false;
}

static EM_BOOL onMouseButton(int type, const EmscriptenMouseEvent* data, void* userdata) {
  os_mouse_button button;
  switch (data->button) {
    case 0: button = MOUSE_LEFT; break;
    case 2: button = MOUSE_RIGHT; break;
    default: return false;
  }

  bool pressed = type == EMSCRIPTEN_EVENT_MOUSEDOWN;

  if (state.onMouseButton) {
    state.onMouseButton((int) button, pressed);
  }

  state.mouseMap[button] = pressed;
  return false;
}

static EM_BOOL onMouseMove(int type, const EmscriptenMouseEvent* data, void* userdata) {
  if (state.mouseMode == MOUSE_MODE_GRABBED) {
    state.mouseX += data->movementX;
    state.mouseY += data->movementY;
  } else {
    state.mouseX = data->clientX;
    state.mouseY = data->clientY;
  }
  if (state.onMouseMove) {
    state.onMouseMove(state.mouseX, state.mouseY);
  }
  return false;
}

static EM_BOOL onMouseWheelMove(int type, const EmscriptenWheelEvent* data, void* userdata) {
  if (state.onMouseWheelMove) {
    state.onMouseWheelMove(data->deltaX, -data->deltaY);
  }
  return false;
}

static EM_BOOL onKeyEvent(int type, const EmscriptenKeyboardEvent* data, void* userdata) {
  os_key key;
  DOM_PK_CODE_TYPE scancode = emscripten_compute_dom_pk_code(data->code);
  switch (scancode) {
    case DOM_PK_ESCAPE: key = OS_KEY_ESCAPE; break;
    case DOM_PK_0: key = OS_KEY_0; break;
    case DOM_PK_1: key = OS_KEY_1; break;
    case DOM_PK_2: key = OS_KEY_2; break;
    case DOM_PK_3: key = OS_KEY_3; break;
    case DOM_PK_4: key = OS_KEY_4; break;
    case DOM_PK_5: key = OS_KEY_5; break;
    case DOM_PK_6: key = OS_KEY_6; break;
    case DOM_PK_7: key = OS_KEY_7; break;
    case DOM_PK_8: key = OS_KEY_8; break;
    case DOM_PK_9: key = OS_KEY_9; break;
    case DOM_PK_MINUS: key = OS_KEY_MINUS; break;
    case DOM_PK_EQUAL: key = OS_KEY_EQUALS; break;
    case DOM_PK_BACKSPACE: key = OS_KEY_BACKSPACE; break;
    case DOM_PK_TAB: key = OS_KEY_TAB; break;
    case DOM_PK_Q: key = OS_KEY_Q; break;
    case DOM_PK_W: key = OS_KEY_W; break;
    case DOM_PK_E: key = OS_KEY_E; break;
    case DOM_PK_R: key = OS_KEY_R; break;
    case DOM_PK_T: key = OS_KEY_T; break;
    case DOM_PK_Y: key = OS_KEY_Y; break;
    case DOM_PK_U: key = OS_KEY_U; break;
    case DOM_PK_I: key = OS_KEY_I; break;
    case DOM_PK_O: key = OS_KEY_O; break;
    case DOM_PK_P: key = OS_KEY_P; break;
    case DOM_PK_BRACKET_LEFT: key = OS_KEY_LEFT_BRACKET; break;
    case DOM_PK_BRACKET_RIGHT: key = OS_KEY_RIGHT_BRACKET; break;
    case DOM_PK_ENTER: key = OS_KEY_ENTER; break;
    case DOM_PK_CONTROL_LEFT: key = OS_KEY_LEFT_CONTROL; break;
    case DOM_PK_A: key = OS_KEY_A; break;
    case DOM_PK_S: key = OS_KEY_S; break;
    case DOM_PK_D: key = OS_KEY_D; break;
    case DOM_PK_F: key = OS_KEY_F; break;
    case DOM_PK_G: key = OS_KEY_G; break;
    case DOM_PK_H: key = OS_KEY_H; break;
    case DOM_PK_J: key = OS_KEY_J; break;
    case DOM_PK_K: key = OS_KEY_K; break;
    case DOM_PK_L: key = OS_KEY_L; break;
    case DOM_PK_SEMICOLON: key = OS_KEY_SEMICOLON; break;
    case DOM_PK_QUOTE: key = OS_KEY_APOSTROPHE; break;
    case DOM_PK_BACKQUOTE: key = OS_KEY_BACKTICK; break;
    case DOM_PK_SHIFT_LEFT: key = OS_KEY_LEFT_SHIFT; break;
    case DOM_PK_BACKSLASH: key = OS_KEY_BACKSLASH; break;
    case DOM_PK_Z: key = OS_KEY_Z; break;
    case DOM_PK_X: key = OS_KEY_X; break;
    case DOM_PK_C: key = OS_KEY_C; break;
    case DOM_PK_V: key = OS_KEY_V; break;
    case DOM_PK_B: key = OS_KEY_B; break;
    case DOM_PK_N: key = OS_KEY_N; break;
    case DOM_PK_M: key = OS_KEY_M; break;
    case DOM_PK_COMMA: key = OS_KEY_COMMA; break;
    case DOM_PK_PERIOD: key = OS_KEY_PERIOD; break;
    case DOM_PK_SLASH: key = OS_KEY_SLASH; break;
    case DOM_PK_SHIFT_RIGHT: key = OS_KEY_RIGHT_SHIFT; break;
    case DOM_PK_ALT_LEFT: key = OS_KEY_LEFT_ALT; break;
    case DOM_PK_SPACE: key = OS_KEY_SPACE; break;
    case DOM_PK_CAPS_LOCK: key = OS_KEY_CAPS_LOCK; break;
    case DOM_PK_F1: key = OS_KEY_F1; break;
    case DOM_PK_F2: key = OS_KEY_F2; break;
    case DOM_PK_F3: key = OS_KEY_F3; break;
    case DOM_PK_F4: key = OS_KEY_F4; break;
    case DOM_PK_F5: key = OS_KEY_F5; break;
    case DOM_PK_F6: key = OS_KEY_F6; break;
    case DOM_PK_F7: key = OS_KEY_F7; break;
    case DOM_PK_F8: key = OS_KEY_F8; break;
    case DOM_PK_F9: key = OS_KEY_F9; break;
    case DOM_PK_SCROLL_LOCK: key = OS_KEY_SCROLL_LOCK; break;
    case DOM_PK_F11: key = OS_KEY_F11; break;
    case DOM_PK_F12: key = OS_KEY_F12; break;
    case DOM_PK_CONTROL_RIGHT: key = OS_KEY_RIGHT_CONTROL; break;
    case DOM_PK_ALT_RIGHT: key = OS_KEY_RIGHT_ALT; break;
    case DOM_PK_NUM_LOCK: key = OS_KEY_NUM_LOCK; break;
    case DOM_PK_HOME: key = OS_KEY_HOME; break;
    case DOM_PK_ARROW_UP: key = OS_KEY_UP; break;
    case DOM_PK_PAGE_UP: key = OS_KEY_PAGE_UP; break;
    case DOM_PK_ARROW_LEFT: key = OS_KEY_LEFT; break;
    case DOM_PK_ARROW_RIGHT: key = OS_KEY_RIGHT; break;
    case DOM_PK_END: key = OS_KEY_END; break;
    case DOM_PK_ARROW_DOWN: key = OS_KEY_DOWN; break;
    case DOM_PK_PAGE_DOWN: key = OS_KEY_PAGE_DOWN; break;
    case DOM_PK_INSERT: key = OS_KEY_INSERT; break;
    case DOM_PK_DELETE: key = OS_KEY_DELETE; break;
    case DOM_PK_OS_LEFT: key = OS_KEY_LEFT_OS; break;
    case DOM_PK_OS_RIGHT: key = OS_KEY_RIGHT_OS; break;
    default: return false;
  }

  os_button_action action = type == EMSCRIPTEN_EVENT_KEYDOWN ? BUTTON_PRESSED : BUTTON_RELEASED;
  state.keyMap[key] = action == BUTTON_PRESSED;

  if (state.onKeyboardEvent) {
    state.onKeyboardEvent(action, key, scancode, data->repeat);
  }

  return false;
}

bool os_init(void) {
  emscripten_set_beforeunload_callback(NULL, onBeforeUnload);
  emscripten_set_visibilitychange_callback(CANVAS, true, onVisibilityChanged);
  emscripten_set_focus_callback(CANVAS, NULL, true, onFocusChanged);
  emscripten_set_blur_callback(CANVAS, NULL, true, onFocusChanged);
  emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, true, onResize);
  emscripten_set_mousedown_callback(CANVAS, NULL, true, onMouseButton);
  emscripten_set_mouseup_callback(CANVAS, NULL, true, onMouseButton);
  emscripten_set_mousemove_callback(CANVAS, NULL, true, onMouseMove);
  emscripten_set_wheel_callback(CANVAS, NULL, true, onMouseWheelMove);
  emscripten_set_keydown_callback(CANVAS, NULL, true, onKeyEvent);
  emscripten_set_keyup_callback(CANVAS, NULL, true, onKeyEvent);
  int width, height;
  emscripten_get_canvas_element_size(CANVAS, &width, &height);
  state.width = width;
  state.height = height;
  return true;
}

void os_destroy(void) {
  emscripten_set_beforeunload_callback(NULL, NULL);
  emscripten_set_visibilitychange_callback(CANVAS, true, NULL);
  emscripten_set_focus_callback(CANVAS, NULL, true, NULL);
  emscripten_set_blur_callback(CANVAS, NULL, true, NULL);
  emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, true, NULL);
  emscripten_set_mousedown_callback(CANVAS, NULL, true, NULL);
  emscripten_set_mouseup_callback(CANVAS, NULL, true, NULL);
  emscripten_set_mousemove_callback(CANVAS, NULL, true, NULL);
  emscripten_set_keydown_callback(CANVAS, NULL, true, NULL);
  emscripten_set_keyup_callback(CANVAS, NULL, true, NULL);
}

const char* os_get_name(void) {
  return "Web";
}

uint32_t os_get_core_count(void) {
  return 1;
}

void os_open_console(void) {
  //
}

double os_get_time(void) {
  return emscripten_get_now() / 1000.;
}

void os_sleep(double seconds) {
  emscripten_sleep((unsigned int) (seconds * 1000. + .5));
}

void os_request_permission(os_permission permission) {
  //
}

const char* os_get_clipboard_text(void) {
  return NULL;
}

void os_set_clipboard_text(const char* text) {
  //
}

void* os_vm_init(size_t size) {
  return malloc(size);
}

bool os_vm_free(void* p, size_t size) {
  free(p);
  return true;
}

bool os_vm_commit(void* p, size_t size) {
  return true;
}

bool os_vm_release(void* p, size_t size) {
  return true;
}

void os_thread_attach(void) {
  //
}

void os_thread_detach(void) {
  //
}

void os_poll_events(void) {
  //
}

void os_on_permission(fn_permission* callback) {
  //
}

size_t os_get_home_directory(char* buffer, size_t size) {
  const char* path = getenv("HOME");
  size_t length = strlen(path);
  if (length >= size) { return 0; }
  memcpy(buffer, path, length);
  buffer[length] = '\0';
  return length;
}

size_t os_get_data_directory(char* buffer, size_t size) {
  const char* path = "/home/web_user";
  size_t length = strlen(path);
  if (length >= size) { return 0; }
  memcpy(buffer, path, length);
  buffer[length] = '\0';
  return length;
}

size_t os_get_working_directory(char* buffer, size_t size) {
  return getcwd(buffer, size) ? strlen(buffer) : 0;
}

size_t os_get_executable_path(char* buffer, size_t size) {
  return 0;
}

size_t os_get_bundle_path(char* buffer, size_t size, const char** root) {
  *root = NULL;
  return 0;
}

bool os_window_open(const os_window_config* flags) {
  return true;
}

bool os_window_is_open(void) {
  return true;
}

bool os_window_is_visible(void) {
  EmscriptenVisibilityChangeEvent visibility;
  emscripten_get_visibility_status(&visibility);
  return !visibility.hidden;
}

bool os_window_is_focused(void) {
  return true;
}

void os_window_get_size(uint32_t* width, uint32_t* height) {
  *width = state.width;
  *height = state.height;
}

float os_window_get_pixel_density(void) {
  return 1.f; // TODO
}

void os_on_quit(fn_quit* callback) {
  state.onQuitRequest = callback;
}

void os_on_visible(fn_visible* callback) {
  state.onWindowVisible = callback;
}

void os_on_focus(fn_focus* callback) {
  state.onWindowFocus = callback;
}

void os_on_resize(fn_resize* callback) {
  state.onWindowResize = callback;
}

void os_on_key(fn_key* callback) {
  state.onKeyboardEvent = callback;
}

void os_on_text(fn_text* callback) {
  //
}

void os_on_mouse_button(fn_mouse_button* callback) {
  state.onMouseButton = callback;
}

void os_on_mouse_move(fn_mouse_move* callback) {
  state.onMouseMove = callback;
}

void os_on_mousewheel_move(fn_mousewheel_move* callback) {
  state.onMouseWheelMove = callback;
}

void os_get_mouse_position(double* x, double* y) {
  *x = state.mouseX;
  *y = state.mouseY;
}

void os_set_mouse_mode(os_mouse_mode mode) {
  if (state.mouseMode != mode) {
    state.mouseMode = mode;
    if (mode == MOUSE_MODE_GRABBED) {
      emscripten_run_script("Module['canvas'].requestPointerLock();");
    } else {
      emscripten_run_script("document.exitPointerLock();");
    }
  }
}

bool os_is_mouse_down(os_mouse_button button) {
  return state.mouseMap[button];
}

bool os_is_key_down(os_key key) {
  return state.keyMap[key];
}
