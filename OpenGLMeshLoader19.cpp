#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <glut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#define GLUT_KEY_ESCAPE 27
#define DEG2RAD(a) (a * 0.0174532925)
//-----------------------------------------------------------------
void Collisions();
float CarX = 0.0f;
float CarZ = 0.77f;
bool game_over = false;
int lives = 3;
bool first_powerup_hit = false;
bool second_powerup_hit = false;
//-----------------------------------------------------------------

int WIDTH = 1280;
int HEIGHT = 720;

GLuint tex;
char title[] = "3D Model Loader Sample";

// 3D Projection Options
GLdouble fovy = 45.0;
GLdouble aspectRatio = (GLdouble)WIDTH / (GLdouble)HEIGHT;
GLdouble zNear = 0.1;
GLdouble zFar = 100;

class Vector
{
public:
	GLdouble x, y, z;
	Vector() {}
	Vector(GLdouble _x, GLdouble _y, GLdouble _z) : x(_x), y(_y), z(_z) {}
	//================================================================================================//
	// Operator Overloading; In C++ you can override the behavior of operators for you class objects. //
	// Here we are overloading the += operator to add a given value to all vector coordinates.        //
	//================================================================================================//
	void operator +=(float value)
	{
		x += value;
		y += value;
		z += value;
	}
};

Vector Eye(20, 5, 20);
Vector At(0, 0, 0);
Vector Up(0, 1, 0);

int cameraZoom = 0;

// Model Variables
Model_3DS model_house;
Model_3DS model_tree;
Model_3DS model_car;
Model_3DS model_obstacle;

// Textures
GLTexture tex_ground;
//-----------------------------------------------------camera-----------------------------------------------------------
class Vector3f {
public:
	float x, y, z;

	Vector3f(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) {
		x = _x;
		y = _y;
		z = _z;
	}

	Vector3f operator+(Vector3f& v) {
		return Vector3f(x + v.x, y + v.y, z + v.z);
	}

	Vector3f operator-(Vector3f& v) {
		return Vector3f(x - v.x, y - v.y, z - v.z);
	}

	Vector3f operator*(float n) {
		return Vector3f(x * n, y * n, z * n);
	}

	Vector3f operator/(float n) {
		return Vector3f(x / n, y / n, z / n);
	}

	Vector3f unit() {
		return *this / sqrt(x * x + y * y + z * z);
	}

	Vector3f cross(Vector3f v) {
		return Vector3f(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
};

class Camera {
public:
	Vector3f eye, center, up;

	Camera(float eyeX = 1.0f, float eyeY = 1.0f, float eyeZ = 1.0f, float centerX = 0.0f, float centerY = 0.0f, float centerZ = 0.0f, float upX = 0.0f, float upY = 1.0f, float upZ = 0.0f) {
		eye = Vector3f(eyeX, eyeY, eyeZ);
		center = Vector3f(centerX, centerY, centerZ);
		up = Vector3f(upX, upY, upZ);
	}

	void moveX(float d) {
		Vector3f right = up.cross(center - eye).unit();
		eye = eye + right * d;
		center = center + right * d;
	}


	void moveY(float d) {
		eye = eye + up.unit() * d;
		center = center + up.unit() * d;
	}

	void moveZ(float d) {
		Vector3f view = (center - eye).unit();
		eye = eye + view * d;
		center = center + view * d;
	}
	void follow(float d) {
		moveY(0.60 * d);
		moveZ(0.94 * d + 0.01f);

	}
	void change() {
		moveX(0.05);
		moveY(0.1);
		moveZ(0.93);
	}

	void rotateX(float a) {
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + up * sin(DEG2RAD(a));
		up = view.cross(right);
		center = eye + view;
	}

	void rotateY(float a) {
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + right * sin(DEG2RAD(a));
		right = view.cross(up);
		center = eye + view;
	}

	void look() {
		gluLookAt(
			eye.x, eye.y, eye.z,
			center.x, center.y, center.z,
			up.x, up.y, up.z
		);
	}

	void top_view() {
		eye.x = 0.492683f;
		eye.y = 1.37376;
		eye.z = 0.492683f;
		center.x = 0.35f;
		center.y = 0.35f;
		center.z = 0.35f;
	}

	void left_side_view() {
		eye.x = -0.627413f;
		eye.y = 0.364559f;
		eye.z = 1.36012f;
		center.x = 0.35f;
		center.y = 0.35f;
		center.z = 0.35f;
	}

	void right_side_view() {
		eye.x = 1.26221f;
		eye.y = 0.370151f;
		eye.z = -0.578557f;
		center.x = 0.35f;
		center.y = 0.35f;
		center.z = 0.35f;
	}


	void move_camera() {
		eye.z += 0.1;
		center.z += 0.1;
	}
};
Camera camera;
void setupCamera() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 640 / 480, 0.001, 100);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	camera.look();
}


