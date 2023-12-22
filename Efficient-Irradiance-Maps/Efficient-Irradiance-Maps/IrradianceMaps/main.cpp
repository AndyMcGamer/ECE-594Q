#include <windows.h>
#include <stdio.h>
#include <GL/glew.h>
#include <fstream>

#include "gl.h"
#include "glu.h"
#include "glut.h"

#include "glm/ext.hpp"

#include "globals.h"
#include "Timer.h"
#include "mouse.h"
#include "GLShader.h"

/*
	Implementation of "An Efficient Representation for Irradiance Environment Maps" - Ramamoorthi and Hanrahan
	Light Probe files were taken from https://www.pauldebevec.com/Probes/.
*/


char window_name[512] = "Efficient Irradiance Map Demo";

Timer timer1;

GLuint points_vbo = 0; /* vertex buffer object for the points */
GLuint texcoords_vbo = 0; /* vertex buffer object for the tex coords */
GLuint normals_vbo = 0; /* vertex buffer object for normals */
GLuint vao = 0; /* vertex array object */
GLuint points_vbo1 = 0; /* vertex buffer object for the points */
GLuint texcoords_vbo1 = 0; /* vertex buffer object for the tex coords */
GLuint normals_vbo1 = 0; /* vertex buffer object for normals */
GLuint vao1 = 0; /* vertex array object */
GLuint shader_program = 0; /* shader program */
GLuint matrix_ID = 0; /* the mvp matrix id */

/* matrices */
extern glm::mat4 proj_mat;
extern glm::mat4 mv_mat;

extern float eye_pos[3];

/* Shader Values */

// Light
const float light_pos[] = { 0.0, 0, 10.0 };
const float light_intensity = 8.0f;
const float specularColor[] = { 1.0, 1.0, 1.0 };
const float specularHardness = 64.0f;
const float ambient_strength = 0.1;
const float ambient_color[] = { 118.0 / 255, 146.0 / 255, 255.0 / 255 };

// Cube
const bool useDiffuse = true;
const float environment_strength = 1.0;

// Plane
const bool useDiffuse1 = false;
const float environment_strength1 = 0.8;

/* Irradiance Constants and Values */

/*
Environment Maps and Sizes.
In the form: [filename] [size]

grace_probe.float 1000
rnl_probe.float 900
stpeters_probe.float 1500
campus_probe.float 640
*/

const char* env_map_file = "grace_probe.float";
const int map_size = 1000;



/* We use these values to precompute Ahat* Y
float A_hat[] = {
	MY_PI, // l = 0
	2.09439510239, // l = 1
	0.7853981634 // l = 2
};
*/

float coefficients[9 * 3];

// Real Spherical Harmonic Constants
float Y_constants[] = {
	0.28209479177, // Y_00
	0.4886025119,  // Y_1-1, Y_10, Y_11
	1.09254843059, // Y_21, Y_2-1, Y_2-2
	0.31539156525, // Y_20
	0.54627421529 // Y_22
};

// For finding irradiance E = summation ( Ahat * L_coeff * Y) 
// we can simplify by precomputing Ahat * Y
float c[] = {
	0.4290427654, // A_2 * Y_22
	0.51166335396, // A_1 * Y_1m / 2
	0.74312386829, // A_2 * Y_20 * 3
	0.88622692544, // A_0 * Y_00
	0.24770795609, // A_2 * Y_20

};

glm::mat4 irradiance_matrices[3];

int num_faces = 6;

// plane
float points1[] = {
	-5.0, -5.0, -10.0,
	-5.0, 5.0, -10.0,
	5.0, 5.0, -10.0,
	5.0, -5.0, -10.0,
};

float texCoords1[] = {
	0.0, 0.0,
	0.0, 1.0,
	1.0, 1.0,
	1.0, 0.0,
};

float normals1[] = {
	0, 0, 1.0,
	0, 0, 1.0,
	0, 0, 1.0,
	0, 0, 1.0,
};

