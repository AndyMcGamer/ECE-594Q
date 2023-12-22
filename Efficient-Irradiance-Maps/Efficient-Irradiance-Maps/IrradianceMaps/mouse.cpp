#include <windows.h>
#include <stdio.h>

#include "gl.h"
#include "glu.h"
#include "glut.h"
#include "globals.h"

#include <glm/ext.hpp>

#include <math.h>

/* store the eye position */
float eye_theta;			/* the theta angle of the eye (the longitude).  it is positive in a counterclockwise direction when looking down -z */
float eye_phi;				/* the phi angle of the eye (the latitude).  i define 0 as the north pole. */
float eye_distance;			/* stores the distance to the object */
float eye_origin[3];		/* stores the origin of where I am looking at */
float eye_fov;				/* field of view of the eye */
float eye_pos[3];			/* the position of the eye, computed from eye_theta, eye_phi, eye_distance and eye_origin */

float up_vector[3];
float right_vector[3];

static float drag_x, drag_y;
static float translate_x, translate_y;
static float zoom_y;

/* matrices */
glm::mat4 proj_mat;
glm::mat4 mv_mat;

static int left_button = GLUT_UP, middle_button = GLUT_UP, right_button = GLUT_UP;



void computeUpAndRightVectors(void);

bool floatEqual(float a, float b) {
	double diff = a - b;

	/* return true if they are within DOUBLE_EPSILON of each other */
	return ((-FLOAT_EPSILON < diff) && (diff < FLOAT_EPSILON));
}

void printVector(char *phrase, float *v) {
	fprintf(stderr,"%s: %2.10f %2.10f %2.10f\n", phrase, v[0], v[1], v[2]);
}

void reshape(int w, int h) {
	/* set the viewport */
	glViewport(0,0,w,h);

#if 0
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(eye_fov, /* field of view in degree */
				   (double)w/(double)h, /* aspect ratio */
					0.1, 700);
#endif

	proj_mat = glm::perspective(glm::radians(eye_fov), (float)w / (float)h, 0.1f, 700.0f);

	return;
}

void computeProjectionMatrix(void) {
#if 0
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(eye_fov, /* field of view in degree */
				   (double)window_width/(double)window_height, /* aspect ratio */
					0.1, 700);
#endif

	proj_mat = glm::perspective(glm::radians(eye_fov), (float)window_width / (float)window_height, 0.1f, 700.0f);

	return;
}

void computeModelViewMatrix(void) {
#if 0
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	/* now move back by the distance to the eye */
	glTranslatef(0, 0, -eye_distance);

	/* now rotate by phi */
	glRotatef(-eye_phi, 1, 0, 0);

	/* now rotate by theta */
	glRotatef(-eye_theta, 0, 0, 1);


	/* translate to get the eye origin in the center of the coordinate system */
	glTranslatef(-eye_origin[0], -eye_origin[1], -eye_origin[2]);
#endif

	/* load identity */
	mv_mat = glm::mat4(1.0f);

	/* now apply the old operations using GLM instead */
	mv_mat = glm::translate(mv_mat, glm::vec3(0, 0, -eye_distance));
	mv_mat = glm::rotate(mv_mat, glm::radians(-eye_phi), glm::vec3(1, 0, 0));
	mv_mat = glm::rotate(mv_mat, glm::radians(-eye_theta), glm::vec3(0, 0, 1));
	mv_mat = glm::translate(mv_mat, glm::vec3(-eye_origin[0], -eye_origin[1], -eye_origin[2]));

	return;
}




float drag_constant = 1;


void beginDrag(int x, int y) {
    drag_x = x;
    drag_y = y;
}

void dragView(int x, int y) {
    eye_theta = eye_theta + drag_constant * (drag_x - x);
    eye_phi = eye_phi + drag_constant * (drag_y - y);

	drag_x = x;
	drag_y = y;
	
	computeUpAndRightVectors();
	return;
}



float zoom_constant = 0.1;// 0.2;

void beginZoom(int x, int y) {
	zoom_y = y;
}



/* these rotations all assume the right-hand rule, so if the vector is pointing towards the
   viewer the positive rotation is counterclockwise */
