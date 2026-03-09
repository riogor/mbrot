#include <chrono>
#include <cmath>
#include <complex>
#include <cstring>
#include <iostream>
#include <thread>

#include <glad/gl.h>

#include "GLFW/glfw3.h"

typedef unsigned char uc;

constexpr int MAX_ITERS = 100;
constexpr int MAGNITUDE_THRESHOLD = 100;

constexpr uc gradient_col1[3] = {0, 0, 40};
constexpr uc gradient_col2[3] = {200, 50, 0};

int WIDTH = 1080;
int HEIGHT = 1080;
double SCALE = 0.25;
double ORIGIN_X = -0.5;
double ORIGIN_Y = -0.8;

bool need_redraw = true;

inline void glfw_error_callback(int error, const char *description) { std::cout << description << std::endl; }

inline int simulate_point(const double cx, const double cy) {
	int iters = 0;
	double zx = 0, zy = 0, zx2 = 0, zy2 = 0;

	while (zx2 + zy2 < MAGNITUDE_THRESHOLD * MAGNITUDE_THRESHOLD && iters < MAX_ITERS) {

		zx2 = zx * zx;
		zy2 = zy * zy;

		zy = 2.0 * zx * zy + cy;
		zx = zx2 - zy2 + cx;

		iters++;
	}

	if (iters == MAX_ITERS)
		return 0;

	return iters;
}

inline int *simulate(const double cx, const double cy) {
	int *array = (int *)malloc(HEIGHT * WIDTH * sizeof(int));

	for (int i = 0; i < HEIGHT; i++)
		for (int j = 0; j < WIDTH; j++) {
			array[i * WIDTH + j] = simulate_point(cx + SCALE * i / HEIGHT, cy + SCALE * j / WIDTH);
		}

	return array;
}

inline void apply_color_to_cell(uc *cell, const int iters, const int maxiter) {

	if (iters == 0)
		return;

	double part = (double)iters / maxiter;

	for (int i = 0; i < 3; i++) {
		cell[i] = (uc)(gradient_col1[i] + part * (gradient_col2[i] - gradient_col1[i]));
	}
}

inline uc *apply_color(const int *array) {
	uc *color_array = (uc *)malloc(WIDTH * HEIGHT * 3 * sizeof(uc));
	std::memset(color_array, 0, WIDTH * HEIGHT * 3 * sizeof(uc));

	int maxiter = 0;

	for (int i = 0; i < HEIGHT; i++) {
		for (int j = 0; j < WIDTH; j++) {
			maxiter = std::max(maxiter, array[i * WIDTH + j]);
		}
	}

	for (int i = 0; i < HEIGHT; i++) {
		for (int j = 0; j < WIDTH; j++) {
			apply_color_to_cell(color_array + i * WIDTH * 3 + j * 3, array[i * WIDTH + j], maxiter);
		}
	}

	return color_array;
}

int main() {

	GLFWwindow *window = nullptr;
	GLuint tex = 0;
	GLuint fbo = 0;

	{
		glfwSetErrorCallback(glfw_error_callback);

		if (!glfwInit())
			return 1;

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		window = glfwCreateWindow(WIDTH, HEIGHT, "mbrot", NULL, NULL);
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
	}

	{
		glGenTextures(1, &tex);

		glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	{
		glGenFramebuffers(1, &fbo);

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.f, 0.f, 0.f, 1.f);

		if (need_redraw) {

			auto start = std::chrono::high_resolution_clock::now();

			int *array = simulate(ORIGIN_X, ORIGIN_Y);
			uc *color_array = apply_color(array);

			auto end = std::chrono::high_resolution_clock::now();
			std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;

			glBindTexture(GL_TEXTURE_2D, tex);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, color_array);
			glBindTexture(GL_TEXTURE_2D, 0);

			free(array);
			free(color_array);

			need_redraw = false;
		}

		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, WIDTH, HEIGHT, 0, HEIGHT, WIDTH, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		glfwSwapBuffers(window);
	}

	glfwTerminate();

	return 0;
}