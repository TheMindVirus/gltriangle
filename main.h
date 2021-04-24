#ifndef _MAIN_H_
#define _MAIN_H_

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <unistd.h>
#include "bcm_host.h"
#include "GLES2/gl2.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"

typedef struct
{
    uint32_t screen_width;
    uint32_t screen_height;

    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;

    GLuint verbose;
    GLuint texture;
    GLuint framebuffer;
    GLuint vertex_buffer;

    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint program;
    GLuint attribute_vertex;

}   TRI_STATE_T, *PTRI_STATE_T;
static TRI_STATE_T _state;
static PTRI_STATE_T state = &_state;

#define check() assert(glGetError() == 0)

#endif//_MAIN_H_
