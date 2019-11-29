#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <EGL/egl.h>

static void initWindow();
static void updateWindow();
static void terminateWindow();

Display* dpy;
Window rootWindow, window;
Atom   wmDeleteWindow;

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
	EGL_CONTEXT_MINOR_VERSION, 0,
	EGL_NONE
};

static void
initWindow()
{
	eglBindAPI(EGL_OPENGL_BIT);
	dpy = XOpenDisplay(NULL);
	if(!dpy) {
		printf("Could not init X11\n");
		exit(-1);
	}
	rootWindow = DefaultRootWindow(dpy);
	wmDeleteWindow = XInternAtom(dpy, "WM_DELETE_WINDOW", 0);

	XSetWindowAttributes swa;
	swa.event_mask = KeyPressMask | KeyReleaseMask;
	
	window = XCreateWindow(dpy, rootWindow, 0, 0, 800, 600, 0, CopyFromParent, InputOutput, CopyFromParent, CWEventMask, &swa);

	XStoreName(dpy, window, "Hello");
	XMapWindow(dpy, window);

	eglDisplay = eglGetDisplay((NativeDisplayType)dpy);
	if(eglDisplay == EGL_NO_DISPLAY) {
		printf("Could not get EGL display!\n");
		exit(-2);
	}
	eglInitialize(eglDisplay, &eglVersionMajor, &eglVersionMinor);
	eglChooseConfig(eglDisplay, eglConfigAttr, &eglConfig, 1, &eglNumConfigs);
	eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, (NativeWindowType)window, eglSurfaceAttr);

	if(eglSurface == EGL_NO_SURFACE) {
		printf("Could not create EGL Surface!\n");
		exit(-3);
	}

	eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, NULL);
	eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
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
		
		updateWindow();
	}
	terminateWindow();
}
