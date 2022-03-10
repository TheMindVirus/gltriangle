#include "main.h"

#include "shader.v"
#include "shader.f"

#define M_2X_PI   (2.0 * M_PI)
#define FOV       ((60.0 * M_PI) / 180.0)

static void show_log(GLint shader);
static void show_program_log(GLint program);
static void init_ogl(PCUBE_STATE_T state);
static void init_shaders(PCUBE_STATE_T state);
static void draw_cube(PCUBE_STATE_T state);
static int get_mouse(PCUBE_STATE_T state, int* outx, int* outy);

static GLfloat* m4init(GLfloat width, GLfloat height);
static GLfloat* m4new(size_t size);
static void m4free(GLfloat* ptr);

static GLfloat* m4normalise(GLfloat* src);
static GLfloat* m4subtractVectors(GLfloat* a, GLfloat* b);
static GLfloat* m4cross(GLfloat* a, GLfloat* b);

static GLfloat* m4lookAt(GLfloat* pos, GLfloat* target, GLfloat* up);
static GLfloat* m4multiply(GLfloat* a, GLfloat* b);
static GLfloat* m4inverse(GLfloat* src);
static GLfloat* m4perspective(GLfloat fov, GLfloat aspect, GLfloat near, GLfloat far);
static GLfloat* m4xRotate(GLfloat* src, float rotX);
static GLfloat* m4yRotate(GLfloat* src, float rotY);

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
        draw_cube(state);
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

static void init_ogl(PCUBE_STATE_T state)
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
    src_rect.width = state->screen_width; // << 16;
    src_rect.height = state->screen_height; // << 16;

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

    //Volumetric Settings
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_FRONT);
    //glDisable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    check();
}

