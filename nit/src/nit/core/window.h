#pragma once
#include "nit/input/input_keycodes.h"

struct GLFWwindow;

namespace nit
{
    enum class CursorMode : u8
    {
        Normal
      , Disabled
      , Hidden
      , Captured
    };
    
    struct WindowCfg
    {
        String      title           = "Window :3";
        i32         width           = 1280;
        i32         height          = 720;
        bool        v_sync          = true;
        bool        start_maximized = true;
        CursorMode  cursor_mode     = CursorMode::Normal;
        bool        render_api_logs = false;
    };

    struct Window
    {
        GLFWwindow* handler     = nullptr;
        String      title;
        bool        v_sync      = false;
        CursorMode  cursor_mode = CursorMode::Normal;
    };

    void        window_set_instance(Window* window_instance);
    Window*     window_get_instance();
    bool        window_has_instance();
    void        window_init(const WindowCfg& cfg = {});
    void        window_finish();
    void        window_close();
        
    void        window_set_title(const String& new_title);
    void        window_set_v_sync(bool enabled);
    void        window_set_cursor_mode(const CursorMode mode);
    bool        window_should_close();
    void        window_retrieve_size(i32* width, i32* height);
    Vector2     window_get_size();
    float       window_get_aspect();
    f64         window_get_time();

    void        input_get_cursor_position(f64* x, f64* y);
    Vector2     input_get_cursor_position();
    i32         input_get_mouse_button(i32 button);
    bool        input_is_mouse_button_pressed(MouseButton button);
    i32         input_get_key(i32 key);
    bool        input_is_key_pressed(Key key);
    const char* input_get_joystick_name(i32 jid);
    const u8*   input_get_joystick_buttons(i32 jid, i32* count);
    const f32*  input_get_joystick_axes(i32 jid, i32* count);
}