/*
 * @author	Brandon Connes and Ian White
 * @date	2/9/2011
 *
 * Creates a cube, grass, and a tree using OpenGL.
 * Creates a camera able to rotate around the scene. (Mouse motion)
 * Able to move scene in all directions, and scale up and down. (WSAD, UP/DN/LF/RT)
 * Able to change color of ground to random color. (Mouse click)
 *
 */
#include "headers/SDL/SDL.h"
#include "headers/SDL/SDL_main.h"
#include "headers/SDL/SDL_opengl.h"
#include <stdio.h>

//New defines that we can use inside of mouse input handling to avoid magic numbers
#define WINDOW_WIDTH  1024
#define WINDOW_HEIGHT 768
#define ORTHO_SIZE 	  10		// Size of orthographic projection matrix.

//	Function for drawing one line.
#define   drawOneLine(x1,y1,z1,x2,y2,z2) glBegin(GL_LINES); \
	   glVertex3f ((x1),(y1),(z1)); glVertex3f ((x2),(y2),(z2)); glEnd();


//Means of the user exiting the main loop - it is static because it will not be seen outside of this file.
static int user_exit = 0;
static int keys_down[256];

// 6 sides of a cube represented in GLdouble vector arrays.
GLdouble V0[] =  {1.0, 1.0, -1.0};
GLdouble V1[] =  {1.0, -1.0, -1.0};
GLdouble V2[] =  {-1.0, -1.0, -1.0};
GLdouble V3[] =  {-1.0, 1.0, -1.0};
GLdouble V4[] =  {1.0, 1.0, 1.0};
GLdouble V5[] =  {1.0, -1.0, 1.0};
GLdouble V6[] =  {-1.0, 1.0, 1.0};
GLdouble V7[] =  {-1.0, -1.0, 1.0};

// 6 sides of a tree represented in GLdouble vector arrays.
GLdouble T0[] = {4, -1, 4};
GLdouble T1[] = {2, -1, 4};
GLdouble T2[] = {2, -1, 2};
GLdouble T3[] = {4, -1, 2};
GLdouble T4[] = {4,  5, 4};
GLdouble T5[] = {2,  5, 4};
GLdouble T6[] = {2,  5, 2};
GLdouble T7[] = {4,  5, 2};

// 6 sides of the leaves of a tree represented in GLdouble vector arrays.
GLdouble L0[] = {7, 5.1,-1};
GLdouble L1[] = {7, 5.1, 7};
GLdouble L2[] = {-1, 5.1,7};
GLdouble L3[] = {-1, 5.1,-1};
GLdouble L4[] = { 7, 8, 7};
GLdouble L5[] = {-1, 8, 7};
GLdouble L6[] = {-1, 8, -1};
GLdouble L7[] = { 7, 8, -1};

double r = .4;
double g = .1;
double b = 0;

GLuint cube;

//Added two new functions - also static as they will not be used outside of this file.
//Eventually these will be moved out of here into a "rendering" subsystem, which is why they are prefixed r_.
static void r_init();
static void r_drawFrame();
static void input_keyDown(SDLKey k);
static void input_keyUp(SDLKey k);
static void input_mouseMove(int xPos, int yPos);
static void input_mouseClick(int x, int y, int button);

//Program entry point
int SDL_main(int argc, char* argv[])
{
	SDL_Event	event;		//Used for handling input events, as you can see later on.
	SDL_Surface	*screen;	//http://www.libsdl.org/cgi/docwiki.cgi/SDL_Surface

	//The following is pretty self-explanatory
	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0)
	{
		printf("Unable to initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	//You can of course customize this.
	SDL_WM_SetCaption("Model Transformations", "Model Transformations");

	//We need to explicitly enable double buffering
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	//Initialize window, setting the resolution to 1024x768 @ 32 bits per pixel. We want an OpenGL window.
	screen = SDL_SetVideoMode(1024, 768, 32, SDL_OPENGL);
	if(!screen)
	{
		printf("Unable to set video mode: %s\n", SDL_GetError());
		return 1;
	}

	//Any other one-time initialization would typically go here.
	//"Renderer" initialization
	r_init();

	//This is what is referred to as the "game loop." Obviously there is not much here currently.
	while(!user_exit)
	{
		//Handle input
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
			case SDL_KEYDOWN:
				input_keyDown(event.key.keysym.sym);
				break;
			case SDL_KEYUP:
				input_keyUp(event.key.keysym.sym);
				break;
			case SDL_MOUSEMOTION:
				input_mouseMove(event.motion.x, event.motion.y);
				break;
			case SDL_MOUSEBUTTONDOWN:
				input_mouseClick(event.button.x, event.button.y, event.button.button);
				break;
			case SDL_QUIT:
				exit(0);
			}
		}
		//Here is where you will do any OpenGL drawing. You would also do things like update moving objects, etc.
		//Do whatever we need to do to draw a single image.
		r_drawFrame();
	}

	//Shut down SDL
	SDL_Quit();

	//Everything went OK.
	return 0;
}


