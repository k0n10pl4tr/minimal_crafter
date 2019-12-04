#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <time.h>

#include "glad.h"
#include <GL/glx.h>

#include "util.h"
#include "rendering.h"
#include "world.h"

#include <unistd.h>

#define FRAMES_PER_SEC 60.0

#define GLX_CONTEXT_MAJOR_VERSION_ARB       0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB       0x2092
#define GLX_CONTEXT_PROFILE_MASK_ARB        0x9126

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

static void initWindow();
static void updateWindow();
static void terminateWindow();

Display* dpy;
Window rootWindow, window;
Atom   wmDeleteWindow;
int screen;

vec3 cameraPosition = { 0.0, 2.0, -10.0 };
float cameraPitch   = M_PI / 2.0;
float cameraYaw     = 0;

char keyLeft = 0,
	 keyRight = 0, 
	 keyUp = 0, 
	 keyDown = 0,
	 keyFront = 0,
	 keyBack = 0,
	 keySpace = 0,
	 keyShift = 0;


int windowWidth, windowHeight;
unsigned char running = 0;

GLXFBConfig fbConfig;
GLXContext  glContext;

static int visualAttribs[] =
{
	GLX_X_RENDERABLE    , True,
	GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
	GLX_RENDER_TYPE     , GLX_RGBA_BIT,
	GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
	GLX_RED_SIZE        , 8,
	GLX_GREEN_SIZE      , 8,
	GLX_BLUE_SIZE       , 8,
	GLX_ALPHA_SIZE      , 8,
	GLX_DEPTH_SIZE      , 24,
	GLX_STENCIL_SIZE    , 8,
	GLX_DOUBLEBUFFER    , True,
	//GLX_SAMPLE_BUFFERS  , 1,
	//GLX_SAMPLES         , 4,
	None
};

static int glxContextAttribs[] = {
	GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
	GLX_CONTEXT_MINOR_VERSION_ARB, 3,
	GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
	None
};

static void
initWindow()
{
	dpy = XOpenDisplay(NULL);
	if(!dpy) {
		printf("Could not init X11\n");
		exit(-1);
	}
	screen = DefaultScreen(dpy);

	int glxMajor, glxMinor;
	glXQueryVersion(dpy, &glxMajor, &glxMinor);
	if(glxMinor < 3)
	{
		printf("Invalid GLX Version\n");
		exit(-3);
	}
	int fbConfigCount;
	GLXFBConfig* fbConfigs = glXChooseFBConfig(dpy, screen, visualAttribs, &fbConfigCount);
	
	printf("Found %d config count, selecting the first one, anyway\n", fbConfigCount);

	fbConfig = fbConfigs[0];
	XFree(fbConfigs);

	XVisualInfo* vi = glXGetVisualFromFBConfig(dpy, fbConfig);
	
	rootWindow = DefaultRootWindow(dpy);
	wmDeleteWindow = XInternAtom(dpy, "WM_DELETE_WINDOW", 0);

	XSetWindowAttributes swa;
	swa.colormap   = XCreateColormap(dpy, rootWindow, vi->visual, AllocNone);
	swa.event_mask = KeyPressMask | KeyReleaseMask | StructureNotifyMask;

	windowWidth = 800;
	windowHeight = 600;
	
	window = XCreateWindow(dpy, rootWindow, 0, 0, windowWidth, windowHeight, 0, vi->depth, InputOutput, vi->visual, CWEventMask | CWColormap, &swa);
	XSetWMProtocols(dpy, window, &wmDeleteWindow, 1);

	XStoreName(dpy, window, "Hello");
	XMapWindow(dpy, window);
	
	glXCreateContextAttribsARBProc contextLoader = NULL;
	contextLoader = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((const unsigned char*)"glXCreateContextAttribsARB");
	
	glContext = contextLoader(dpy, fbConfig, 0, True, glxContextAttribs);
	glXMakeCurrent(dpy, window, glContext);

	if(!gladLoadGLLoader((GLADloadproc)glXGetProcAddress)) {
		printf("Could not read the opengl functions.\n");
		exit(-2);
	}
	
	int glMajorVersion, glMinorVersion;
	glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
	glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);
	
	if(glMajorVersion < 3 && glMinorVersion < 3) {
		printf("Sorry, your OpenGL Version is not that is used.\n");
		printf("OpenGL version used: 3.3, your OpenGL version: %d.%d\n", glMajorVersion, glMinorVersion);
		printf("Shutting down\n");
		exit(-5);
	}

	printf("OpenGL Version: %d.%d\n", glMajorVersion, glMinorVersion);

	startClock();
	initRenderingSystem();
}