//------------------------------------------------------draw functions---------------------------------------------------
void drawGround(double thickness) {
	glPushMatrix();
	glColor3d(0.38, 0.4, 0.42);
	glTranslated(0.0, 0.5 * thickness, 0.5);
	glScaled(1.0, thickness, 1.0);
	glutSolidCube(1);
	glPopMatrix();
}

void border_part() {
	glPushMatrix();
	glTranslated(0.45, 0.05, 0.65);
	glScaled(0.08, 0.1, 0.7);
	glutSolidCube(1);
	glPopMatrix();
}

void drawRightBorder() {
	//first border part
	glPushMatrix();
	glColor3d(0.0, 0.0, 0.0);
	border_part();
	glPopMatrix();

	//second border part
	glPushMatrix();
	glColor3d(1.0, 1.0, 0.0);
	glTranslated(0.0, 0.0, -0.7);
	border_part();
	glPopMatrix();

	//third border part
	glPushMatrix();
	glColor3d(0.0, 0.0, 0.0);
	glTranslated(0.0, 0.0, -1.4);
	border_part();
	glPopMatrix();

	//fourth border part
	glPushMatrix();
	glColor3d(1.0, 1.0, 0.0);
	glTranslated(0.0, 0.0, -2.35);
	glScaled(1.0, 1.0, 1.25);
	border_part();
	glPopMatrix();
}

void drawLeftBorder() {
	glPushMatrix();
	glTranslated(-0.9, 0.0, 0.0);
	drawRightBorder();
	glPopMatrix();
}

void drawRoadLinePart() {
	glPushMatrix();
	glColor3d(1.0, 1.0, 1.0);
	glTranslated(-0.45, 0.022, -0.25);
	glScaled(1.0, 0.02, 1.0);
	border_part();
	glPopMatrix();
}

void drawObstacle() {
	glPushMatrix();
	glColor3d(1.0, 0.0, 0.0);
	glTranslated(-0.2, 0.05, 0.0);
	glScaled(0.15, 0.25, 0.1);
	glutSolidCube(1);
	glPopMatrix();
}

void drawPowerup() {
	glPushMatrix();
	glColor3d(1.0, 1.0, 0.0);
	glTranslated(0.25, 0.05, -0.2);
	glScaled(0.15, 0.15, 0.15);
	glutSolidCube(1);
	glPopMatrix();
}

void drawSet() {
	glPushMatrix();

	//drawing ground
	glPushMatrix();
	glTranslated(0.0, 0.0, -2.0);
	glScaled(1, 1, 3);
	drawGround(0.02);
	glPopMatrix();

	//drawing borders
	drawRightBorder();
	drawLeftBorder();

	//drawing first road line
	drawRoadLinePart();

	//obstacle
	drawObstacle();

	//drawing second road line
	glPushMatrix();
	glTranslated(0.0, 0.0, -1.5);
	drawRoadLinePart();
	glPopMatrix();

	glPopMatrix();
}

void drawCar() {
	glPushMatrix();
	glTranslated(0, 0.2, 0.77);
	glutSolidSphere(0.2, 20, 20);
	glPopMatrix();
}

