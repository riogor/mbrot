#include <chrono>
#include <iostream>
#include <thread>

#include <glad/gl.h>

#include "GLFW/glfw3.h"

inline void glfw_error_callback(int error, const char *description) { std::cout << description << std::endl; }

int main() {
	glfwSetErrorCallback(glfw_error_callback);

	if (!glfwInit())
		return 1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *window = glfwCreateWindow(1080, 1080, "mbrot", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return 2;
	}
	glfwMakeContextCurrent(window);

	int version = gladLoadGL(glfwGetProcAddress);
	if (version == 0) {
		return 3;
	}

	glfwSwapInterval(1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.f, 0.f, 1.f, 1.f);

		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		glfwSwapBuffers(window);
	}

	glfwTerminate();

	return 0;
}