// cube
float points[] = {
	-1.0, -1.0, 0,
	-1.0, 1.0, 0,
	1.0, 1.0, 0,
	1.0, -1.0, 0,

	-1.0, -1.0, -2.0,
	-1.0, 1.0, -2.0,
	-1.0, 1.0, 0,
	-1.0, -1.0, 0,

	1.0, 1.0, -2.0,
	1.0, -1.0, -2.0,
	1.0, -1.0, 0,
	1.0, 1.0, 0,
	
	-1.0, -1.0, -2.0,
	-1.0, 1.0, -2.0,
	1.0, 1.0, -2.0,
	1.0, -1.0, -2.0,
	
	-1.0, -1.0, -2.0,
	1.0, -1.0, -2.0,
	1.0, -1.0, 0,
	-1.0, -1.0, 0,
	
	-1.0, 1.0, -2.0,
	1.0, 1.0, -2.0,
	1.0, 1.0, 0,
	-1.0, 1.0, 0,
};


float tex_coords[] = {
	0.0, 0.0,
	0.0, 1.0,
	1.0, 1.0,
	1.0, 0.0,

	0.0, 0.0,
	0.0, 1.0,
	1.0, 1.0,
	1.0, 0.0,

	0.0, 0.0,
	0.0, 1.0,
	1.0, 1.0,
	1.0, 0.0,

	0.0, 0.0,
	0.0, 1.0,
	1.0, 1.0,
	1.0, 0.0,

	0.0, 0.0,
	0.0, 1.0,
	1.0, 1.0,
	1.0, 0.0,

	0.0, 0.0,
	0.0, 1.0,
	1.0, 1.0,
	1.0, 0.0,
};

float normals[] = {
	0, 0, 1.0,
	0, 0, 1.0,
	0, 0, 1.0,
	0, 0, 1.0,

	-1.0, 0, 0,
	-1.0, 0, 0,
	-1.0, 0, 0,
	-1.0, 0, 0,

	1.0, 0, 0,
	1.0, 0, 0,
	1.0, 0, 0,
	1.0, 0, 0,

	0, 0, -1.0,
	0, 0, -1.0,
	0, 0, -1.0,
	0, 0, -1.0,

	0, -1.0, 0,
	0, -1.0, 0,
	0, -1.0, 0,
	0, -1.0, 0,

	0, 1.0, 0,
	0, 1.0, 0,
	0, 1.0, 0,
	0, 1.0, 0,

};

inline bool is_little_endian() {
	const int value = 0x01;
	const void* address = static_cast<const void*>(&value);
	const unsigned char* least_significant_address = static_cast<const unsigned char*>(address);

	return (*least_significant_address == 0x01);
}

inline float sinc(const float theta) {
	if (fabs(theta) < 0.0001) return 1.0;
	return sin(theta) / theta;
}


