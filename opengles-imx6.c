#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglvivante.h>

struct egl_device {
	EGLNativeDisplayType display_type;
	EGLDisplay display;
	const EGLint *config_attributes;
	EGLConfig config;
	EGLNativeWindowType window;
	EGLSurface surface;
	const EGLint *context_attributes;
	EGLContext context;
};

int egl_initialize(struct egl_device *device)
{
	device->display_type = (EGLNativeDisplayType)fbGetDisplayByIndex(0);
	device->display = eglGetDisplay(device->display_type);

	eglInitialize(device->display, NULL, NULL);
	eglBindAPI(EGL_OPENGL_ES_API);

	static const EGLint config_attributes[] = {
		EGL_SAMPLES,			0,
		EGL_RED_SIZE,			8,
		EGL_GREEN_SIZE,			8,
		EGL_BLUE_SIZE,			8,
		EGL_ALPHA_SIZE,			EGL_DONT_CARE,
		EGL_DEPTH_SIZE,			0,
		EGL_SURFACE_TYPE,		EGL_WINDOW_BIT,
		EGL_NONE
	};

	EGLint config_count = 0;
	eglChooseConfig(device->display, config_attributes, &device->config, 1, &config_count);

	device->config_attributes = config_attributes;
	device->window = fbCreateWindow(device->display_type, 0, 0, 0, 0);
	device->surface = eglCreateWindowSurface(device->display, device->config, device->window, NULL);

	static const EGLint context_attributes[] = {
		EGL_CONTEXT_CLIENT_VERSION,		2,
		EGL_NONE
	};

	device->context = eglCreateContext(device->display, device->config, EGL_NO_CONTEXT, context_attributes);
	device->context_attributes = context_attributes;

	eglMakeCurrent(device->display, device->surface, device->surface, device->context);

	return 0;
}

int egl_deinitialize(struct egl_device *device)
{
	eglMakeCurrent(device->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

	eglDestroyContext(device->display, device->context);
	device->context = (EGLContext)0;

	eglDestroySurface(device->display, device->surface);
	device->surface = (EGLSurface)0;

	fbDestroyWindow(device->window);
	device->window = (EGLNativeWindowType)0;

	eglTerminate(device->display);
	device->display = (EGLDisplay)0;

	eglReleaseThread();

	return 0;
}

int main(int argc, char *argv[])
{
	int display_width;
	int display_height;
	struct egl_device device
		= { 0 };

	egl_initialize(&device);
	fbGetDisplayGeometry(device.display_type, &display_width, &display_height);

	glClearColor(255.0f / 255.0f, 0.0 / 255.0f, 0.0 / 255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	eglSwapBuffers(device.display, device.surface);

	egl_deinitialize(&device);

	return 0;
}
