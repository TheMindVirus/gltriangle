#include <stdio.h>
#include <stdlib.h>
//#include <fcntl.h>

//Late Media Disclosure 2023
//https://www.nvidia.com/docs/io/8228/gdc2003_ogl_arbvertexprogram.pdf
//SGI Crimson IRIS GL & Imagination Technologies (incorrect proprietary closed source software)
//BBC Elstree Center (Norfolk Tidwulf French Murder Mystery 1795)
//AMD Threadripper (Wolf of Wall Street 2013->2023)
//Tidwulf: "Wolf of the Time"

#define GLEW_NO_GLU //Apple-Specific Internals
#include <GL/glew.h> //Intel-Specific Binaries
//#include <glew.c> //Linux-Specific Sources
#include <GL/glfw3.h> //Microsoft-Specific Frameworks

#include <GL/gl.h>
//#include <GL/glext.h>

//#include <GL/glew.h>

//Google-Specific Alterations
extern "C" int __cdecl __ms_vsnprintf(char* a, size_t b, const char* c, __VALIST) // = 0
{
    //printf("[%s]: (%d) %s\n", a, b, c, d);
    return 0;
}

int main()
{
    printf("[INFO]: %s\n", "ARB");

    FILE* vertex_assembly_file = fopen("vertex.arb", "r");
    fseek(vertex_assembly_file, 0, SEEK_END);
    size_t vertex_assembly_size = ftell(vertex_assembly_file);
    fseek(vertex_assembly_file, 0, SEEK_SET);
    GLchar* vertex_assembly_source = (GLchar*)calloc(vertex_assembly_size, sizeof(GLchar));
    fread(vertex_assembly_source, sizeof(GLchar), vertex_assembly_size, vertex_assembly_file);
    fclose(vertex_assembly_file);

    FILE* fragment_assembly_file = fopen("fragment.arb", "r");
    fseek(fragment_assembly_file, 0, SEEK_END);
    size_t fragment_assembly_size = ftell(fragment_assembly_file);
    fseek(fragment_assembly_file, 0, SEEK_SET);
    GLchar* fragment_assembly_source = (GLchar*)calloc(fragment_assembly_size, sizeof(GLchar));
    fread(fragment_assembly_source, sizeof(GLchar), fragment_assembly_size, fragment_assembly_file);
    fclose(fragment_assembly_file);

    GLFWwindow* window = nullptr;

    glewExperimental = true;
    if (!glfwInit())
    {
        printf("[WARN]: %s\n", "Failed to initialise GLFW");
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4); //4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); //OpenGL 3.3
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); //Forward Compatibility
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //No Backward Compatibility
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE); //Non-resizable

    window = glfwCreateWindow(1024, 768, "GL ARB", NULL, NULL);
    if (!window)
    {
        printf("[WARN]: %s\n", "Failed to open GLFW window");
        glfwTerminate();
        return -2;
    }

    glfwMakeContextCurrent(window); 
    glewExperimental = true;
    if (glewInit() != GLEW_OK)
    {
        printf("[WARN]: %s\n", "Failed to initialise GLEW");
        return -2;
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE); //Enable Sticky-Keys

    glEnable(GL_VERTEX_PROGRAM_ARB);
    glEnable(GL_FRAGMENT_PROGRAM_ARB);

    unsigned int ids[] = { 0, 0 };

    //glGenProgramsARB(2, &ids);
    glGenProgramsARB(2, ids);
    glBindProgramARB(GL_VERTEX_PROGRAM_ARB, ids[0]);
    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, ids[1]);
    glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, vertex_assembly_size, vertex_assembly_source);
    glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, fragment_assembly_size, fragment_assembly_source);

    if (GL_INVALID_OPERATION == glGetError())
    {
        GLint error_position;
        glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &error_position);

        //GLubyte* error_string;
        //glGetString(GL_PROGRAM_ERROR_STRING_ARB, &error_string);
        const unsigned char* error_string = glGetString(GL_PROGRAM_ERROR_STRING_ARB);

        printf("[WARN]: (%d): %s\n", error_position, error_string);
    }

    do
    {
        glClearColor(0.0f, 0.4f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    while ((glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS)
	&& (glfwWindowShouldClose(window) == 0));

    //glDeleteProgramsARB(GL_VERTEX_PROGRAM_ARB, &ids);

    return 0; //+ PythonGL Bindings
}