//	Takes xPos and yPos, gets the change in x and the
//	change in y and calculates the rotation angles among
//	the y axis using dx, and the rotation among the x axis
//	using dy, and then rotates among both, and then resets
//	the cursor to the center.
static void input_mouseMove(int xPos, int yPos)
{
	float halfWinWidth, halfWinHeight,
		  dx, dy, yRotationAngle, xRotationAngle;

	halfWinWidth  = (float)WINDOW_WIDTH  / 2.0;
	halfWinHeight = (float)WINDOW_HEIGHT / 2.0;

	dx = (xPos - halfWinWidth) / 2; dy = (yPos - halfWinHeight) / 2;

	yRotationAngle = 0;
	yRotationAngle += dx;
	if(yRotationAngle > 360.f)
		yRotationAngle -= 360.f;

	xRotationAngle = 0;
	xRotationAngle += dy;
	if(xRotationAngle > 360.f)
		xRotationAngle -= 360.f;

	glRotatef(yRotationAngle, 0.f, 1.f, 0.f);
	glRotatef(xRotationAngle, 1.f, 0.f, 0.f);

	//Reset cursor to center

	SDL_WarpMouse(halfWinWidth, halfWinHeight);
}

static void input_keyDown(SDLKey k)
{
	keys_down[k] = 1;
	if(k == SDLK_ESCAPE || k == SDLK_q)		// Press 'q' or 'esc' -> quit
		user_exit = 1;
	else if(k == SDLK_s)					// Press 's' -> down on y-axis
		glTranslatef(0.0, -1.0, 0.0);
	else if(k == SDLK_w)					// Press 'w' -> up on y-axis
		glTranslatef(0.0, 1.0, 0.0);
	else if(k == SDLK_a)					// Press 'a' -> up on z-axis
		glTranslatef(0.0, 0.0, 1.0);
	else if(k == SDLK_d)					// Press 'd' -> down on z-axis
		glTranslatef(0.0, 0.0, -1.0);
	else if(k == SDLK_UP)					// Press 'up' -> down on x-axis
		glTranslatef(-1.0, 0.0, 0.0);
	else if(k == SDLK_DOWN)					// Press 'down' -> up on x-axis
		glTranslatef(1.0, 0.0, 0.0);
	else if(k == SDLK_LEFT)					// Press 'left' -> scale down
		glScalef(0.95, 0.95, 0.95);
	else if(k == SDLK_RIGHT)				// Press 'right' -> scale up
		glScalef(1.05, 1.05, 1.05);
}

static void input_keyUp(SDLKey k) { keys_down[k] = 0; }
static void r_init()
{
	//Enable depth buffering
	glEnable(GL_DEPTH_TEST);

	//Change to projection mode
	glMatrixMode(GL_PROJECTION);

	//Load the identity matrix
	glLoadIdentity();

	//Create an orthographic projection matrix with the left plane at
	//X = -10.0, right plane at X = 10.0, bottom plane at Y = -10.0,
	//top plane at Y = 10.0, near plane at 50.0, and far plane at -50.0.
	glOrtho(-ORTHO_SIZE, ORTHO_SIZE, -ORTHO_SIZE, ORTHO_SIZE, 50.0, -50.0);

	//Swap back to modelview
	glMatrixMode(GL_MODELVIEW);

	//Load the identity matrix
	glLoadIdentity();

	gluLookAt(2.0, 2.0, 0.0,		// Place the 'eye' at (2, 2, 0)
			  1.0, 1.0, 1.0,		// Place the 'center' at (1, 1, 1)
			  0.0, -1.0, 0.0);		// Place 'up' at (-1)
}

