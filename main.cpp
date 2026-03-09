#include <chrono>
#include <cmath>
#include <complex>
#include <iostream>
#include <thread>

#include <glad/gl.h>

#include "GLFW/glfw3.h"

typedef unsigned char uc;

#define MAX_ITERS 100
#define THRESHOLD 2
#define WIDTH 1080
#define HEIGHT 1080

const uc gradient_col1[3] = {0, 0, 40};
const uc gradient_col2[3] = {200, 50, 0};

inline void glfw_error_callback(int error, const char *description) { std::cout << description << std::endl; }

inline int simulate_point(double cx, double cy) {
	int iters = 0;

	double zx = 0, zy = 0, zx2 = 0, zy2 = 0;

	while (zx2 + zy2 < THRESHOLD * THRESHOLD && iters < MAX_ITERS) {

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

inline int *simulate(double cx, double cy) {
	int *array = (int *)malloc(HEIGHT * WIDTH * sizeof(int));

	for (int i = 0; i < HEIGHT; i++)
		for (int j = 0; j < WIDTH; j++) {
			array[i * WIDTH + j] = simulate_point(cx + 0.25 * i / HEIGHT, cy + 0.25 * j / WIDTH);
		}

	return array;
}

inline uc *get_gradient(int iters, int maxiter) {

	uc *res = (uc *)malloc(3 * sizeof(uc));

	double part = (double)iters / maxiter;

	for (int i = 0; i < 3; i++) {
		res[i] = gradient_col1[i] + part * (gradient_col2[i] - gradient_col1[i]);
	}

	return res;
}

inline uc *apply_color(int *array) {
	uc *color_array = (uc *)malloc(WIDTH * HEIGHT * 3 * sizeof(uc));

	int maxiter = 0;

	for (int i = 0; i < HEIGHT; i++) {
		for (int j = 0; j < WIDTH; j++) {
			maxiter = std::max(maxiter, array[i * WIDTH + j]);
		}
	}

	for (int i = 0; i < HEIGHT; i++) {
		for (int j = 0; j < WIDTH; j++) {
			if (array[i * WIDTH + j] == 0) {
				for (int o = 0; o < 3; o++) {
					color_array[i * WIDTH * 3 + j * 3 + o] = 0;
				}
				continue;
			}

			uc *res = get_gradient(array[i * WIDTH + j], maxiter);
			for (int o = 0; o < 3; o++) {
				color_array[i * WIDTH * 3 + j * 3 + o] = res[o];
			}
			free(res);
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

	bool need_redraw = true;

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.f, 0.f, 0.f, 1.f);

		if (need_redraw) {

			// auto start = std::chrono::high_resolution_clock::now();

			int *array = simulate(-0.5, -0.8);
			uc *color_array = apply_color(array);

			// auto end = std::chrono::high_resolution_clock::now();
			// std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;

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