/* this is the display() function that draws the object on the screen */
void display(void) {
	double time;
	char name[128];
	
	computeModelViewMatrix();

	timer1.startTimer();

	glClearColor(0.5,0.5,0.5,1);
	
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shader_program);

	glm::mat4 mvp = proj_mat * mv_mat;

	GLint uniMvp = glGetUniformLocation(shader_program, "mvp");
	glUniformMatrix4fv(uniMvp, 1, GL_FALSE, glm::value_ptr(mvp));

	GLint uniMvMat = glGetUniformLocation(shader_program, "mvMat");
	glUniformMatrix4fv(uniMvMat, 1, GL_FALSE, glm::value_ptr(mv_mat));

	GLint uniRMat = glGetUniformLocation(shader_program, "r_matrix");
	glUniformMatrix4fv(uniRMat, 1, GL_FALSE, glm::value_ptr(irradiance_matrices[0]));

	GLint uniGMat = glGetUniformLocation(shader_program, "g_matrix");
	glUniformMatrix4fv(uniGMat, 1, GL_FALSE, glm::value_ptr(irradiance_matrices[1]));

	GLint uniBMat = glGetUniformLocation(shader_program, "b_matrix");
	glUniformMatrix4fv(uniBMat, 1, GL_FALSE, glm::value_ptr(irradiance_matrices[2]));

	GLint uniEnvStrength = glGetUniformLocation(shader_program, "environmentStrength");
	glUniform1f(uniEnvStrength, environment_strength);

	GLint uniAmbStrength = glGetUniformLocation(shader_program, "ambientStrength");
	glUniform1f(uniAmbStrength, ambient_strength);

	GLint uniAmbColor = glGetUniformLocation(shader_program, "ambientColor");
	glUniform3f(uniAmbColor, ambient_color[0], ambient_color[1], ambient_color[2]);

	GLint uniLightPos = glGetUniformLocation(shader_program, "lightPos");
	glUniform3f(uniLightPos, light_pos[0], light_pos[1], light_pos[2]);

	GLint uniLightIntensity = glGetUniformLocation(shader_program, "lightIntensity");
	glUniform1f(uniLightIntensity, light_intensity);

	GLint uniSpecularHardness = glGetUniformLocation(shader_program, "specularHardness");
	glUniform1f(uniSpecularHardness, specularHardness);

	GLint uniSpecularColor = glGetUniformLocation(shader_program, "specularColor");
	glUniform3f(uniSpecularColor, specularColor[0], specularColor[1], specularColor[2]);

	GLint uniEyePos = glGetUniformLocation(shader_program, "eyePos");
	glUniform3f(uniEyePos, eye_pos[0], eye_pos[1], eye_pos[2]);

	GLint uniUseDiffuse = glGetUniformLocation(shader_program, "useDiffuse");
	glUniform1i(uniUseDiffuse, useDiffuse);


	glBindVertexArray(vao);
	glDrawArrays(GL_QUADS, 0, 4 * num_faces);

	glUseProgram(shader_program);


	uniMvp = glGetUniformLocation(shader_program, "mvp");
	glUniformMatrix4fv(uniMvp, 1, GL_FALSE, glm::value_ptr(mvp));

	uniMvMat = glGetUniformLocation(shader_program, "mvMat");
	glUniformMatrix4fv(uniMvMat, 1, GL_FALSE, glm::value_ptr(mv_mat));

	uniRMat = glGetUniformLocation(shader_program, "r_matrix");
	glUniformMatrix4fv(uniRMat, 1, GL_FALSE, glm::value_ptr(irradiance_matrices[0]));

	uniGMat = glGetUniformLocation(shader_program, "g_matrix");
	glUniformMatrix4fv(uniGMat, 1, GL_FALSE, glm::value_ptr(irradiance_matrices[1]));

	uniBMat = glGetUniformLocation(shader_program, "b_matrix");
	glUniformMatrix4fv(uniBMat, 1, GL_FALSE, glm::value_ptr(irradiance_matrices[2]));

	uniEnvStrength = glGetUniformLocation(shader_program, "environmentStrength");
	glUniform1f(uniEnvStrength, environment_strength1);

	uniAmbStrength = glGetUniformLocation(shader_program, "ambientStrength");
	glUniform1f(uniAmbStrength, ambient_strength);

	uniAmbColor = glGetUniformLocation(shader_program, "ambientColor");
	glUniform3f(uniAmbColor, ambient_color[0], ambient_color[1], ambient_color[2]);

	uniLightPos = glGetUniformLocation(shader_program, "lightPos");
	glUniform3f(uniLightPos, light_pos[0], light_pos[1], light_pos[2]);

	uniLightIntensity = glGetUniformLocation(shader_program, "lightIntensity");
	glUniform1f(uniLightIntensity, light_intensity);

	uniSpecularHardness = glGetUniformLocation(shader_program, "specularHardness");
	glUniform1f(uniSpecularHardness, specularHardness);

	uniSpecularColor = glGetUniformLocation(shader_program, "specularColor");
	glUniform3f(uniSpecularColor, specularColor[0], specularColor[1], specularColor[2]);

	uniEyePos = glGetUniformLocation(shader_program, "eyePos");
	glUniform3f(uniEyePos, eye_pos[0], eye_pos[1], eye_pos[2]);

	uniUseDiffuse = glGetUniformLocation(shader_program, "useDiffuse");
	glUniform1i(uniUseDiffuse, useDiffuse1);


	glBindVertexArray(vao1);
	glDrawArrays(GL_QUADS, 0, 4);



	timer1.stopTimer();

	time = timer1.getTime();

	sprintf_s(name, "%s: %.4lf sec/frame, %.2lf fps", window_name, time, 1.0/time);

	glutSetWindowTitle(name);