static void init_shaders(PCUBE_STATE_T state)
{
    /*static const GLfloat vertex_data[] =
    {
        -1.0, -1.0,  1.0,  1.0,
         1.0, -1.0,  1.0,  1.0,
         1.0,  1.0,  1.0,  1.0,
        -1.0,  1.0,  1.0,  1.0,
    };*/
    static const GLfloat vertex_data[] =
    {
        -0.5, -0.5, -0.5,  1.0,
        -0.5,  0.5, -0.5,  1.0,
         0.5, -0.5, -0.5,  1.0,
        -0.5,  0.5, -0.5,  1.0,
         0.5,  0.5, -0.5,  1.0,
         0.5, -0.5, -0.5,  1.0,

        -0.5, -0.5,  0.5,  1.0,
         0.5, -0.5,  0.5,  1.0,
        -0.5,  0.5,  0.5,  1.0,
        -0.5,  0.5,  0.5,  1.0,
         0.5, -0.5,  0.5,  1.0,
         0.5,  0.5,  0.5,  1.0,

        -0.5, -0.5, -0.5,  1.0,
         0.5, -0.5, -0.5,  1.0,
        -0.5, -0.5,  0.5,  1.0,
        -0.5, -0.5,  0.5,  1.0,
         0.5, -0.5, -0.5,  1.0,
         0.5, -0.5,  0.5,  1.0,

        -0.5,  0.5, -0.5,  1.0,
        -0.5,  0.5,  0.5,  1.0,
         0.5,  0.5, -0.5,  1.0,
        -0.5,  0.5,  0.5,  1.0,
         0.5,  0.5,  0.5,  1.0,
         0.5,  0.5, -0.5,  1.0,

        -0.5, -0.5, -0.5,  1.0,
        -0.5, -0.5,  0.5,  1.0,
        -0.5,  0.5, -0.5,  1.0,
        -0.5, -0.5,  0.5,  1.0,
        -0.5,  0.5,  0.5,  1.0,
        -0.5,  0.5, -0.5,  1.0,

         0.5, -0.5, -0.5,  1.0,
         0.5,  0.5, -0.5,  1.0,
         0.5, -0.5,  0.5,  1.0,
         0.5, -0.5,  0.5,  1.0,
         0.5,  0.5, -0.5,  1.0,
         0.5,  0.5,  0.5,  1.0,
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

    state->size_uniform = glGetUniformLocation(state->program, "size");
    state->frames_uniform = glGetUniformLocation(state->program, "frames");
    state->frame_uniform = glGetUniformLocation(state->program, "frame");
    state->steps_uniform = glGetUniformLocation(state->program, "steps");
    state->matrix_uniform = glGetUniformLocation(state->program, "matrix");
    state->texture_uniform = glGetUniformLocation(state->program, "_Texture");
    check();

    glGenTextures(1, &state->texture); check();
    glBindTexture(GL_TEXTURE_2D, state->texture); check();
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, state->screen_width, state->screen_height,
//                                0, GL_RGBA, GL_UNSIGNED_BYTE, 0); check();
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

static void draw_cube(PCUBE_STATE_T state)
{
    state->size = 2;
    state->frames = 1;
    state->frame = 0;
    state->steps = 10;

    state->rotX -= 0.04; if (state->rotX > M_2X_PI) { state->rotX -= M_2X_PI; } if (state->rotX < M_2X_PI) { state->rotX += M_2X_PI; }
    state->rotY -= 0.07; if (state->rotX > M_2X_PI) { state->rotX -= M_2X_PI; } if (state->rotY < M_2X_PI) { state->rotY += M_2X_PI; }

    state->camera = m4init(state->screen_width, state->screen_height);
    state->camera = m4xRotate(state->camera, state->rotX);
    state->camera = m4yRotate(state->camera, state->rotY);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); check();
    glBindBuffer(GL_ARRAY_BUFFER, state->vertex_buffer); check();
    glUseProgram(state->program); check();
    glUniform1f(state->size_uniform, state->size);
    glUniform1f(state->frames_uniform, state->frames);
    glUniform1f(state->frame_uniform, state->frame);
    glUniform1f(state->steps_uniform, state->steps);
    glUniformMatrix4fv(state->matrix_uniform, 16, false, state->camera);
    glBindTexture(GL_TEXTURE_2D, state->texture); check();
    glDrawArrays(GL_TRIANGLES, 0, 36); check();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glFlush();
    glFinish(); check();
    eglSwapBuffers(state->display, state->surface); check();
    m4free(state->camera);
}

static int get_mouse(PCUBE_STATE_T state, int* outx, int* outy)
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

static GLfloat* m4init(GLfloat width, GLfloat height)
{
    GLfloat* pos = m4new(3);
    GLfloat* target = m4new(3);
    GLfloat* up = m4new(3);
    pos[2] = 2;
    up[1] = 1;
    GLfloat* dst = m4lookAt(pos, target, up);
    m4free(pos);
    m4free(target);
    m4free(up);

    GLfloat* persp = m4perspective(FOV, width / height, 1, 2000);
    GLfloat* inv = m4inverse(dst);
    GLfloat* tmp = dst; dst = m4multiply(persp, inv);
    m4free(persp);
    m4free(inv);
    m4free(tmp);

    return dst;
}

static GLfloat* m4new(size_t size)
{
    GLfloat* dst = (GLfloat*)calloc(size, sizeof(GLfloat));
    for (size_t i = 0; i < size; ++i) { dst[i] = 0.0; }
    return dst;
}
static void m4free(GLfloat* ptr) { free(ptr); }

static GLfloat* m4normalise(GLfloat* src)
{
    GLfloat* dst = m4new(3);
    GLfloat length = sqrt(pow(src[0], 2) + pow(src[1], 2) + pow(src[2], 2));
    if (length > 0.000001)
    {
        dst[0] = src[0] / length;
        dst[1] = src[1] / length;
        dst[2] = src[2] / length;
    }
    return dst;
}

static GLfloat* m4subtractVectors(GLfloat* a, GLfloat* b)
{
    GLfloat* dst = m4new(3);
    dst[0] = a[0] - b[0];
    dst[1] = a[1] - b[1];
    dst[2] = a[2] - b[2];
    return dst;
}

static GLfloat* m4cross(GLfloat* a, GLfloat* b)
{
    GLfloat* dst = m4new(3);
    dst[0] = (a[1] * b[2]) - (a[2] * b[1]);
    dst[1] = (a[2] * b[0]) - (a[0] * b[2]);
    dst[2] = (a[0] * b[1]) - (a[1] * b[0]);
    return dst;
}

static GLfloat* m4lookAt(GLfloat* pos, GLfloat* target, GLfloat* up)
{
    GLfloat* dst = m4new(16);
    GLfloat* tmp = m4subtractVectors(pos, target);
    GLfloat* za = m4normalise(tmp); m4free(tmp);
    tmp = m4cross(up, za); GLfloat* xa = m4normalise(tmp); m4free(tmp);
    tmp = m4cross(za, xa); GLfloat* ya = m4normalise(tmp); m4free(tmp);
    dst[ 0] = xa[0];
    dst[ 1] = xa[1];
    dst[ 2] = xa[2];
    dst[ 3] = 0;
    dst[ 4] = ya[0];
    dst[ 5] = ya[1];
    dst[ 6] = ya[2];
    dst[ 7] = 0;
    dst[ 8] = za[0];
    dst[ 9] = za[1];
    dst[10] = za[2];
    dst[11] = 0;
    dst[12] = pos[0];
    dst[13] = pos[1];
    dst[14] = pos[2];
    dst[15] = 1;
    m4free(xa);
    m4free(ya);
    m4free(za);
    return dst;
}

static GLfloat* m4multiply(GLfloat* a, GLfloat* b)
{
    GLfloat* dst = m4new(16);
    dst[ 0] = (a[ 0] * b[ 0]) + (a[ 4] * b[ 1]) + (a[ 8] * b[ 2]) + (a[12] * b[ 3]);
    dst[ 1] = (a[ 1] * b[ 0]) + (a[ 5] * b[ 1]) + (a[ 9] * b[ 2]) + (a[13] * b[ 3]);
    dst[ 2] = (a[ 2] * b[ 0]) + (a[ 6] * b[ 1]) + (a[10] * b[ 2]) + (a[14] * b[ 3]);
    dst[ 3] = (a[ 3] * b[ 0]) + (a[ 7] * b[ 1]) + (a[11] * b[ 2]) + (a[15] * b[ 3]);
    dst[ 4] = (a[ 0] * b[ 4]) + (a[ 4] * b[ 5]) + (a[ 8] * b[ 6]) + (a[12] * b[ 7]);
    dst[ 5] = (a[ 1] * b[ 4]) + (a[ 5] * b[ 5]) + (a[ 9] * b[ 6]) + (a[13] * b[ 7]);
    dst[ 6] = (a[ 2] * b[ 4]) + (a[ 6] * b[ 5]) + (a[10] * b[ 6]) + (a[14] * b[ 7]);
    dst[ 7] = (a[ 3] * b[ 4]) + (a[ 7] * b[ 5]) + (a[11] * b[ 6]) + (a[15] * b[ 7]);
    dst[ 8] = (a[ 0] * b[ 8]) + (a[ 4] * b[ 9]) + (a[ 8] * b[10]) + (a[12] * b[11]);
    dst[ 9] = (a[ 1] * b[ 8]) + (a[ 5] * b[ 9]) + (a[ 9] * b[10]) + (a[13] * b[11]);
    dst[10] = (a[ 2] * b[ 8]) + (a[ 6] * b[ 9]) + (a[10] * b[10]) + (a[14] * b[11]);
    dst[11] = (a[ 3] * b[ 8]) + (a[ 7] * b[ 9]) + (a[11] * b[10]) + (a[15] * b[11]);
    dst[12] = (a[ 0] * b[12]) + (a[ 4] * b[13]) + (a[ 8] * b[14]) + (a[12] * b[15]);
    dst[13] = (a[ 1] * b[12]) + (a[ 5] * b[13]) + (a[ 9] * b[14]) + (a[13] * b[15]);
    dst[14] = (a[ 2] * b[12]) + (a[ 6] * b[13]) + (a[10] * b[14]) + (a[14] * b[15]);
    dst[15] = (a[ 3] * b[12]) + (a[ 7] * b[13]) + (a[11] * b[14]) + (a[15] * b[15]);
    return dst;
}

static GLfloat* m4inverse(GLfloat* src)
{
    GLfloat* dst = m4new(16);
    GLfloat* tmp = m4new(24);
    tmp[ 0] = src[10] * src[15];
    tmp[ 1] = src[14] * src[11];
    tmp[ 2] = src[ 6] * src[15];
    tmp[ 3] = src[14] * src[ 7];
    tmp[ 4] = src[ 6] * src[11];
    tmp[ 5] = src[10] * src[ 7];
    tmp[ 6] = src[ 2] * src[15];
    tmp[ 7] = src[14] * src[ 3];
    tmp[ 8] = src[ 2] * src[11];
    tmp[ 9] = src[10] * src[ 3];
    tmp[10] = src[ 2] * src[ 7];
    tmp[11] = src[ 6] * src[ 3];
    tmp[12] = src[ 8] * src[13];
    tmp[13] = src[12] * src[ 9];
    tmp[14] = src[ 4] * src[13];
    tmp[15] = src[12] * src[ 5];
    tmp[16] = src[ 4] * src[ 9];
    tmp[17] = src[ 8] * src[ 5];
    tmp[18] = src[ 0] * src[13];
    tmp[19] = src[12] * src[ 1];
    tmp[20] = src[ 0] * src[ 9];
    tmp[21] = src[ 8] * src[ 1];
    tmp[22] = src[ 0] * src[ 5];
    tmp[23] = src[ 4] * src[ 1];
    dst[ 0] = (((tmp[ 0] * src[ 5]) + (tmp[ 3] * src[ 9]) + (tmp[ 4] * src[13])) -
               ((tmp[ 1] * src[ 5]) + (tmp[ 2] * src[ 9]) + (tmp[ 5] * src[13])));
    dst[ 1] = (((tmp[ 1] * src[ 1]) + (tmp[ 6] * src[ 9]) + (tmp[ 9] * src[13])) -
               ((tmp[ 0] * src[ 1]) + (tmp[ 7] * src[ 9]) + (tmp[ 8] * src[13])));
    dst[ 2] = (((tmp[ 2] * src[ 1]) + (tmp[ 7] * src[ 5]) + (tmp[10] * src[13])) -
               ((tmp[ 3] * src[ 1]) + (tmp[ 6] * src[ 5]) + (tmp[11] * src[13])));
    dst[ 3] = (((tmp[ 5] * src[ 1]) + (tmp[ 8] * src[ 5]) + (tmp[11] * src[13])) -
               ((tmp[ 4] * src[ 1]) + (tmp[ 9] * src[ 5]) + (tmp[10] * src[13])));
    GLfloat d = 1.0 / ((src[ 0] * dst[ 0]) + (src[ 4] * dst[ 1]) + (src[ 8] * dst[ 2]) + (src[12] * dst[ 3]));
    dst[ 0] *= d;
    dst[ 1] *= d;
    dst[ 2] *= d;
    dst[ 3] *= d;
    dst[ 4] = d * (((tmp[ 1] * src[ 4]) + (tmp[ 2] * src[ 8]) + (tmp[ 5] * src[12])) -
                   ((tmp[ 0] * src[ 4]) + (tmp[ 3] * src[ 8]) + (tmp[ 4] * src[12])));
    dst[ 5] = d * (((tmp[ 0] * src[ 0]) + (tmp[ 7] * src[ 8]) + (tmp[ 8] * src[12])) -
                   ((tmp[ 1] * src[ 0]) + (tmp[ 6] * src[ 8]) + (tmp[ 9] * src[12])));
    dst[ 6] = d * (((tmp[ 3] * src[ 0]) + (tmp[ 6] * src[ 4]) + (tmp[11] * src[12])) -
                   ((tmp[ 2] * src[ 0]) + (tmp[ 7] * src[ 4]) + (tmp[10] * src[12])));
    dst[ 7] = d * (((tmp[ 4] * src[ 0]) + (tmp[ 9] * src[ 4]) + (tmp[10] * src[ 8])) -
                   ((tmp[ 5] * src[ 0]) + (tmp[ 8] * src[ 4]) + (tmp[11] * src[ 8])));
    dst[ 8] = d * (((tmp[12] * src[ 7]) + (tmp[15] * src[11]) + (tmp[16] * src[15])) -
                   ((tmp[13] * src[ 7]) + (tmp[14] * src[11]) + (tmp[17] * src[15])));
    dst[ 9] = d * (((tmp[13] * src[ 3]) + (tmp[18] * src[11]) + (tmp[21] * src[15])) -
                   ((tmp[12] * src[ 3]) + (tmp[19] * src[11]) + (tmp[20] * src[15])));
    dst[10] = d * (((tmp[14] * src[ 3]) + (tmp[19] * src[ 7]) + (tmp[22] * src[15])) -
                   ((tmp[15] * src[ 3]) + (tmp[18] * src[ 7]) + (tmp[23] * src[15])));
    dst[11] = d * (((tmp[17] * src[ 3]) + (tmp[20] * src[ 7]) + (tmp[23] * src[11])) -
                   ((tmp[16] * src[ 3]) + (tmp[21] * src[ 7]) + (tmp[22] * src[11])));
    dst[12] = d * (((tmp[14] * src[10]) + (tmp[17] * src[14]) + (tmp[13] * src[ 6])) -
                   ((tmp[16] * src[14]) + (tmp[12] * src[ 6]) + (tmp[15] * src[10])));
    dst[13] = d * (((tmp[20] * src[14]) + (tmp[12] * src[ 2]) + (tmp[19] * src[10])) -
                   ((tmp[18] * src[10]) + (tmp[21] * src[14]) + (tmp[13] * src[ 2])));
    dst[14] = d * (((tmp[18] * src[ 6]) + (tmp[23] * src[14]) + (tmp[15] * src[ 2])) -
                   ((tmp[22] * src[14]) + (tmp[14] * src[ 2]) + (tmp[19] * src[ 6])));
    dst[15] = d * (((tmp[22] * src[10]) + (tmp[16] * src[ 2]) + (tmp[21] * src[ 6])) -
                   ((tmp[20] * src[ 6]) + (tmp[23] * src[10]) + (tmp[17] * src[ 2])));
    m4free(tmp);
    return dst;
}

static GLfloat* m4perspective(GLfloat fov, GLfloat aspect, GLfloat near, GLfloat far)
{
    GLfloat* dst = m4new(16);
    GLfloat f = tan((M_PI * 0.5) - (0.5 * fov));
    GLfloat inv = 1.0 / (near - far);
    dst[ 0] = f / aspect;
    dst[ 5] = f;
    dst[10] = (near + far) * inv;
    dst[11] = -1;
    dst[14] = near * far * inv * 2;
    return dst;
}

static GLfloat* m4xRotate(GLfloat* src, float rotX)
{
    GLfloat* dst = m4new(16);
    GLfloat c = cos(rotX);
    GLfloat s = sin(rotX);
    dst[ 0] = src[ 0];
    dst[ 1] = src[ 1];
    dst[ 2] = src[ 2];
    dst[ 3] = src[ 3];
    dst[ 4] = (c * src[ 4]) + (s * src[ 8]);
    dst[ 5] = (c * src[ 5]) + (s * src[ 9]);
    dst[ 6] = (c * src[ 6]) + (s * src[10]);
    dst[ 7] = (c * src[ 7]) + (s * src[11]);
    dst[ 8] = (c * src[ 8]) - (s * src[ 4]);
    dst[ 9] = (c * src[ 9]) - (s * src[ 5]);
    dst[10] = (c * src[10]) - (s * src[ 6]);
    dst[11] = (c * src[11]) - (s * src[ 7]);
    dst[12] = src[12];
    dst[13] = src[13];
    dst[14] = src[14];
    dst[15] = src[15];
    m4free(src);
    return dst;
}

static GLfloat* m4yRotate(GLfloat* src, float rotY)
{
    GLfloat* dst = m4new(16);
    GLfloat c = cos(rotY);
    GLfloat s = sin(rotY);
    dst[ 0] = (c * src[ 0]) - (s * src[ 8]);
    dst[ 1] = (c * src[ 1]) - (s * src[ 9]);
    dst[ 2] = (c * src[ 2]) - (s * src[10]);
    dst[ 3] = (c * src[ 3]) - (s * src[11]);
    dst[ 4] = src[ 4];
    dst[ 5] = src[ 5];
    dst[ 6] = src[ 6];
    dst[ 7] = src[ 7];
    dst[ 8] = (c * src[ 8]) + (s * src[ 0]);
    dst[ 9] = (c * src[ 9]) + (s * src[ 1]);
    dst[10] = (c * src[10]) + (s * src[ 2]);
    dst[11] = (c * src[11]) + (s * src[ 3]);
    dst[12] = src[12];
    dst[13] = src[13];
    dst[14] = src[14];
    dst[15] = src[15];
    m4free(src);
    return dst;
}