//=======================================================================
// Lighting Configuration Function
//=======================================================================
void InitLightSource()
{
	// Enable Lighting for this OpenGL Program
	glEnable(GL_LIGHTING);

	// Enable Light Source number 0
	// OpengL has 8 light sources
	glEnable(GL_LIGHT0);

	// Define Light source 0 ambient light
	GLfloat ambient[] = { 0.1f, 0.1f, 0.1, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

	// Define Light source 0 diffuse light
	GLfloat diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

	// Define Light source 0 Specular light
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	// Finally, define light source 0 position in World Space
	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

//=======================================================================
// Material Configuration Function
//======================================================================
void InitMaterial()
{
	// Enable Material Tracking
	glEnable(GL_COLOR_MATERIAL);

	// Sich will be assigneet Material Properties whd by glColor
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	// Set Material's Specular Color
	// Will be applied to all objects
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);

	// Set Material's Shine value (0->128)
	GLfloat shininess[] = { 96.0f };
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}

//=======================================================================
// OpengGL Configuration Function
//=======================================================================
void myInit(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	gluPerspective(fovy, aspectRatio, zNear, zFar);
	//*******************************//
	// fovy:			Angle between the bottom and top of the projectors, in degrees.			 //
	// aspectRatio:		Ratio of width to height of the clipping plane.							 //
	// zNear and zFar:	Specify the front and back clipping planes distances from camera.		 //
	//*******************************//

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();

	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
	//*******************************//
	// EYE (ex, ey, ez): defines the location of the camera.									 //
	// AT (ax, ay, az):	 denotes the direction where the camera is aiming at.					 //
	// UP (ux, uy, uz):  denotes the upward orientation of the camera.							 //
	//*******************************//

	InitLightSource();

	InitMaterial();

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_NORMALIZE);
}

//=======================================================================
// Render Ground Function
//=======================================================================
void RenderGround()
{
	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-20, 0, -20);
	glTexCoord2f(5, 0);
	glVertex3f(20, 0, -20);
	glTexCoord2f(5, 5);
	glVertex3f(20, 0, 20);
	glTexCoord2f(0, 5);
	glVertex3f(-20, 0, 20);
	glEnd();
	glPopMatrix();

	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}

//=======================================================================
// Display Function
//=======================================================================
void myDisplay(void)
{
	setupCamera();
	std::cout << "CarZ = " << CarZ << std::endl;
	std::cout << "CarX = " << CarX << std::endl;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	GLfloat lightIntensity[] = { 0.7, 0.7, 0.7, 1.0f };
	GLfloat lightPosition[] = { 0.0f, 100.0f, 0.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity);
	//-----------------------------------------------------drawing-------------------------------------------
	glPushMatrix();
	glRotated(45, 0, 1.0, 0);

	//test drawing
	glPushMatrix();
	glColor3d(1.0, 0.0, 0.0);
	glTranslated(CarX, 0.1, CarZ);
	glScaled(0.15, 0.25, 0.1);
	glutSolidCube(1);
	glPopMatrix();

	//drawing sets
		//drawing set 1
	glPushMatrix();
	if (!first_powerup_hit) {
		drawPowerup();
	}
	drawSet();
	glPopMatrix();

	//drawing set 2
	glPushMatrix();
	glTranslated(0.0, 0.0, -3);
	drawSet();
	glPopMatrix();
	if (!second_powerup_hit) {
		glPushMatrix();
		glTranslated(-0.2, 0.0, -3);
		drawPowerup();
		glPopMatrix();
	}

	//drawing set 3
	glPushMatrix();
	glTranslated(0.0, 0.0, -6);
	drawSet();
	glPopMatrix();

	//gameover handling
	if (lives == 0) {
		game_over = true;
		std::cout << "gameover!!!!!!" << std::endl;
	}


	//--------------------------------------------------------------------------------------------------------
	//// Draw Ground
	//RenderGround();

	// Draw Tree Model
	glPushMatrix();
	glTranslatef(10, 0, 0);
	glScalef(0.7, 0.7, 0.7);
	model_tree.Draw();
	glPopMatrix();

	//// Draw house Model
	//glPushMatrix();
	//glRotatef(90.f, 1, 0, 0);
	//model_house.Draw();
	//glPopMatrix();

	// Draw car Model
	glPushMatrix();
	glTranslatef(CarX, 0.01, CarZ);
	glTranslatef(0.075, 0.0, 0.0);
	glScalef(0.003, 0.003, 0.003);
	glRotatef(90, 0, 1, 0);
	model_car.Draw();
	glPopMatrix();

	//// Draw obstacle Model
	//glPushMatrix();
	//glTranslatef(0.0, 0.01, 0.2);
	//glScalef(10, 10, 10);
	//model_obstacle.Draw();
	//glPopMatrix();

	//sky box
	glPushMatrix();

	GLUquadricObj* qobj;
	qobj = gluNewQuadric();
	glTranslated(50, 0, 0);
	glRotated(90, 1, 0, 1);
	glBindTexture(GL_TEXTURE_2D, tex);
	gluQuadricTexture(qobj, true);
	gluQuadricNormals(qobj, GL_SMOOTH);
	gluSphere(qobj, 100, 100, 100);
	gluDeleteQuadric(qobj);


	glPopMatrix();
	glPopMatrix();


	glutSwapBuffers();
}