//	checkGLErrors("Errors in display()!\n");

	glutSwapBuffers();
}





/* handles keyboard input */
/* keys:
   'q' - quit
*/
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
		case 27:
		case 'q':
		case 'Q':
			exit(0);
			break;
		default:
			break;
	}

	return;
}

void redisplay(void) {
    glutPostRedisplay();
	return;
}

/* 
Precompute the 9 lighting distribution coefficients.
*/
void calculateLightingCoefficients(const char* filename, const int size) {
	float f;
	unsigned char temp[sizeof(float)];
	std::ifstream file(filename, std::ios::binary);

	int i, j, k;

	for (i = 0; i < size; i++)
	{
		for (j = 0; j < size; j++)
		{
			float u = (j - size / 2.0) / (size / 2.0);
			float v = (i - size / 2.0) / (size / 2.0);
			float r = sqrt(u * u + v * v);

			float theta = MY_PI * r;
			float phi = atan2(v, u) + MY_PI;

			float x = sin(theta) * cos(phi);
			float y = sin(theta) * sin(phi);
			float z = cos(theta);

			float sin_dtheta_dphi = sinc(theta) * (TWO_PI / size) * (TWO_PI / size);

			for (k = 0; k < 3; k++)
			{
				
				file.read(reinterpret_cast<char*>(temp), sizeof(float));
				if (!is_little_endian()) {
					std::swap(temp[0], temp[3]);
					std::swap(temp[1], temp[2]);
				}
				f = reinterpret_cast<float&>(temp);
				

				// L_00
				coefficients[0 * 3 + k] += f * Y_constants[0] * sin_dtheta_dphi;

				// L_1-1 L_10 L_11
				coefficients[1 * 3 + k] += f * Y_constants[1] * y * sin_dtheta_dphi;
				coefficients[2 * 3 + k] += f * Y_constants[1] * z * sin_dtheta_dphi;
				coefficients[3 * 3 + k] += f * Y_constants[1] * x * sin_dtheta_dphi;


				// L_2-2 L_2-1 L_20 L_21 L_22
				coefficients[4 * 3 + k] += f * Y_constants[2] * x * y * sin_dtheta_dphi;
				coefficients[5 * 3 + k] += f * Y_constants[2] * y * z * sin_dtheta_dphi;
				coefficients[6 * 3 + k] += f * Y_constants[3] * (3 * z * z - 1) * sin_dtheta_dphi;
				coefficients[7 * 3 + k] += f * Y_constants[2] * x * z * sin_dtheta_dphi;
				coefficients[8 * 3 + k] += f * Y_constants[4] * (x * x - y * y) * sin_dtheta_dphi;
				
			}
		}
	}
	
	file.close();

	/*for (i = 0; i < 9; i++)
	{
		fprintf(stderr, "%f %f %f\n", coefficients[i * 3 + 0], coefficients[i * 3 + 1], coefficients[i * 3 + 2]);
	}*/
	
	return;
}

