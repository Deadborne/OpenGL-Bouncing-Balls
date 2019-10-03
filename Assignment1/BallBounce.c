// for my own convenience: https://www.geeksforgeeks.org/multithreading-c-2/

#include <stdlib.h>

#include "Dependencies/glew/glew.h"
#include "Dependencies/freeglut/freeglut.h"
#include <windows.h> 
#include <Math.h>     
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>

#define PI 3.14159265f

//a few Global Variables (sue me) for original ball, gravity, screen refresh.

char title[] = "Bouncing Ball";  // Window title
int windowWidth = 800;     // Window width
int windowHeight = 600;     // Window height
int windowPosX = 50;      // Window's top-left corner x
int windowPosY = 50;      // Window's top-left corner y

int refreshMillis = 33;      // Screen refresh period (milliseconds). 30FPS = 33 milliseconds per frame.
GLfloat grav = 0.02; //faux gravitational constant

static int numberBalls = -1;

//generate a number to determine how many additional balls
int random()
{
	int r;
	srand(time(NULL));
	r = rand() % 7 + 2;
	printf("Your random number is %d\n", r);
	return r;
};

//bouncy ball struct for additional balls
struct BouncyBall {
	GLfloat radius;
	GLfloat xSpawn, ySpawn;
	GLfloat XMaxBound, XMinBound, YMaxBound, YMinBound;
	GLfloat xSpeed, ySpeed;
	bool glock;
};
static struct BouncyBall ballArray[8];

GLfloat v1, v2, v3, v4, v5, v6, v7, v8, v9;

//ball 1
GLfloat ballRadius = 0.1f;   // Radius
GLfloat ballX = -0.2f;         // Where the ball spawns (x,y)
GLfloat ballY = 0.9f;
GLfloat ballXMax, ballXMin, ballYMax, ballYMin; // Ball's center (x, y) bounds
GLfloat xSpeed = -0.06f;      // Ball's speed in x and y directions
GLfloat ySpeed = 0.009f;

//ball 2
GLfloat ballRadius1 = 0.14f;   // Radius
GLfloat ballX1 = 0.2f;        // Where the ball spawns (x,y)
GLfloat ballY1 = 0.2f;
GLfloat ballXMax1, ballXMin1, ballYMax1, ballYMin1; // Ball's center (x, y) bounds
GLfloat xSpeed1 = 0.04f;      // Ball's speed in x and y directions
GLfloat ySpeed1 = 0.004f;

// Projection clipping area
GLdouble clipAreaXLeft, clipAreaXRight, clipAreaYBottom, clipAreaYTop;

/* Initialize OpenGL Graphics */
void initGL() {
	glClearColor(0.0, 0.0, 0.0, 1.0); // Set background color
}