void rotateAboutX(float *vector, float angle) {
	float new_vector[3];

	/* note that the ith component of the vector stays the same */
	new_vector[1] = cos(angle * MY_PI/180.0) * vector[1] - sin(angle * MY_PI/180.0) * vector[2];
	new_vector[2] = sin(angle * MY_PI/180.0) * vector[1] + cos(angle * MY_PI/180.0) * vector[2];

	/* copy it back to up_vector */
	vector[1] = new_vector[1];
	vector[2] = new_vector[2];
}


void rotateAboutY(float *vector, float angle) {
	float new_vector[3];

	/* note that the jth component of the vector stays the same */
	new_vector[0] = cos(angle * MY_PI/180.0) * vector[0] + sin(angle * MY_PI/180.0) * vector[2];
	new_vector[2] = -sin(angle * MY_PI/180.0) * vector[0] + cos(angle * MY_PI/180.0) * vector[2];

	/* copy it back to up_vector */
	vector[0] = new_vector[0];
	vector[2] = new_vector[2];
}


void rotateAboutZ(float *vector, float angle) {
	float new_vector[3];

	/* note that the kth component of the vector stays the same */
	new_vector[0] = cos(angle * MY_PI/180.0) * vector[0] - sin(angle * MY_PI/180.0) * vector[1];
	new_vector[1] = sin(angle * MY_PI/180.0) * vector[0] + cos(angle * MY_PI/180.0) * vector[1];

	/* copy it back to up_vector */
	vector[0] = new_vector[0];
	vector[1] = new_vector[1];
}


float translate_constant = 0.003;

void beginTranslate(int x, int y) {
	translate_x = x;
	translate_y = y;
}

float dotProduct(float *v1, float *v2) {
	return ((v1[0] * v2[0]) + (v1[1] * v2[1]) + (v1[2] * v2[2]));
}

void scaleVector(float *new_vector, float *vector, float scale) {
	new_vector[0] = vector[0] * scale;
	new_vector[1] = vector[1] * scale;
	new_vector[2] = vector[2] * scale;
}

void addVector(float *res_vector, float *vector1, float *vector2) {
	res_vector[0] = vector1[0] + vector2[0];
	res_vector[1] = vector1[1] + vector2[1];
	res_vector[2] = vector1[2] + vector2[2];
	return;
}

void crossProduct(float *res_vector, float *vector1, float *vector2) {
	res_vector[0] = (vector1[1] * vector2[2]) - (vector1[2] * vector2[1]);
	res_vector[1] = (vector1[0] * vector2[2]) - (vector1[2] * vector2[0]);
	res_vector[2] = (vector1[0] * vector2[1]) - (vector1[1] * vector2[0]);
}


/* this computes the eye position.  it assumes that sight_vector has been
   computed, which is the vector from the eye_pos to the eye_origin */
void computeEyePosition(float *sight_vector) {
	float eye_offset[3];
	/* first scale the sight vector by the eye_distance */
	/* we negate it because we want the offset vector from the eye_origin to the eye_pos */
	scaleVector(eye_offset, sight_vector, -eye_distance);
	addVector(eye_pos, eye_origin, eye_offset);

	return;
}


void doZoom(int x, int y) {
	float sight_vector[3];
	eye_distance = eye_distance + zoom_constant * (y - zoom_y);
	zoom_y = y;

	/* compute the line of sight vector */
	sight_vector[0] = 0;	sight_vector[1] = 0;	sight_vector[2] = -1;
	
	rotateAboutX(sight_vector, eye_phi);
	rotateAboutZ(sight_vector, eye_theta);

	computeEyePosition(sight_vector);
}



void computeUpAndRightVectors(void) {
	float sight_vector[3];

	/* initialize the up vector */
	up_vector[0] = 0;	up_vector[1] = 1;	up_vector[2] = 0;

	rotateAboutX(up_vector, eye_phi);
	rotateAboutZ(up_vector, eye_theta);

	/* at this point i have a valid up vector */

	/* now compute the line of sight vector */
	sight_vector[0] = 0;	sight_vector[1] = 0;	sight_vector[2] = -1;
	
	rotateAboutX(sight_vector, eye_phi);
	rotateAboutZ(sight_vector, eye_theta);

	/* for debugging, make sure that these two are normal two each other */
	if (!floatEqual(dotProduct(up_vector, sight_vector), 0)) {
		fprintf(stderr,"ERROR! The dot product between the up_vector and sight_vector should be 0!\n");
		fflush(stderr);
		exit(-1);
	}

#if 0
	fprintf(stderr,"up_vector: %f %f %f\n", up_vector[0], up_vector[1], up_vector[2]);

	fprintf(stderr,"sight_vector: %f %f %f\n", sight_vector[0], sight_vector[1], sight_vector[2]);
#endif

	/* now take the cross product between sight vector and up vector to get the right vector */
	crossProduct(right_vector, sight_vector, up_vector);

	computeEyePosition(sight_vector);
}

