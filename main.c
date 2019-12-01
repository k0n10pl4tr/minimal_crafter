#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <EGL/egl.h>
#include <time.h>

#include "glad.h"
#include "util.h"
#include "rendering.h"

#include <unistd.h>

#define FRAMES_PER_SEC 60.0

static void initWindow();
static void updateWindow();
static void terminateWindow();

Display* dpy;
Window rootWindow, window;
Atom   wmDeleteWindow;

int windowWidth, windowHeight;

unsigned char running = 0;

EGLDisplay eglDisplay;
EGLConfig  eglConfig;
EGLSurface eglSurface;
EGLContext eglContext;
EGLint     eglVersionMajor, eglVersionMinor;
EGLint     eglNumConfigs;

EGLint eglConfigAttr[] = {
	EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
	EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
	EGL_NONE
};

EGLint eglSurfaceAttr[] = {
	EGL_NONE
};

EGLint eglContextAttr[] = {
	EGL_CONTEXT_MAJOR_VERSION, 3,
	EGL_CONTEXT_MINOR_VERSION, 3,
	EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
	EGL_NONE
};

static void
initWindow()
{
	eglBindAPI(EGL_OPENGL_API);
	dpy = XOpenDisplay(NULL);
	if(!dpy) {
		printf("Could not init X11\n");
		exit(-1);
	}
	rootWindow = DefaultRootWindow(dpy);
	wmDeleteWindow = XInternAtom(dpy, "WM_DELETE_WINDOW", 0);

	XSetWindowAttributes swa;
	swa.event_mask = KeyPressMask | KeyReleaseMask | StructureNotifyMask;

	windowWidth = 800;
	windowHeight = 600;

	window = XCreateWindow(dpy, rootWindow, 0, 0, windowWidth, windowHeight, 0, CopyFromParent, InputOutput, CopyFromParent, CWEventMask, &swa);
	XSetWMProtocols(dpy, window, &wmDeleteWindow, 1);

	XStoreName(dpy, window, "Hello");
	XMapWindow(dpy, window);

	eglDisplay = eglGetDisplay((NativeDisplayType)dpy);
	if(eglDisplay == EGL_NO_DISPLAY) {
		printf("Could not get EGL display!\n");
		exit(-2);
	}
	eglInitialize(eglDisplay, &eglVersionMajor, &eglVersionMinor);
	printf("EGL Version: %d.%d\n", eglVersionMajor, eglVersionMinor);
	eglChooseConfig(eglDisplay, eglConfigAttr, &eglConfig, 1, &eglNumConfigs);
	eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, (NativeWindowType)window, eglSurfaceAttr);

	if(eglSurface == EGL_NO_SURFACE) {
		printf("Could not create EGL Surface!\n");
		exit(-3);
	}

	eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, NULL);
	eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
	if(!gladLoadGLLoader((GLADloadproc)eglGetProcAddress)) {
		printf("Could not read the opengl functions.\n");
		exit(-2);
	}
	printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
	startClock();
	initRenderingSystem();
}

static void
updateWindow()
{
	eglSwapBuffers(eglDisplay, eglSurface);
	while(XPending(dpy)) {
		XEvent xev;
		XNextEvent(dpy, &xev);
		switch(xev.type) {
		case ClientMessage:
			if(xev.xclient.data.l[0] == wmDeleteWindow)
				running = 0;
			break;
		case ConfigureNotify:
			if(windowWidth != xev.xconfigure.width || windowHeight != xev.xconfigure.height) {
				windowWidth = xev.xconfigure.width;
				windowHeight = xev.xconfigure.height;

				resizeRenderingSystem(windowWidth, windowHeight);
			}
		}
	}
}

static void
terminateWindow()
{
	eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroyContext(eglDisplay, eglContext);
	eglDestroySurface(eglDisplay, eglSurface);
	eglTerminate(eglDisplay);

	XDestroyWindow(dpy, window);
	XCloseDisplay(dpy);
}

int
main(int argc, char *argv[])
{
	initWindow();
	running = 1;
	while(running) {
		double startProcess = getCurrentTimeNano();
		glClearColor(0.2, 0.3, 0.7, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		render();
		
		updateWindow();
		double end = getCurrentTimeNano();
		sleepNanosec((1.0 / FRAMES_PER_SEC) - (end - startProcess));
	}
	terminateWindow();
}