static void input_mouseClick(int x, int y, int but)
{
	r = (double) rand() / (double) RAND_MAX;
	g = (double) rand() / (double) RAND_MAX;
	b = (double) rand() / (double) RAND_MAX;

	r_drawFrame();
}


//Produces a simple image
static void r_drawFrame()
{
	//Clear the color buffer (these are the final values that get sent to the monitor).
	glClearColor(0.0, 0.6, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	GLuint cube = glGenLists(1);

	//Cube
	//TODO: on click, randomize color values, make stipple grass
	glLineWidth(4.0);
	glColor3f(0.0, 0.0, 0.0);		//	Black
	glNewList(cube, GL_COMPILE_AND_EXECUTE);
	glBegin(GL_QUADS);
		glVertex3dv(V0);		glVertex3dv(V1);		glVertex3dv(V2);		glVertex3dv(V3);
		glColor3f(1.0, 1.0, 0.0);
		glVertex3dv(V1);		glVertex3dv(V2);		glVertex3dv(V7);		glVertex3dv(V5);
		glColor3f(1.0, 0.0, 0.0);
		glVertex3dv(V0);		glVertex3dv(V4);		glVertex3dv(V5);		glVertex3dv(V1);
		glColor3f(0.0, 0.0, 1.0);
		glVertex3dv(V4);		glVertex3dv(V6);		glVertex3dv(V7);		glVertex3dv(V5);
		glColor3f(0.0, 1.0, 0.0);
		glVertex3dv(V2);		glVertex3dv(V3);		glVertex3dv(V6);		glVertex3dv(V7);
		glColor3f(1.0, 0.0, 1.0);
		glVertex3dv(V0);		glVertex3dv(V3);		glVertex3dv(V6);		glVertex3dv(V4);
	glEnd();
	glEndList();

	// X plane, represented as grass / ground
	glColor3f(r, g, b);		//	Light green, open to change.
	glBegin(GL_POLYGON);
		glVertex3d(-5, -1.1, -5);
		glVertex3d(-5, -1.1, 5);
		glVertex3d(5, -1.1, 5);
		glVertex3d(5, -1.1, -5);
	glEnd();

	//	Tree trunk
	glColor3f(.8, .3, 0);		//	Brown
	glBegin(GL_QUADS);
		glVertex3dv(T0);		glVertex3dv(T1);		glVertex3dv(T2);	glVertex3dv(T3);
		glVertex3dv(T4);		glVertex3dv(T5);		glVertex3dv(T6);	glVertex3dv(T7);
		glVertex3dv(T0);		glVertex3dv(T4);		glVertex3dv(T5);	glVertex3dv(T1);
		glVertex3dv(T1);		glVertex3dv(T2);		glVertex3dv(T6);	glVertex3dv(T5);
		glVertex3dv(T2);		glVertex3dv(T3);		glVertex3dv(T7);	glVertex3dv(T6);
		glVertex3dv(T0);		glVertex3dv(T3);		glVertex3dv(T7);	glVertex3dv(T4);
	glEnd();

	//	Tree leaves
	glColor3f(0, 1, 0);			//	Green
	glBegin(GL_QUADS);
		glVertex3dv(L0);		glVertex3dv(L1);		glVertex3dv(L2);	glVertex3dv(L3);
		glVertex3dv(L4);		glVertex3dv(L5);		glVertex3dv(L6);	glVertex3dv(L7);
		glVertex3dv(L0);		glVertex3dv(L4);		glVertex3dv(L5);	glVertex3dv(L1);
		glVertex3dv(L1);		glVertex3dv(L2);		glVertex3dv(L6);	glVertex3dv(L5);
		glVertex3dv(L2);		glVertex3dv(L3);		glVertex3dv(L6);	glVertex3dv(L5);
		glVertex3dv(L0);		glVertex3dv(L3);		glVertex3dv(L6);	glVertex3dv(L7);
	glEnd();

	//	Simulate grass with stippled lines
	glColor3f(0.2, 0.2, 0.2);		// Dark grey
	glLineWidth(1);
    glEnable (GL_LINE_STIPPLE);
    glLineStipple(1, 0x1C47);
    glBegin(GL_LINES);
    int i;
    for(i = -4; i < 5; i++){
    	glVertex3f(i, -1, -5);
    	glVertex3f(i, -1, 5);
    }
    glEnd();

	//Swap the front and back frame buffers to display the image in our window.
	SDL_GL_SwapBuffers();
}