/* Callback handler for window re-paint event */
void display() {

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //enable transparency

	glClear(GL_COLOR_BUFFER_BIT);  // Clear the color buffer
	glMatrixMode(GL_MODELVIEW);    // To operate on the model-view matrix
	glLoadIdentity();              // Reset model-view matrix

	//ball1
	// Use triangular segments to form a circle
	glBegin(GL_TRIANGLE_FAN);
	glColor4f(1.0f, 0.0f, 0.5f, 0.5f);  // Color of ball with alpha for transparency (4th param)
	glVertex2f(ballX, ballY);       // Center of circle
	int numSegments = 100;
	GLfloat angle;
	for (int i = 0; i <= numSegments; i++) { // Last vertex same as first vertex
		angle = i * 2.0f * PI / numSegments;  // 360 deg for all segments
		glVertex2f(ballX + cos(angle) * ballRadius, ballY + sin(angle) * ballRadius);
	}
	glEnd();

	//ball2
	// Use triangular segments to form a circle
	glBegin(GL_TRIANGLE_FAN);
	glColor4f(0.0f, 0.5f, 1.0f, 0.5f);  // Color of ball with alpha for transparency (4th param)
	glVertex2f(ballX1, ballY1);       // Center of circle
	GLfloat angle1;
	for (int i = 0; i <= numSegments; i++) { // Last vertex same as first vertex
		angle1 = i * 2.0f * PI / numSegments;  // 360 deg for all segments
		glVertex2f(ballX1 + cos(angle1) * ballRadius1, ballY1 + sin(angle1) * ballRadius1);
	}
	glEnd();

	//all balls in BallArray
	for (int i = 0; i <= (sizeof(ballArray) / sizeof(struct BouncyBall)); i++)
	{
		glBegin(GL_TRIANGLE_FAN);
		if (i % 3 == 0) {
			glColor4f(1.0f, 0.0f, 0.0f, 0.5f);
		}
		else if (i % 3 == 1) {
			glColor4f(0.0f, 1.0f, 0.0f, 0.5f);
		}
		else if (i % 3 == 2) {
			glColor4f(0.0f, 0.0f, 1.0f, 0.5f);
		}
		glVertex2f(ballArray[i].xSpawn, ballArray[i].ySpawn); 
		GLfloat angleX;
		for (int j = 0; j <= numSegments; j++) { 
			angleX = j * 2.0f * PI / numSegments;
			glVertex2f(ballArray[i].xSpawn + cos(angleX) * ballArray[i].radius, ballArray[i].ySpawn + sin(angleX) * ballArray[i].radius);
		}
		glEnd();
	}

	glutSwapBuffers();  // Swap front and back buffers (of double buffered mode)

	// Animation Control - compute the location for the next refresh
	ballX += xSpeed;
	ballY += ySpeed;

	ballX1 += xSpeed1;
	ballY1 += ySpeed1;
	

	//location for balls in array
	for (int i = 0; i <= (sizeof(ballArray) / sizeof(struct BouncyBall)); i++) {
		ballArray[i].xSpawn += ballArray[i].xSpeed;
		ballArray[i].ySpawn += ballArray[i].ySpeed;
	}

	bool gravlock = true;
	bool gravlock1 = true;
	for (int i = 0; i <= (sizeof(ballArray) / sizeof(struct BouncyBall)); i++) {
		ballArray[i].glock = true;
	}

	// Check if ball 1 exceeds the edges
	if (ballX > ballXMax) {
		ballX = ballXMax;
		xSpeed = -xSpeed;
	}
	else if (ballX < ballXMin) {
		ballX = ballXMin;
		xSpeed = -xSpeed;
	}
	if (ballY > ballYMax) {
		ballY = ballYMax;
		ySpeed = -ySpeed;
	}
	else if (ballY < ballYMin) {
		ballY = ballYMin;
		ySpeed = -ySpeed;
		gravlock = true;
	}

	//Check if ball 2 exceeds the edges
	if (ballX1 > ballXMax1) {
		ballX1 = ballXMax1;
		xSpeed1 = -xSpeed1;
	}
	else if (ballX1 < ballXMin1) {
		ballX1 = ballXMin1;
		xSpeed1 = -xSpeed1;
	}
	if (ballY1 > ballYMax1) {
		ballY1 = ballYMax1;
		ySpeed1 = -ySpeed1;
	}
	else if (ballY1 < ballYMin1) {
		ballY1 = ballYMin1;
		ySpeed1 = -ySpeed1;
		gravlock1 = true;
	}

	//check if array balls exceed the edges
	for (int i = 0; i <= (sizeof(ballArray) / sizeof(struct BouncyBall)); i++)
	{
		if (ballArray[i].xSpawn > ballXMax) {
			ballArray[i].xSpawn = ballXMax;
			ballArray[i].xSpeed = (ballArray[i].xSpeed) * -1;
		}
		else if (ballArray[i].xSpawn < ballXMin){
			ballArray[i].xSpawn = ballXMin;
			ballArray[i].xSpeed = -1 * (ballArray[i].xSpeed);
		}
		if (ballArray[i].ySpawn > ballYMax) {
			ballArray[i].ySpawn = ballYMax;
			ballArray[i].ySpeed = -1 * (ballArray[i].ySpeed);
		}
		else if (ballArray[i].ySpawn < ballYMin) {
			ballArray[i].ySpawn = ballYMin;
			ballArray[i].ySpeed = -1 * (ballArray[i].ySpeed);
			ballArray[i].glock = true;
		}
	}

	//collision detection between balls
	if (sqrt((ballY1 - ballY)*(ballY1 - ballY) + (ballX1 - ballX)*(ballX1 - ballX)) <= (ballRadius1 + ballRadius))
	{
		if (xSpeed > 0)
		{
			if (xSpeed1 > 0)
			{
				if (ballX > ballX1)
				{
					xSpeed += xSpeed1;
					xSpeed1 = -xSpeed1;
				}
				else
				{
					xSpeed1 += xSpeed;
					xSpeed = -xSpeed;
				}
			}
			else
			{
				xSpeed = -xSpeed;
				xSpeed1 = -xSpeed1;
			}
		}
		else
		{
			if (xSpeed1 < 0)
			{
				if (ballX < ballX1)
				{
					xSpeed += xSpeed1;
					xSpeed1 = -xSpeed1;
				}
				else
				{
					xSpeed1 += xSpeed;
					xSpeed = -xSpeed;
				}
			}
			else
			{
				xSpeed = -xSpeed;
				xSpeed1 = -xSpeed1;
			}
		}

		if (ySpeed > 0)
		{
			if (ySpeed1 > 0)
			{
				if (ballY > ballY1)
				{
					ySpeed += ySpeed1;
					ySpeed1 = -ySpeed1;
				}
				else
				{
					ySpeed1 += ySpeed;
					ySpeed = -ySpeed;
				}
			}
			else
			{
				ySpeed = -ySpeed;
				ySpeed1 = -ySpeed1;
			}
		}
		else
		{
			if (ySpeed1 < 0)
			{
				if (ballY < ballY1)
				{
					ySpeed += ySpeed1;
					ySpeed1 = -ySpeed1;
				}
				else
				{
					ySpeed1 += ySpeed;
					ySpeed = -ySpeed;
				}
			}
			else
			{
				ySpeed = -ySpeed;
				ySpeed1 = -ySpeed1;
			}
		}
	}

	//collision detection for ALL balls
	for (int i = 0; i <= (sizeof(ballArray) / sizeof(struct BouncyBall)); i++) {
		for (int j = 0; j <= (sizeof(ballArray) / sizeof(struct BouncyBall)); j++) {
			GLfloat yCross = ballArray[i].ySpawn - ballArray[j].ySpawn;
			GLfloat xCross = ballArray[i].xSpawn - ballArray[j].xSpawn;
			if (i == j) {
			}
			//else if (sqrt((ballY1 - ballY)*(ballY1 - ballY) + (ballX1 - ballX)*(ballX1 - ballX)) <= (ballRadius1 + ballRadius)){
			//i is BallX1, j is BallX
			else if (sqrt((yCross*yCross) + (xCross*xCross)) <= (ballArray[i].radius + ballArray[j].radius)) {
				if (ballArray[j].xSpeed > 0)
				{
					if (ballArray[i].xSpeed > 0)
					{
						if (ballArray[j].xSpawn > ballArray[i].xSpawn)
						{
							ballArray[j].xSpeed += ballArray[i].xSpeed;
							ballArray[i].xSpeed = -ballArray[i].xSpeed;
						}
						else
						{
							ballArray[i].xSpeed += ballArray[j].xSpeed;
							ballArray[j].xSpeed = -ballArray[j].xSpeed;
						}
					}
					else
					{
						ballArray[j].xSpeed = -ballArray[j].xSpeed;
						ballArray[i].xSpeed = -ballArray[i].xSpeed;
					}
				}
				else
				{
					if (ballArray[i].xSpeed < 0)
					{
						if (ballArray[j].xSpawn < ballArray[i].xSpawn)
						{
							ballArray[j].xSpeed += ballArray[i].xSpeed;
							ballArray[i].xSpeed = -ballArray[i].xSpeed;
						}
						else
						{
							ballArray[i].xSpeed += ballArray[j].xSpeed;
							ballArray[j].xSpeed = -ballArray[j].xSpeed;
						}
					}
					else
					{
						ballArray[j].xSpeed = -ballArray[j].xSpeed;
						ballArray[i].xSpeed = -ballArray[i].xSpeed;
					}
				}

				if (ballArray[j].ySpeed > 0)
				{
					if (ballArray[i].ySpeed > 0)
					{
						if (ballArray[j].ySpawn > ballArray[i].ySpawn)
						{
							ballArray[j].ySpeed += ballArray[i].ySpeed;
							ballArray[i].ySpeed = -ballArray[i].ySpeed;
						}
						else
						{
							ballArray[i].ySpeed += ballArray[j].ySpeed;
							ballArray[j].ySpeed = ballArray[j].ySpeed;
						}
					}
					else
					{
						ballArray[j].ySpeed = ballArray[j].ySpeed;
						ballArray[i].ySpeed = -ballArray[i].ySpeed;
					}
				}
				else
				{
					if (ballArray[j].ySpeed < 0)
					{
						if (ballArray[j].ySpawn < ballArray[i].ySpawn)
						{
							ballArray[j].ySpeed += ballArray[i].ySpeed;
							ballArray[i].ySpeed = -ballArray[i].ySpeed;
						}
						else
						{
							ballArray[i].ySpeed += ballArray[j].ySpeed;
							ballArray[j].ySpeed = -ballArray[i].ySpeed;
						}
					}
					else
					{
						ballArray[j].ySpeed = -ballArray[j].ySpeed;
						ballArray[i].ySpeed = -ballArray[i].ySpeed;
					}
				}
			}
		}
	}
	
	//gravity for ball 1
	if (ySpeed >= 0 && gravlock == false) {
		ySpeed = ySpeed + grav;
	}
	else {
		gravlock = true;
		ySpeed = ySpeed - grav;
	}

	//gravity for ball 2
	if (ySpeed1 >= 0 && gravlock1 == false) {
		ySpeed1 = ySpeed1 + grav;
	}
	else {
		gravlock1 = true;
		ySpeed1 = ySpeed1 - grav;
	}

	//gravity for array balls
	for (int i = 0; i <= (sizeof(ballArray) / sizeof(struct BouncyBall)); i++) {
		if (ballArray[i].ySpeed >= 0 && ballArray[i].glock == false) {
			ballArray[i].ySpeed = ballArray[i].ySpeed + grav;
		}
		else {
			ballArray[i].glock = true;
			ballArray[i].ySpeed = ballArray[i].ySpeed - grav;
		}
	}

}


