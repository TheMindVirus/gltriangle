#include "main.hxx"

//http://www.opengl-tutorial.org/beginners-tutorials/tutorial-1-opening-a-window/

int main()
{
	GLuint program = 0;
	GLFWwindow* window = NULL;

	glewExperimental = true;
	if (!glfwInit()) //Initialise GLFW
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

	window = glfwCreateWindow(1024, 768, "GL Triangle", NULL, NULL);
	if (!window)
	{
		printf("[WARN]: %s\n", "Failed to open GLFW window");
		glfwTerminate();
		goto stop;
	}
	
	glfwMakeContextCurrent(window); 
	glewExperimental = true;
	if (glewInit() != GLEW_OK) //Initialise GLEW
	{
		printf("[WARN]: %s\n", "Failed to initialise GLEW");
		goto stop;
	}

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE); //Enable Sticky-Keys

	program = LoadShaders("shader.vertex", "shader.fragment");
	if (!program)
	{
		printf("[WARN]: %s\n", "Failed to load Shaders");
		goto stop;
	}

	do
	{
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		GLuint VertexArray;
		glGenVertexArrays(1, &VertexArray);
		glBindVertexArray(VertexArray);

		GLuint VertexBuffer;
		glGenBuffers(1, &VertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_triangle), g_triangle, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glUseProgram(program);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glDisableVertexAttribArray(0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	} 
	while ((glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS)
	&& (glfwWindowShouldClose(window) == 0));
	
	goto stop;

stop:
	printf("Press any key to continue...");
	char any = getc(stdin);
	return 0;
}