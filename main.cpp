#include <chrono>
#include <complex>
#include <iostream>
#include <thread>

#include <glad/gl.h>

#include "GLFW/glfw3.h"

inline void glfw_error_callback(int error, const char *description) { std::cout << description << std::endl; }

#define MAX_ITERS 100
#define THRESHOLD 3
#define WIDTH 1080
#define HEIGHT 1080

inline int simulate_point(std::complex<double> c) {
	int iters = 0;

	std::complex<double> z = 0;

	while (abs(z) < THRESHOLD && iters < MAX_ITERS) {
		z = z * z + c;
		iters++;
	}

	if (iters == MAX_ITERS)
		return 0;

	return iters;
}

inline unsigned char *simulate(std::complex<double> origin) {
	unsigned char *array = (unsigned char *)malloc(HEIGHT * WIDTH * 3 * sizeof(char));

	for (int i = 0; i < HEIGHT; i++)
		for (int j = 0; j < WIDTH; j++) {
			int ret = simulate_point(origin + std::complex<double>((double)i / HEIGHT, (double)j / WIDTH) * 5.0);

			array[i * WIDTH * 3 + j * 3 + 2] = (unsigned char)(ret * 5);
		}

	return array;
}

int main() {
	glfwSetErrorCallback(glfw_error_callback);

	if (!glfwInit())
		return 1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "mbrot", NULL, NULL);
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

	GLuint tex;
	glGenTextures(1, &tex);

	{
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		float borderColor[] = {0.0f, 0.0f, 0.0f, 1.0f};
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	GLuint fbo;
	glGenFramebuffers(1, &fbo);

	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.f, 0.f, 0.f, 1.f);

		auto *array = simulate(std::complex<double>(-3.0, -2.5));
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, array);
		free(array);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, WIDTH, HEIGHT, 0, HEIGHT, WIDTH, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		glfwSwapBuffers(window);
	}

	glfwTerminate();

	return 0;
}