/* Call back when the windows is re-sized */
void reshape(GLsizei width, GLsizei height) {
	// Compute aspect ratio of the new window
	if (height == 0) height = 1;                // To prevent divide by 0
	GLfloat aspect = (GLfloat)width / (GLfloat)height;

	// Set the viewport to cover the new window
	glViewport(0, 0, width, height);

	// Set the aspect ratio of the clipping area to match the viewport
	glMatrixMode(GL_PROJECTION);  // To operate on the Projection matrix
	glLoadIdentity();             // Reset the projection matrix
	if (width >= height) {
		clipAreaXLeft = -1.0 * aspect;
		clipAreaXRight = 1.0 * aspect;
		clipAreaYBottom = -1.0;
		clipAreaYTop = 1.0;
	}
	else {
		clipAreaXLeft = -1.0;
		clipAreaXRight = 1.0;
		clipAreaYBottom = -1.0 / aspect;
		clipAreaYTop = 1.0 / aspect;
	}

	gluOrtho2D(clipAreaXLeft, clipAreaXRight, clipAreaYBottom, clipAreaYTop); //sets 2d orthographic viewing region. No touchy.

	ballXMin = clipAreaXLeft + ballRadius;
	ballXMax = clipAreaXRight - ballRadius;
	ballYMin = clipAreaYBottom + ballRadius;
	ballYMax = clipAreaYTop - ballRadius;

	ballXMin1 = clipAreaXLeft + ballRadius1;
	ballXMax1 = clipAreaXRight - ballRadius1;
	ballYMin1 = clipAreaYBottom + ballRadius1;
	ballYMax1 = clipAreaYTop - ballRadius1;

	//array balls
	for (int i = 0; i <= (sizeof(ballArray) / sizeof(struct BouncyBall)); i++) {

		ballArray[i].XMinBound = clipAreaXLeft + ballArray[i].radius;
		ballArray[i].XMaxBound = clipAreaXRight - ballArray[i].radius;
		ballArray[i].XMinBound = clipAreaYBottom + ballArray[i].radius;
		ballArray[i].XMaxBound = clipAreaYTop - ballArray[i].radius;
	}

}

