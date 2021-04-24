#include "main.h"

#include "shader.v"
#include "shader.f"

static void show_log(GLint shader);
static void show_program_log(GLint program);
static void init_ogl(PTRI_STATE_T state);
static void init_shaders(PTRI_STATE_T state);
static void draw_triangle(PTRI_STATE_T state);
static int get_mouse(PTRI_STATE_T state, int* outx, int* outy);

int main()
{
    GLuint running = 1;
    GLfloat test = 0.1;
    printf("[TEST]: %d %f\n", running, test);

    bcm_host_init();
    memset(state, 0, sizeof(*state));
    state->verbose = 1;

    init_ogl(state);
    init_shaders(state);

    while (running)
    {
        int x, y, b = 0;
        b = get_mouse(state, &x, &y);
        if (b) { break; }
        draw_triangle(state);
    }
    return 0;
}

static void show_log(GLint shader)
{
    char log[1024];
    glGetShaderInfoLog(shader, sizeof(log), NULL, log);
    printf("[INFO]: %s\n", log);
}

static void show_program_log(GLint program)
{
    char log[1024];
    glGetProgramInfoLog(program, sizeof(log), NULL, log);
    printf("[INFO]: %s\n", log);
}

static void init_ogl(PTRI_STATE_T state)
{
    int32_t success = 0;
    EGLBoolean result = 0;
    EGLint num_config = 0;

    static EGL_DISPMANX_WINDOW_T native_window;
    EGLConfig config;

    DISPMANX_ELEMENT_HANDLE_T dispman_element;
    DISPMANX_DISPLAY_HANDLE_T dispman_display;
    DISPMANX_UPDATE_HANDLE_T dispman_update;
    VC_RECT_T dst_rect;
    VC_RECT_T src_rect;

    static const EGLint attribute_list[] =
    {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_NONE
    };

    static const EGLint context_attributes[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    state->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    assert(state->display != EGL_NO_DISPLAY);
    check();

    result = eglInitialize(state->display, NULL, NULL);
    assert(EGL_FALSE != result);
    check();

    result = eglChooseConfig(state->display, attribute_list, &config, 1, &num_config);
    assert(EGL_FALSE != result);
    check();

    result = eglBindAPI(EGL_OPENGL_ES_API);
    assert(EGL_FALSE != result);
    check();

    state->context = eglCreateContext(state->display, config, EGL_NO_CONTEXT, context_attributes);
    assert(state->context != EGL_NO_CONTEXT);
    check();

    success = graphics_get_display_size(0, &state->screen_width, &state->screen_height);
    assert(success >= 0);

    dst_rect.x = 0;
    dst_rect.y = 0;
    dst_rect.width = state->screen_width;
    dst_rect.height = state->screen_height;

    src_rect.x = 0;
    src_rect.y = 0;
    src_rect.width = state->screen_width << 16;
    src_rect.height = state->screen_height << 16;

    dispman_display = vc_dispmanx_display_open(0);
    dispman_update = vc_dispmanx_update_start(0);
    dispman_element = vc_dispmanx_element_add(dispman_update, dispman_display,
        0, &dst_rect, 0, &src_rect, DISPMANX_PROTECTION_NONE, 0, 0, 0);

    native_window.element = dispman_element;
    native_window.width = state->screen_width;
    native_window.height = state->screen_height;
    vc_dispmanx_update_submit_sync(dispman_update);
    check();

    state->surface = eglCreateWindowSurface(state->display, config, &native_window, NULL);
    assert(state->surface != EGL_NO_SURFACE);
    check();

    result = eglMakeCurrent(state->display, state->surface, state->surface, state->context);
    assert(EGL_FALSE != result);
    check();

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    check();
}

static void init_shaders(PTRI_STATE_T state)
{
    static const GLfloat vertex_data[] =
    {
        -1.0, -1.0,  1.0,  1.0,
         1.0, -1.0,  1.0,  1.0,
         1.0,  1.0,  1.0,  1.0,
        -1.0,  1.0,  1.0,  1.0
    };

    if (state->verbose) { printf("[INFO]: %s\n", "Compiling Vertex Shader..."); }
    state->vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(state->vertex_shader, 1, &vertex_shader_source, 0);
    glCompileShader(state->vertex_shader);
    check(); if (state->verbose) { show_log(state->vertex_shader); }

    if (state->verbose) { printf("[INFO]: %s\n", "Compiling Fragment Shader..."); }
    state->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(state->fragment_shader, 1, &fragment_shader_source, 0);
    glCompileShader(state->fragment_shader);
    check(); if (state->verbose) { show_log(state->fragment_shader); }

    if (state->verbose) { printf("[INFO]: %s\n", "Linking Shader Program..."); }
    state->program = glCreateProgram();
    glAttachShader(state->program, state->vertex_shader);
    glAttachShader(state->program, state->fragment_shader);
    glLinkProgram(state->program);
    check(); if (state->verbose) { show_program_log(state->program); }

    state->attribute_vertex = glGetAttribLocation(state->program, "vertex");
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glGenBuffers(1, &state->vertex_buffer);
    check();

    glGenTextures(1, &state->texture); check();
    glBindTexture(GL_TEXTURE_2D, state->texture); check();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, state->screen_width, state->screen_height,
                                0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 0); check();
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    check();
    glGenFramebuffers(1, &state->framebuffer); check();
    glBindFramebuffer(GL_FRAMEBUFFER, state->framebuffer); check();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
        state->texture, 0); check();
    glBindFramebuffer(GL_FRAMEBUFFER, 0); check();
    glViewport(0, 0, state->screen_width, state->screen_height); check();
    glBindBuffer(GL_ARRAY_BUFFER, state->vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);
    glVertexAttribPointer(state->attribute_vertex, 4, GL_FLOAT, 0, 16, 0);
    glEnableVertexAttribArray(state->attribute_vertex);
    check();
}

static void draw_triangle(PTRI_STATE_T state)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); check();
    glBindBuffer(GL_ARRAY_BUFFER, state->vertex_buffer); check();
    glUseProgram(state->program); check();
    glBindTexture(GL_TEXTURE_2D, state->texture); check();
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4); check();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glFlush();
    glFinish(); check();
    eglSwapBuffers(state->display, state->surface); check();
}

static int get_mouse(PTRI_STATE_T state, int* outx, int* outy)
{
    static int fd = -1;
    const int width = state->screen_width;
    const int height = state->screen_height;
    static int x = 800;
    static int y = 400;
    const int XSIGN = 1 << 4;
    const int YSIGN = 1 << 5;

    if (fd < 0) { fd = open("/dev/input/mouse0", O_RDONLY | O_NONBLOCK); }
    if (fd >= 0)
    {
        struct { char buttons, dx, dy; } m;
        while (1)
        {
            int bytes = read(fd, &m, sizeof(m));
            if (bytes < (int)(sizeof(m))) { goto _exit; }
            if (m.buttons & 8) { break; }
            read(fd, &m, 1);
        }
        if (m.buttons & 3) { return m.buttons & 3; }
        x += m.dx;
        y += m.dy;
        if (m.buttons & XSIGN) { x -= 256; }
        if (m.buttons & YSIGN) { y -= 256; }
        if (x < 0) { x = 0; }
        if (y < 0) { y = 0; }
        if (x > width) { x = width; }
        if (y > height) { y = height; }
    }
_exit:
    if (outx) { *outx = x; }
    if (outy) { *outy = y; }
    return 0;
}
