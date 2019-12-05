#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>

#include "util.h"
#include "rendering.h"
#include "world.h"
#include "glad.h"

#include <GL/glx.h>

#define FRAMES_PER_SEC 60.0

#define GLX_CONTEXT_MAJOR_VERSION_ARB       0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB       0x2092
#define GLX_CONTEXT_PROFILE_MASK_ARB        0x9126

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

void initWindow();
void updateWindow();
void terminateWindow();
void processKeyState(XEvent* xev, unsigned char state);
void processCamera(float cameraMoveSpeed, float cameraRotateSpeed);

static Display* dpy;
static Window   rootWindow, window;
static Atom     wmDeleteWindow;
static int      screen;

static vec3  cameraPosition = { 0.0, 16.0, 10.0 };
static vec3  cameraNormal   = { 0.0, 1.0, 0.0 };
static vec3  cameraLook     = { 0.0, 0.0, 0.0 };

static float cameraPitch   = M_PI / 2.0;
static float cameraYaw     = 0;

static char keyLeft = 0,
	    keyRight = 0, 
	    keyUp = 0, 
	    keyDown = 0,
	    keyFront = 0,
	    keyBack = 0,
	    keySpace = 0,
	    keyShift = 0;


static int windowWidth, windowHeight;
static unsigned char running = 0;

static GLXFBConfig fbConfig;
static GLXContext  glContext;

static int visualAttribs[] = {
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

void
initWindow()
{
	int glxMajor, glxMinor;
	int fbConfigCount;
	int glMajorVersion, glMinorVersion;
	XSetWindowAttributes swa;
	glXCreateContextAttribsARBProc contextLoader = NULL;
	GLXFBConfig* fbConfigs;
	XVisualInfo* vi;

	dpy = XOpenDisplay(NULL);
	if(!dpy) {
		printf("Could not init X11\n");
		exit(-1);
	}
	screen = DefaultScreen(dpy);

	glXQueryVersion(dpy, &glxMajor, &glxMinor);
	if(glxMinor < 3) {
		printf("Invalid GLX Version\n");
		exit(-3);
	}
	fbConfigs = glXChooseFBConfig(dpy, screen, visualAttribs, &fbConfigCount);
	
	printf("Found %d config count, selecting the first one, anyway\n", fbConfigCount);

	fbConfig = fbConfigs[0];
	XFree(fbConfigs);

	vi = glXGetVisualFromFBConfig(dpy, fbConfig);
	
	rootWindow = DefaultRootWindow(dpy);
	wmDeleteWindow = XInternAtom(dpy, "WM_DELETE_WINDOW", 0);

	swa.colormap   = XCreateColormap(dpy, rootWindow, vi->visual, AllocNone);
	swa.event_mask = KeyPressMask | KeyReleaseMask | StructureNotifyMask;

	windowWidth = 800;
	windowHeight = 600;
	
	window = XCreateWindow(dpy, rootWindow, 0, 0, windowWidth, windowHeight, 0, vi->depth, InputOutput, vi->visual, CWEventMask | CWColormap, &swa);
	XSetWMProtocols(dpy, window, &wmDeleteWindow, 1);

	XStoreName(dpy, window, "Hello");
	XMapWindow(dpy, window);
	
	contextLoader = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((const unsigned char*)"glXCreateContextAttribsARB");
	
	glContext = contextLoader(dpy, fbConfig, 0, True, glxContextAttribs);
	glXMakeCurrent(dpy, window, glContext);

	if(!gladLoadGLLoader((GLADloadproc)glXGetProcAddress)) {
		printf("Could not read the opengl functions.\n");
		exit(-2);
	}
	
	glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
	glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);
	
	if(glMajorVersion < 3 && glMinorVersion < 3) {
		printf("Sorry, your OpenGL Version is not that is used.\n");
		printf("OpenGL version used: 3.3, your OpenGL version: %d.%d\n", glMajorVersion, glMinorVersion);
		printf("Shutting down\n");
		exit(-5);
	}

	printf("OpenGL Version: %d.%d\n", glMajorVersion, glMinorVersion);
}

void
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
			processKeyState(&xev, 1);
			break;
		case KeyRelease:
			processKeyState(&xev, 0);
			break;
		}
	}
}

void
terminateWindow()
{
	glXMakeCurrent(dpy, None, None);
	glXDestroyContext(dpy, glContext);
	XDestroyWindow(dpy, window);
	XCloseDisplay(dpy);
}

void
processKeyState(XEvent *xev, unsigned char state)
{
	if(XLookupKeysym(&xev->xkey, 0) == XK_Left)   keyLeft = state;
	if(XLookupKeysym(&xev->xkey, 0) == XK_Right)  keyRight = state;
	if(XLookupKeysym(&xev->xkey, 0) == XK_Up)     keyUp = state;
	if(XLookupKeysym(&xev->xkey, 0) == XK_Down)   keyDown = state;
	if(XLookupKeysym(&xev->xkey, 0) == XK_w)      keyFront = state;
	if(XLookupKeysym(&xev->xkey, 0) == XK_s)      keyBack = state;
	if(XLookupKeysym(&xev->xkey, 0) == XK_space)  keySpace = state;
	if(XLookupKeysym(&xev->xkey, 0) == XK_Shift_L) keyShift = state;
}

void
processCamera(float cameraMoveSpeed, float cameraRotateSpeed)
{
	if(keyLeft) cameraYaw -= cameraRotateSpeed;
	if(keyRight) cameraYaw += cameraRotateSpeed;
	if(keyUp) cameraPitch += cameraRotateSpeed;
	if(keyDown) cameraPitch -= cameraRotateSpeed;

	if(keyFront) { 
		cameraPosition[0] += cos(cameraYaw) * cameraMoveSpeed;
		cameraPosition[2] += sin(cameraYaw) * cameraMoveSpeed;
	}

	if(keyBack) { 
		cameraPosition[0] -= cos(cameraYaw) * cameraMoveSpeed;
		cameraPosition[2] -= sin(cameraYaw) * cameraMoveSpeed;
	}
	
	if(keySpace) 
		cameraPosition[1] += cameraMoveSpeed;
	
	if(keyShift)
		cameraPosition[1] -= cameraMoveSpeed;

	cameraLook[0] = cameraPosition[0] + cos(cameraYaw);
	cameraLook[2] = cameraPosition[2] + sin(cameraYaw);
	cameraLook[1] = cameraPosition[1] + cos(cameraPitch);
}

int
main(int argc, char *argv[])
{
	initWindow();
	startClock();
	initRenderingSystem();

	createWorld(32, 4, 32);

	running = 1;

	while(running) {
		double startProcess, endProcess;
		startProcess = getCurrentTimeNano();
		
		processCamera(10.0/60.0, 5.0/60.0);

		setCamera(cameraPosition, cameraNormal, cameraLook);
		render(cameraPosition[0], cameraPosition[1], cameraPosition[2]);
		
		updateWindow();

		endProcess = getCurrentTimeNano();
		sleepNanosec((1.0 / FRAMES_PER_SEC) - (endProcess - startProcess));
	}
	terminateWindow();
}