/* Called back when the timer expired */
void Timer(int value) {
	glutPostRedisplay();    // Post a paint request to activate display()
	glutTimerFunc(refreshMillis, Timer, 0); // subsequent timer call at milliseconds
}

//creates 2 to 8 additional balls to ruin my life
 BallMaker(int no) {
	
	//radius, xSpawn, ySpawn, XMax, Xmin, YMax,YMin, xSpeed, ySpeed, gravity lock
	struct BouncyBall a = {0.1f + ((0.01f)*no), 0.1f * (no+1), 0.1f * (no+1), 0.0f,0.0f,0.0f,0.0f, 0.01f * (no+1), 0.01f * (no+1), true};
	ballArray[no] = a;

	return;
}

/// Main function
int main(int argc, char** argv) {

	//pthread_t tid;
	int r = random();
	numberBalls = r;

	/*for (int i = 0; i < r; i++) {
	pthread_create(&tid, NULL, BallMaker(i), (void *)&tid);
	}*/

	/*for (int i = 0; i < r; i++) {
		BallMaker(i);
	}*/

	BallMaker(1);
	BallMaker(2);

	//GL stuff
	glutInit(&argc, argv);            // Boot up GLUT
	glutInitDisplayMode(GLUT_DOUBLE); // Enable double buffered mode
	glutInitWindowSize(windowWidth, windowHeight);  // Create a window with given height and width
	glutInitWindowPosition(windowPosX, windowPosY); // Initial window top-left corner (x, y)
	glutCreateWindow(title);      // Create window with given title
	glutDisplayFunc(display);     // Register callback handler for window re-paint
	glutReshapeFunc(reshape);     // Register callback handler for window re-shape
	glutTimerFunc(0, Timer, 0);   // First timer call immediately
	initGL();                     // Our own OpenGL initialization
	glutMainLoop();               // Enter event-processing loop

	//pthread_exit(NULL);

	return 0;
}