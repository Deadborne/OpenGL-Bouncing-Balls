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

char title[] = "Why is OpenGl worth more than threads in this assignment?";  // Window title
int windowWidth = 1600;     // Window width
int windowHeight = 600;     // Window height
int windowPosX = 150;      // Window's top-left corner x
int windowPosY = 150;      // Window's top-left corner y

int refreshMillis = 33;      // Screen refresh period (milliseconds). 30FPS = 33 milliseconds per frame.
GLfloat grav = 0.02; //faux gravitational constant

static int numberBalls = -1;

pthread_mutex_t lock;
int badIterator = 0;



void iteratorUp() {
		pthread_mutex_lock(&lock);
	badIterator++;
		pthread_mutex_unlock(&lock);
}


//generate a number to determine how many additional balls
int random()
{
	int r;
	srand(time(NULL));
	r = rand() % 7 + 2;
	printf("Behold, a fiesta with %d balls\n ", r+1);
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
static struct BouncyBall ballArray[11];

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

	//create all balls in BallArray
	int numSegments = 100;
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

	//check if array balls exceed the edges
	for (int i = 0; i <= (sizeof(ballArray) / sizeof(struct BouncyBall)); i++)
	{
		if (ballArray[i].xSpawn > ballXMax) {
			ballArray[i].xSpawn = ballXMax;
			ballArray[i].xSpeed = -ballArray[i].xSpeed;
		}
		else if (ballArray[i].xSpawn < ballXMin){
			ballArray[i].xSpawn = ballXMin;
			ballArray[i].xSpeed = -ballArray[i].xSpeed;
		}
		if (ballArray[i].ySpawn > ballYMax) {
			ballArray[i].ySpawn = ballYMax;
			ballArray[i].ySpeed = -ballArray[i].ySpeed;
		}
		else if (ballArray[i].ySpawn < ballArray[i].YMinBound) {
			ballArray[i].ySpawn = ballArray[i].YMinBound;
			ballArray[i].ySpeed = -ballArray[i].ySpeed;
			ballArray[i].glock = true;
		}
	}

	//check if balls are going crazy
	for (int i = 0; i <= numberBalls; i++) {
		if (ballArray[i].ySpeed < -0.3f)
			ballArray[i].ySpeed = -0.0006f;
		if(ballArray[i].ySpeed > 0.3f)
			ballArray[i].ySpeed = 0.0006f;
	}


	//collision detection for ALL balls
	for (int i = 0; i <= numberBalls; i++) {
		for (int j = i+1; j <= numberBalls; j++) {

			GLfloat yCross = ballArray[i].ySpawn - ballArray[j].ySpawn;
			GLfloat xCross = ballArray[i].xSpawn - ballArray[j].xSpawn;

			if (sqrt((yCross*yCross) + (xCross*xCross)) <= (ballArray[i].radius + ballArray[j].radius)) {
				if (ballArray[j].xSpeed > 0){
					if (ballArray[i].xSpeed > 0){
						if (ballArray[j].xSpawn > ballArray[i].xSpawn) {
							ballArray[i].xSpeed = -ballArray[i].xSpeed;
							if (ballArray[j].xSpeed < 0) //to check
								ballArray[j].xSpawn += ballArray[i].radius;
						}

						else {
							ballArray[j].xSpeed = -ballArray[j].xSpeed;
							if (ballArray[j].xSpeed > 0) //to check
								ballArray[j].xSpawn -= ballArray[i].radius;
						}
					}
					else{
						ballArray[j].xSpeed = -ballArray[j].xSpeed;
						ballArray[i].xSpeed = -ballArray[i].xSpeed;
					}
				}
				else
				{
					if (ballArray[i].xSpeed < 0){
						if (ballArray[j].xSpawn < ballArray[i].xSpawn) {
							ballArray[i].xSpeed = -ballArray[i].xSpeed;
							if (ballArray[j].xSpeed > 0) //to check
								ballArray[j].xSpawn -= ballArray[i].radius;
						}
						else {
							ballArray[j].xSpeed = -ballArray[j].xSpeed;
							if (ballArray[j].xSpeed < 0) //to check
								ballArray[j].xSpawn += ballArray[i].radius;
						}
					}
					else{
						ballArray[j].xSpeed = -ballArray[j].xSpeed;
						ballArray[i].xSpeed = -ballArray[i].xSpeed;
					}
				}

				if (ballArray[j].ySpeed > 0)
				{
					if (ballArray[i].ySpeed > 0){
						if (ballArray[j].ySpawn > ballArray[i].ySpawn) {
							ballArray[i].ySpeed = -ballArray[i].ySpeed;
						}
						else {
							ballArray[j].ySpeed = -ballArray[j].ySpeed;
						}
					}
					else{
						ballArray[j].ySpeed = -ballArray[j].ySpeed;
						ballArray[i].ySpeed = -ballArray[i].ySpeed;
					}
				}
				else
				{
					if (ballArray[j].ySpeed < 0){
						if (ballArray[j].ySpawn < ballArray[i].ySpawn) {
							ballArray[i].ySpeed = -ballArray[i].ySpeed;
							ballArray[j].ySpawn += ballArray[i].radius;
						}
						else {
							ballArray[j].ySpeed = -ballArray[i].ySpeed;
							ballArray[j].ySpawn += ballArray[i].radius;
						}
					}
					else{
						ballArray[j].ySpeed = -ballArray[j].ySpeed;
						ballArray[i].ySpeed = -ballArray[i].ySpeed;
					}
				}
			}
		}
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

//using this keyboard function to change the background color (and show transparency)
void keyboard(unsigned char key, int x, int y)
{
	// switching colors at the press of a key
	GLfloat colors[][3] = { { 0.0f, 0.0f, 0.0f },{ 0.5f, 0.3f, 1.0f } };
	static int back;

	switch (key) {
	case 27:
		exit(0);
	default:
		back ^= 1;
		glClearColor(colors[back][0], colors[back][1], colors[back][2], 1.0f);
		glutPostRedisplay();
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

	//array balls
	for (int i = 0; i <= (sizeof(ballArray) / sizeof(struct BouncyBall)); i++) {

		ballArray[i].XMinBound = clipAreaXLeft + ballArray[i].radius;
		ballArray[i].XMaxBound = clipAreaXRight - ballArray[i].radius;
		ballArray[i].YMinBound = clipAreaYBottom + ballArray[i].radius;
		ballArray[i].YMaxBound = clipAreaYTop - ballArray[i].radius;
	}

}

/* Called back when the timer expired */
void Timer(int value) {
	glutPostRedisplay();    // Post a paint request to activate display()
	glutTimerFunc(refreshMillis, Timer, 0); // subsequent timer call at milliseconds
}

//creates 2 to 8 additional balls to ruin my life
 void* BallMaker(void *arg) {
	 
	 pthread_mutex_lock(&lock);
	 badIterator++;
	 int no = badIterator;

	//radius, xSpawn, ySpawn, XMax, Xmin, YMax,YMin, xSpeed, ySpeed, gravity lock
	 if (no % 2 == 0) {
		 struct BouncyBall a = { (0.10f) + ((no % 3)*0.01f), 0.35f * (no + 1), 0.56f * (no + 1), 0.0f,0.0f,0.0f,0.0f, 0.009f * (((no + 1) % 3) + 1), 0.0004f * (((no + 1) % 3) + 1), true };
		 ballArray[no] = a;
	 }
	 else {
		 struct BouncyBall a = { (0.05f) * ((no % 3)+1), -0.35f * (no + 1), 0.56f * (no + 1), 0.0f,0.0f,0.0f,0.0f, -0.009f * (((no + 1) % 3) + 1), -0.0004f * (((no + 1) % 3) + 1), true };
		 ballArray[no] = a;
	 }
	pthread_mutex_unlock(&lock);
	return NULL;
}

/// Main function
int main(int argc, char** argv) {

	int err;

	pthread_t tid;

	if (pthread_mutex_init(&lock, NULL) != 0) {
		printf("Your mutex is fucked.\n");
		return 1;
	}
	
	int r = random();
	numberBalls = r;

	struct BouncyBall firstBall = { 0.10f, -0.3f, 0.5f, 0.0f,0.0f,0.0f,0.0f, 0.03f, 0.002f, true };
	ballArray[0] = firstBall;

	for (int i = 0; i < r; i++) {
		pthread_create(&tid, NULL, BallMaker, NULL);
	}
	pthread_join(tid, NULL);

	//GL stuff
	glutInit(&argc, argv);            // Boot up GLUT
	glutInitDisplayMode(GLUT_DOUBLE); // For double-buffered mode
	glutInitWindowSize(windowWidth, windowHeight);  // Create a window with given height and width
	glutInitWindowPosition(windowPosX, windowPosY); // Where the window shows up on screen
	glutCreateWindow(title);      // Create window with given title
	glutDisplayFunc(display);     // For repaint
	glutReshapeFunc(reshape);     // For reshaping the window
	glutTimerFunc(0, Timer, 0);   // Start the timer
	initGL();                     // Our own OpenGL initialization
	glutKeyboardFunc(keyboard);   // Keyboard function for changing background
	glutMainLoop();               // Event loop

	pthread_exit(NULL);
	pthread_mutex_destroy(&lock);

	return 0;
}
