#include "window.h"
#ifdef NIT_GRAPHICS_API_OPENGL
#include <glad/glad.h>
#endif
#include <glfw/glfw3.h>

#define NIT_CHECK_WINDOW_CREATED NIT_CHECK_MSG(window, "Forget to call set_instance!");

#define NIT_CHECK_WINDOW_INITIALIZED \
    NIT_CHECK_WINDOW_CREATED \
    NIT_CHECK_MSG(window->handler, "Forget to call Init!");


namespace nit
{
    Window* window = nullptr;

    void window_set_instance(Window* window_instance)
    {
        NIT_CHECK(window_instance);
        window = window_instance;
    }

    Window* window_get_instance()
    {
        NIT_CHECK(window);
        return window;
    }

    bool window_has_instance()
    {
        return window != nullptr;
    }

    void APIENTRY OnDebugMessageCallback(GLenum source, GLenum type, u32 id, GLenum severity,
                                         GLsizei length, const char* message, const void* user_param)
    {
        NIT_LOG_TRACE("[OpenGL] %s ", message);
    }
    
    void window_init(const WindowCfg& cfg)
    {
        NIT_LOG_TRACE("Creating Window...");

        if (!window_has_instance())
        {
            static Window window_instance;
            window_set_instance(&window_instance);
        }
        
        window_finish();

        const bool initialized = glfwInit();

        if (!initialized)
        {
            NIT_CHECK_MSG(false, "GLFW Window initialisation failed!");
            return;
        }

        window->handler = glfwCreateWindow(cfg.width, cfg.height, cfg.title.c_str(), nullptr, nullptr);

        if (!window->handler)
        {
            NIT_CHECK_MSG(false, "GLFW Window creation failed!");
            return;
        }
        
        glfwMakeContextCurrent(window->handler);

        const bool gl_context = gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

        if (!gl_context)
        {
            NIT_CHECK_MSG(false, "Failed to set the OpenGL context!");
            return;
        }

#ifdef NIT_GRAPHICS_API_OPENGL
        if (cfg.render_api_logs)
        {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(OnDebugMessageCallback, nullptr);
        }
#endif
        
        window_set_v_sync(cfg.v_sync);
        window_set_title(cfg.title);
        window_set_cursor_mode(cfg.cursor_mode);

        if (cfg.start_maximized)
        {
            glfwMaximizeWindow(window->handler);
        }

        NIT_LOG_TRACE("Window created!");
    }

    void window_finish()
    {
        NIT_CHECK_WINDOW_CREATED
        if (window->handler)
        {
            window_close();
            glfwDestroyWindow(window->handler);
            window->handler = nullptr;
            glfwTerminate();
        }
    }

    void window_close()
    {
        NIT_CHECK_WINDOW_INITIALIZED
        glfwSetWindowShouldClose(window->handler, true);
    }

    void window_set_title(const String& new_title)
    {
        NIT_CHECK_WINDOW_INITIALIZED
        window->title = new_title;
        glfwSetWindowTitle(window->handler, new_title.c_str());
    }

    void window_set_v_sync(bool enabled)
    {
        NIT_CHECK_WINDOW_INITIALIZED
        window->v_sync = enabled;
        glfwSwapInterval(enabled ? 1 : 0);
    }

    void window_set_cursor_mode(const CursorMode mode)
    {
        NIT_CHECK_WINDOW_INITIALIZED
        window->cursor_mode = mode;

        switch (mode)
        {
        case CursorMode::Normal:
            glfwSetInputMode(window->handler, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            return;
        case CursorMode::Disabled:
            glfwSetInputMode(window->handler, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            return;
        case CursorMode::Hidden:
            glfwSetInputMode(window->handler, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            return;
        case CursorMode::Captured:
            glfwSetInputMode(window->handler, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
        }
    }

    bool window_should_close()
    {
        NIT_CHECK_WINDOW_INITIALIZED
        glfwSwapBuffers(window->handler);
        glfwPollEvents();
        return glfwWindowShouldClose(window->handler);
    }

    void window_retrieve_size(i32* width, i32* height)
    {
        NIT_CHECK_WINDOW_INITIALIZED
        glfwGetWindowSize(window->handler, width, height);
    }

    Vector2 window_get_size()
    {
        i32 x, y;
        window_retrieve_size(&x, &y);
        return { (f32)x, (f32) y };
    }

    float window_get_aspect()
    {
        i32 x, y;
        window_retrieve_size(&x, &y);
        return (f32)x / (f32)y;
    }

    f64 window_get_time()
    {
        NIT_CHECK_WINDOW_INITIALIZED
        return glfwGetTime();
    }
    
    void input_get_cursor_position(f64* x, f64* y)
    {
        NIT_CHECK_WINDOW_INITIALIZED
        glfwGetCursorPos(window->handler, x, y);
    }

    Vector2 input_get_cursor_position()
    {
        f64 x, y;
        input_get_cursor_position(&x, &y);
        return { (f32) x, (f32) y };
    }

    i32 input_get_mouse_button(i32 button)
    {
        NIT_CHECK_WINDOW_INITIALIZED
        return glfwGetMouseButton(window->handler, button);
    }

    bool input_is_mouse_button_pressed(MouseButton button)
    {
        const auto state = input_get_mouse_button(button);
        return state == GLFW_PRESS;
    }

    i32 input_get_key(i32 key)
    {
        NIT_CHECK_WINDOW_INITIALIZED
        return glfwGetKey(window->handler, key);
    }

    bool input_is_key_pressed(Key key)
    {
        const auto state = input_get_key(key);
        return state == GLFW_PRESS;
    }

    const char* input_get_joystick_name(i32 jid)
    {
        NIT_CHECK_WINDOW_INITIALIZED
        return glfwGetJoystickName(jid);
    }

    const u8* input_get_joystick_buttons(i32 jid, i32* count)
    {
        NIT_CHECK_WINDOW_INITIALIZED
        return glfwGetJoystickButtons(jid, count);
    }

    const f32* input_get_joystick_axes(i32 jid, i32* count)
    {
        NIT_CHECK_WINDOW_INITIALIZED
        return glfwGetJoystickAxes(jid, count);
    }
}