void doTranslate(int x, int y) {
	float sight_vector[3];
	float new_up[3], new_right[3];

	/* update the eye_origin using the up and right vectors */
	scaleVector(new_up, up_vector, translate_constant * (y - translate_y));
	scaleVector(new_right, right_vector, translate_constant * (translate_x - x));

	addVector(eye_origin, eye_origin, new_right);
	addVector(eye_origin, eye_origin, new_up);

	translate_x = x;
	translate_y = y;

	/* now compute the line of sight vector */
	sight_vector[0] = 0;	sight_vector[1] = 0;	sight_vector[2] = -1;
	
	rotateAboutX(sight_vector, eye_phi);
	rotateAboutZ(sight_vector, eye_theta);
	
	computeEyePosition(sight_vector);
	return;
}




void mouseInput(int button, int state, int x, int y) {
    switch(button) {
		case GLUT_LEFT_BUTTON:
			left_button = state;
			if (state == GLUT_DOWN) {
				beginDrag(x, y);
			}
			else {
				dragView(x, y);
				glutPostRedisplay();
			}
			break;
		case GLUT_MIDDLE_BUTTON:
			middle_button = state;
			if (state == GLUT_DOWN) {
				beginZoom(x, y);
			}
			else {
				doZoom(x, y);
				glutPostRedisplay();
			}
			break;
		case GLUT_RIGHT_BUTTON:
			right_button = state;
			if (state == GLUT_DOWN) {
				beginTranslate(x, y);
			}
			else {
				doTranslate(x, y);
				glutPostRedisplay();
			}
			break;
	}

	return;
}

void mouseMotion(int x, int y) {
	if (left_button == GLUT_DOWN) {
		dragView(x, y);
	}

	if (middle_button == GLUT_DOWN) {
		doZoom(x, y);
	}

	if (right_button == GLUT_DOWN) {
		doTranslate(x,y);
	}

	/* if desired, print out the mouse information */
#if 0
	fprintf(stderr,"eye origin: %f %f %f\n", eye_origin[0], eye_origin[1], eye_origin[2]);
	fprintf(stderr,"eye position: %f %f %f\n", eye_pos[0], eye_pos[1], eye_pos[2]);

	fprintf(stderr,"eye theta: %f\n", eye_theta);
	fprintf(stderr,"eye phi: %f\n", eye_phi);

	fprintf(stderr,"eye_distance: %f\n", eye_distance);
	fprintf(stderr,"eye_fov: %f\n\n\n", eye_fov);

#endif

    glutPostRedisplay();
}


void initMouse(void) {
	/* first initialize the eye parameters */
	eye_theta = 180;
	eye_phi = 0;

	eye_distance = 10;
	eye_fov = 30;

	/* set the eye origin to (0,0,0) */
	eye_origin[0] = 0;
	eye_origin[1] = 0;
	eye_origin[2] = 0;
	
	computeUpAndRightVectors();

	/* set the glut callback functions */
    glutMouseFunc(mouseInput);
    glutMotionFunc(mouseMotion);


#if 0
	fprintf(stderr,"eye origin: %f %f %f\n", eye_origin[0], eye_origin[1], eye_origin[2]);
	fprintf(stderr,"eye position: %f %f %f\n", eye_pos[0], eye_pos[1], eye_pos[2]);

	fprintf(stderr,"eye theta: %f\n", eye_theta);
	fprintf(stderr,"eye phi: %f\n", eye_phi);

	fprintf(stderr,"eye_distance: %f\n", eye_distance);
	fprintf(stderr,"eye_fov: %f\n\n\n", eye_fov);
#endif

	return;
}