//=======================================================================
// Keyboard Function
//=======================================================================
void myKeyboard(unsigned char key, int x, int y)
{
	float d = 0.01;
	std::cout << "CarZ = " << CarZ << std::endl;
	std::cout << "CarX = " << CarX << std::endl;

	switch (key) {
	case'c': {  //go right
    //printf("%g \n", santaX);
		camera.change();
		break;
	}
	case 'w':
		camera.moveY(d);
		break;
	case 's':
		camera.moveY(-d);
		break;
	case 'a':
		camera.moveX(d);
		break;
	case 'd':
		camera.moveX(-d);
		break;
	case 'q':
		camera.moveZ(d);
		break;
	case 'e':
		camera.moveZ(-d);
		break;
	case 't': //camera top view
		camera.top_view();
		break;
	case 'z': //camera left side view
		camera.left_side_view();
		break;
	case 'x': //camera right side view
		camera.right_side_view();
		break;

	case 'b': {  //behind
		//printf("%g \n", CarZ);

		/*if (CarZ > 0.2)*/ 
			CarZ = CarZ - 0.1f;
			camera.follow(0.1f);
		//camera.move_camera();
		break;
	}

	case'm': {  //forward
		// printf("%g \n", santaZ); //
		CarZ = CarZ + 0.1f;
		camera.follow(-0.1f);
		break;
	}

	case'l': {  // goleft
		//printf("%g \n", CarX);
		//if (CarX < 0.4)
		CarX = CarX - 0.1f;
		if (CarX < -0.4) {
			lives = lives - 1;
			std::cout << "lives = " << lives << std::endl;
			CarX = 0.075;
		}
		break;
	}

	case'r': {  //go right
		//printf("%g \n", santaX);

		CarX = CarX + 0.1f;
		if (CarX > 0.4) {
			lives = lives - 1;
			std::cout << "lives = " << lives << std::endl;
			CarX = 0.075;
		}
		break;
	}


	case GLUT_KEY_ESCAPE:
		exit(EXIT_SUCCESS);
	}
	Collisions();
	glutPostRedisplay();
	//case 'w':
	//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//	break;
	//case 'r':
	//	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//	break;

}

void Special(int key, int x, int y) {
	float a = 1.0;

	switch (key) {
	case GLUT_KEY_UP:
		camera.rotateX(a);
		break;
	case GLUT_KEY_DOWN:
		camera.rotateX(-a);
		break;
	case GLUT_KEY_LEFT:
		camera.rotateY(a);
		break;
	case GLUT_KEY_RIGHT:
		camera.rotateY(-a);
		break;
	}

	glutPostRedisplay();
}

//=======================================================================
// Motion Function
//=======================================================================
void myMotion(int x, int y)
{
	y = HEIGHT - y;

	if (cameraZoom - y > 0)
	{
		Eye.x += -0.1;
		Eye.z += -0.1;
	}
	else
	{
		Eye.x += 0.1;
		Eye.z += 0.1;
	}

	cameraZoom = y;

	glLoadIdentity();	//Clear Model_View Matrix

	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);	//Setup Camera with modified paramters

	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glutPostRedisplay();	//Re-draw scene 
}