static void
updateWindow()
{
	glXSwapBuffers(dpy, window);
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
			break;

		case KeyPress:
			if(XLookupKeysym(&xev.xkey, 0) == XK_Left)  keyLeft = 1;
			if(XLookupKeysym(&xev.xkey, 0) == XK_Right) keyRight = 1;
			if(XLookupKeysym(&xev.xkey, 0) == XK_Up)    keyUp = 1;
			if(XLookupKeysym(&xev.xkey, 0) == XK_Down)  keyDown = 1;
			if(XLookupKeysym(&xev.xkey, 0) == XK_w)     keyFront = 1;
			if(XLookupKeysym(&xev.xkey, 0) == XK_s)     keyBack = 1;
			if(XLookupKeysym(&xev.xkey, 0) == XK_space) keySpace = 1;
			if(XLookupKeysym(&xev.xkey, 0) == XK_Shift_L) keyShift = 1;
			break;
		case KeyRelease:
			if(XLookupKeysym(&xev.xkey, 0) == XK_Left)  keyLeft = 0;
			if(XLookupKeysym(&xev.xkey, 0) == XK_Right) keyRight = 0;
			if(XLookupKeysym(&xev.xkey, 0) == XK_Up)    keyUp = 0;
			if(XLookupKeysym(&xev.xkey, 0) == XK_Down)  keyDown = 0;
			if(XLookupKeysym(&xev.xkey, 0) == XK_w)     keyFront = 0;
			if(XLookupKeysym(&xev.xkey, 0) == XK_s)     keyBack = 0;
			if(XLookupKeysym(&xev.xkey, 0) == XK_space) keySpace = 0;
			if(XLookupKeysym(&xev.xkey, 0) == XK_Shift_L) keyShift = 0;
			break;
		}
	}
}

static void
terminateWindow()
{
	glXMakeCurrent(dpy, None, None);
	glXDestroyContext(dpy, glContext);
	XDestroyWindow(dpy, window);
	XCloseDisplay(dpy);
}

int
main(int argc, char *argv[])
{
	initWindow();
	running = 1;
	
	createWorld(2, 4, 2);
	for(unsigned int i = 0; i < 2 * 4 * 2; i++)
		generateChunkModel(i % 2, (i / 2) % 4, i / 8);
	vec3 camNormal   = { 0.0, 1.0, 0.0 };
	vec3 camLook     = { 0.0, 0.0, 0.0 };

	while(running) {
		double startProcess = getCurrentTimeNano();
		
		if(keyLeft) cameraYaw -= 5.0/60.0;
		if(keyRight) cameraYaw += 5.0/60.0;
		if(keyUp) cameraPitch += 5.0/60.0;
		if(keyDown) cameraPitch -= 5.0/60.0;

		if(keyFront) { 
			cameraPosition[0] += cos(cameraYaw) * 10.0/60.0;
			cameraPosition[2] += sin(cameraYaw) * 10.0/60.0;
		}

		if(keyBack) { 
			cameraPosition[0] -= cos(cameraYaw) * 10.0/60.0;
			cameraPosition[2] -= sin(cameraYaw) * 10.0/60.0;
		}
		
		if(keySpace) 
			cameraPosition[1] += 10.0/60.0;
		
		if(keyShift)
			cameraPosition[1] -= 10.0/60.0;

		camLook[0] = cameraPosition[0] + cos(cameraYaw);
		camLook[2] = cameraPosition[2] + sin(cameraYaw);
		camLook[1] = cameraPosition[1] + cos(cameraPitch);
	

		setCamera(cameraPosition, camNormal, camLook);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		render();
		
		updateWindow();
		double end = getCurrentTimeNano();
		sleepNanosec((1.0 / FRAMES_PER_SEC) - (end - startProcess));
	}
	terminateWindow();
}