void generateIrradianceMatrices() {
	for (size_t i = 0; i < 3; i++)
	{
		irradiance_matrices[i][0][0] = c[0] * coefficients[8 * 3 + i];
		irradiance_matrices[i][0][1] = c[0] * coefficients[4 * 3 + i];
		irradiance_matrices[i][0][2] = c[0] * coefficients[7 * 3 + i];
		irradiance_matrices[i][0][3] = c[1] * coefficients[3 * 3 + i];

		irradiance_matrices[i][1][0] = c[0] * coefficients[4 * 3 + i];
		irradiance_matrices[i][1][1] = -c[0] * coefficients[8 * 3 + i];
		irradiance_matrices[i][1][2] = c[0] * coefficients[5 * 3 + i];
		irradiance_matrices[i][1][3] = c[1] * coefficients[1 * 3 + i];

		irradiance_matrices[i][2][0] = c[0] * coefficients[7 * 3 + i];
		irradiance_matrices[i][2][1] = c[0] * coefficients[5 * 3 + i];
		irradiance_matrices[i][2][2] = c[2] * coefficients[6 * 3 + i];
		irradiance_matrices[i][2][3] = c[1] * coefficients[2 * 3 + i];

		irradiance_matrices[i][3][0] = c[1] * coefficients[3 * 3 + i];
		irradiance_matrices[i][3][1] = c[1] * coefficients[1 * 3 + i];
		irradiance_matrices[i][3][2] = c[1] * coefficients[2 * 3 + i];
		irradiance_matrices[i][3][3] = c[3] * coefficients[0 * 3 + i] - c[4] * coefficients[6 * 3 + i];
	}

	// fprintf(stderr, "%f %f %f %f\n", irradiance_matrices[0][3][0], irradiance_matrices[0][3][1], irradiance_matrices[0][3][2], irradiance_matrices[0][3][3]);
}


/* initializes extensions and reads in fragment program */
void init(void) {
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	/* load extensions */
	glewInit();
	initMouse();

	/* generate vertex buffer object */
	glGenBuffers(1, &points_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glBufferData(GL_ARRAY_BUFFER, 12 * num_faces * sizeof(float), points, GL_STATIC_DRAW);

	glGenBuffers(1, &texcoords_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, texcoords_vbo);
	glBufferData(GL_ARRAY_BUFFER, 8 * num_faces * sizeof(float), tex_coords, GL_STATIC_DRAW);

	glGenBuffers(1, &normals_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
	glBufferData(GL_ARRAY_BUFFER, 12 * num_faces * sizeof(float), normals, GL_STATIC_DRAW);


	/* generate vertex arrays */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, texcoords_vbo);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glGenBuffers(1, &points_vbo1);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo1);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), points1, GL_STATIC_DRAW);

	glGenBuffers(1, &texcoords_vbo1);
	glBindBuffer(GL_ARRAY_BUFFER, texcoords_vbo1);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), texCoords1, GL_STATIC_DRAW);

	glGenBuffers(1, &normals_vbo1);
	glBindBuffer(GL_ARRAY_BUFFER, normals_vbo1);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), normals1, GL_STATIC_DRAW);


	/* generate vertex arrays */
	glGenVertexArrays(1, &vao1);
	glBindVertexArray(vao1);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, texcoords_vbo1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, normals_vbo1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);


	/* load shaders */
	shader_program = LoadShader("diffusevert.glsl", "diffusefrag.glsl");
//	matrix_ID = glGetUniformLocation(shader_program, "MVP");

	calculateLightingCoefficients(env_map_file, map_size);

	generateIrradianceMatrices();

	return;
}


int main(int argc, char **argv) {
    /* Initialize GLUT */
    glutInit(&argc, argv);
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );
    glutInitWindowSize(window_width, window_height);
	glutCreateWindow("window_name");
    glutInitWindowPosition(100,50);

	glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);
	
	/* set an idle function */
	glutIdleFunc(redisplay);

	/* init the extensions and other */
	init();


	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);


    /* Enter main loop */
	glutMainLoop();

	return 1;
}