//=======================================================================
// Mouse Function
//=======================================================================
void myMouse(int button, int state, int x, int y)
{
	/*y = HEIGHT - y;

	if (state == GLUT_DOWN)
	{
		cameraZoom = y;
	}*/
}

//=======================================================================
// Reshape Function
//=======================================================================
void myReshape(int w, int h)
{
	if (h == 0) {
		h = 1;
	}

	WIDTH = w;
	HEIGHT = h;

	// set the drawable region of the window
	glViewport(0, 0, w, h);

	// set up the projection matrix 
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, (GLdouble)WIDTH / (GLdouble)HEIGHT, zNear, zFar);

	// go back to modelview matrix so we can move the objects about
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
}

//=======================================================================
// Assets Loading Function
//=======================================================================
void LoadAssets()
{
	// Loading Model files
	model_house.Load("Models/house/house.3DS");
	model_tree.Load("Models/tree/Tree1.3ds");
	model_car.Load("Models/car2/taxi.3DS");
	//model_obstacle.Load("Models/obstacle/traffic_cone.3DS");

	// Loading texture files
	tex_ground.Load("Textures/ground.bmp");
	loadBMP(&tex, "Textures/blu-sky-3.bmp", true);
}

//=======================================================================
// collisions Function
//=======================================================================
void Collisions() {
	//first obstacle collison (-0.2, 0.05, 0.0)
	if (CarX >= -0.3 && CarX < -0.1) {
		std::cout << "entered x area of block 1" << std::endl;
		if (CarZ >= -0.03 && CarZ < 0.27) {
			std::cout << "collision with obstacle occured" << std::endl;
			lives = lives - 1;
			std::cout << "lives = " << lives << std::endl;
			CarX = 0.0;
		}
	}
	
	//second obstacle collison
	if (CarX >= -0.2 && CarX < -0.1) {
		std::cout << "entered x area of block 2" << std::endl;
		if (CarZ >= -3.8 && CarZ < -3.6) {
			std::cout << "collision with obstacle occured" << std::endl;
			lives = lives - 1;
			std::cout << "lives = " << lives << std::endl;
			CarX = 0;
		}
	}

	//third obstacle collison
	if (CarX >= -0.2 && CarX < -0.1) {
		std::cout << "entered x area of block 3" << std::endl;
		if (CarZ >= -6.8 && CarZ < -6.6) {
			std::cout << "collision with obstacle occured" << std::endl;
			lives = lives - 1;
			std::cout << "lives = " << lives << std::endl;
			CarX = 0;
		}
	}

	////first powerup collision
	//if (CarX >= 0.05 && CarX < 0.34) {
	//	std::cout << "entered x area of powerup 1" << std::endl;
	//	if (CarZ >= -0.93 && CarZ < -0.78) {
	//		std::cout << "you took first powerup" << std::endl;
	//		first_powerup_hit = true;
	//		lives += 1;
	//		std::cout << "lives = " << lives << std::endl;
	//	}
	//}

	////second powerup collision
	//if (CarX >= -0.15 && CarX < 0.11) {
	//	std::cout << "entered x area of powerup 2" << std::endl;
	//	if (CarZ >= -3.93 && CarZ < -3.78) {
	//		std::cout << "you took second powerup" << std::endl;
	//		second_powerup_hit = true;
	//		lives += 1;
	//		std::cout << "lives = " << lives << std::endl;
	//	}
	//}

}

//=======================================================================
// Main Function
//=======================================================================
void main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(WIDTH, HEIGHT);

	glutInitWindowPosition(100, 90);

	glutCreateWindow(title);

	glutDisplayFunc(myDisplay);

	glutKeyboardFunc(myKeyboard);

	glutSpecialFunc(Special);

	glutMotionFunc(myMotion);

	glutMouseFunc(myMouse);

	glutReshapeFunc(myReshape);

	myInit();

	LoadAssets();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);

	glutMainLoop();
}