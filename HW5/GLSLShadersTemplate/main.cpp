#include <windows.h>
#include <stdio.h>
#include <GL/glew.h>

#include "gl.h"
#include "glu.h"
#include "glut.h"

#include "glm/ext.hpp"

#include "globals.h"
#include "Timer.h"
#include "mouse.h"
#include "GLShader.h"
#include <atlimage.h>


char window_name[512] = "Fragment Program Template (no mouse)";

Timer timer1;
bool useShadow = false;

bool moveLight = false;

/* Shader Values */
int period = 1;
float ambient_strength = 0.3;
float ambient_color[] = {118.0 / 255, 146.0 / 255, 255.0 / 255, 1.0};
extern float lightPosition[3];
float lightIntensity = 8.0f;
extern float eye_pos[3];
float specularColor[] = { 1.0, 1.0, 1.0 };
float specularHardness = 150.0f;

float specularColor1[] = { 0.1, 1.0, 0.2 };
float specularHardness1 = 1.0f;
int gridSize = 10; // Determines grid size for stars (10 = 50 stars)
int numStripes = 13;


/* Grass Shader Values */
GLuint texid;


/* Depth Map */
GLuint depthbo;
GLuint depthMap;


float timeElapsed = 0;
float flagSpeed = 100;
bool playFlag = true;

GLuint points_vbo = 0; /* vertex buffer object for the points */
GLuint texcoords_vbo = 0; /* vertex buffer object for the tex coords */
GLuint vao = 0; /* vertex array object */
GLuint shader_program = 0; /* shader program */
GLuint matrix_ID = 0; /* the mvp matrix id */

GLuint points_vbo1 = 0; /* vertex buffer object for the points */
GLuint texcoords_vbo1 = 0; /* vertex buffer object for the tex coords */
GLuint vao1 = 0; /* vertex array object */
GLuint shader_program1 = 0; /* shader program */

GLuint simple_shader = 0;
GLuint simple_shader1 = 0;

/* matrices */
extern glm::mat4 proj_mat;
extern glm::mat4 mv_mat;
extern glm::mat4 proj_light;
extern glm::mat4 mv_light;


float points[] = {
	-0.5 * 8, -0.5 * 8,
	-0.5 * 8, 0.5 * 8,
	0.5 * 8, 0.5 * 8,
	0.5 * 8, -0.5 * 8
};

float newPoints[50*50*8];


float tex_coords[] = {
	0.0, 0.0,
	0.0, 1.0,
	1.0, 1.0,
	1.0, 0.0
};

float newTexCoords[50*50*8];



/* this is the display() function that draws the object on the screen */
void display(void) {

	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, shadow_width, shadow_height);
	glBindFramebuffer(GL_FRAMEBUFFER, depthbo);
	glClear(GL_DEPTH_BUFFER_BIT);
	/*ConfigureShaderAndMatrices();
	RenderScene();*/


	computeLightMatrix();
	glm::mat4 lightmatrix = proj_light * mv_light;
	glm::mat4 biasMatrix(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
	);
	glm::mat4 depthBiasMVP = biasMatrix * lightmatrix;
	glBindVertexArray(vao);
	glDrawArrays(GL_QUADS, 0, 50 * 50 * 4);
	glUseProgram(simple_shader);

	GLint depthmvpsimple = glGetUniformLocation(simple_shader, "depth_mvp");
	glUniformMatrix4fv(depthmvpsimple, 1, GL_FALSE, glm::value_ptr(depthBiasMVP));

	GLint uniPeriod1 = glGetUniformLocation(simple_shader, "period");
	glUniform1i(uniPeriod1, period);

	GLint uniTime1 = glGetUniformLocation(simple_shader, "t");
	glUniform1f(uniTime1, timeElapsed);


	glBindVertexArray(vao1);
	glDrawArrays(GL_QUADS, 0, 4);

	glUseProgram(simple_shader1);

	GLint depthmvpsimple1 = glGetUniformLocation(simple_shader1, "depth_mvp");
	glUniformMatrix4fv(depthmvpsimple1, 1, GL_FALSE, glm::value_ptr(depthBiasMVP));



	

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, window_width, window_height);
	double time;
	char name[128];
	
	computeModelViewMatrix();

	timer1.startTimer();

	glClearColor(0.5,0.5,0.5,1);
	
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shader_program);

	glm::mat4 mvp = proj_mat * mv_mat;
	
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, texid);

	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, depthMap);

	GLint uniMvp = glGetUniformLocation(shader_program, "mvp");
	glUniformMatrix4fv(uniMvp, 1, GL_FALSE, glm::value_ptr(mvp));

	GLint uniPeriod = glGetUniformLocation(shader_program, "period");
	glUniform1i(uniPeriod, period);

	GLint uniStripes = glGetUniformLocation(shader_program, "num_stripes");
	glUniform1i(uniStripes, numStripes);

	GLint uniGridSize = glGetUniformLocation(shader_program, "grid_dim");
	glUniform1i(uniGridSize, gridSize);

	GLint uniTime = glGetUniformLocation(shader_program, "t");
	glUniform1f(uniTime, timeElapsed);

	GLint uniAmbStrength = glGetUniformLocation(shader_program, "ambient_strength");
	glUniform1f(uniAmbStrength, ambient_strength);

	GLint uniAmbColor = glGetUniformLocation(shader_program, "ambient_color");
	glUniform4f(uniAmbColor, ambient_color[0], ambient_color[1], ambient_color[2], ambient_color[3]);

	GLint uniLightPos = glGetUniformLocation(shader_program, "light_position");
	glUniform3f(uniLightPos, lightPosition[0], lightPosition[1], lightPosition[2]);

	GLint uniLightIntensity = glGetUniformLocation(shader_program, "light_intensity");
	glUniform1f(uniLightIntensity, lightIntensity);

	GLint uniSpecularHardness = glGetUniformLocation(shader_program, "specular_hardness");
	glUniform1f(uniSpecularHardness, specularHardness);

	GLint uniSpecularColor = glGetUniformLocation(shader_program, "specular_color");
	glUniform3f(uniSpecularColor, specularColor[0], specularColor[1], specularColor[2]);

	GLint uniEyePos = glGetUniformLocation(shader_program, "eye_pos");
	glUniform3f(uniEyePos, eye_pos[0], eye_pos[1], eye_pos[2]);

	// read in the mvp matrix
	// 
	// 
	

	// pass in the mvp matrix
//	glUniformMatrix4fv(matrix_ID, 1, GL_FALSE, &mvp[0][0]);


	glBindVertexArray(vao);
	glDrawArrays(GL_QUADS, 0, 50*50*4);

	glUseProgram(shader_program1);

	GLint uniMvp1 = glGetUniformLocation(shader_program1, "mvp");
	glUniformMatrix4fv(uniMvp1, 1, GL_FALSE, glm::value_ptr(mvp));

	GLint depthmvp1 = glGetUniformLocation(shader_program1, "depth_mvp");
	glUniformMatrix4fv(depthmvp1, 1, GL_FALSE, glm::value_ptr(depthBiasMVP));

	GLint uniTex = glGetUniformLocation(shader_program1, "base");
	glUniform1i(uniTex, 0);

	GLint uniShadow1 = glGetUniformLocation(shader_program1, "shadowMap");
	glUniform1i(uniShadow1, 1);

	GLint uniUseShadow = glGetUniformLocation(shader_program1, "useShadow");
	glUniform1i(uniUseShadow, useShadow);

	GLint uniAmbStrength1 = glGetUniformLocation(shader_program1, "ambient_strength");
	glUniform1f(uniAmbStrength1, ambient_strength);

	GLint uniAmbColor1 = glGetUniformLocation(shader_program1, "ambient_color");
	glUniform4f(uniAmbColor1, ambient_color[0], ambient_color[1], ambient_color[2], ambient_color[3]);

	GLint uniLightPos1 = glGetUniformLocation(shader_program1, "light_position");
	glUniform3f(uniLightPos1, lightPosition[0], lightPosition[1], lightPosition[2]);

	GLint uniLightIntensity1 = glGetUniformLocation(shader_program1, "light_intensity");
	glUniform1f(uniLightIntensity1, lightIntensity);

	GLint uniSpecularHardness1 = glGetUniformLocation(shader_program1, "specular_hardness");
	glUniform1f(uniSpecularHardness1, specularHardness1);

	GLint uniSpecularColor1 = glGetUniformLocation(shader_program1, "specular_color");
	glUniform3f(uniSpecularColor1, specularColor1[0], specularColor1[1], specularColor1[2]);

	GLint uniEyePos1 = glGetUniformLocation(shader_program1, "eye_pos");
	glUniform3f(uniEyePos1, eye_pos[0], eye_pos[1], eye_pos[2]);
	

	glBindVertexArray(vao1);
	glDrawArrays(GL_QUADS, 0, 4);


	timer1.stopTimer();

	time = timer1.getTime();
	timeElapsed += time * flagSpeed * playFlag;

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
		case '=':
			period++;
			break;
		case '-':
			period--;
			break;
		case 32:
			playFlag = !playFlag;
			break;
		default:
			break;
	}

	if (glutGetModifiers() == GLUT_ACTIVE_SHIFT) {
		moveLight = true;
	}
	else {
		moveLight = false;
	}

	return;
}

void redisplay(void) {
    glutPostRedisplay();
	return;
}


/* initializes extensions and reads in fragment program */
void init(void) {
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	/* load extensions */
	glewInit();
	initMouse();

	float xIncrement = 0.076;
	float yIncrement = 0.04;

	float uIncrement = 0.02;
	float vIncrement = 0.02;

	for (int i = 0; i < 50; i++)
	{
		for (int j = 0; j < 50; j++)
		{
			newPoints[i * 50 * 8 + j * 8] = -1.9 + i * xIncrement;
			newPoints[i * 50 * 8 + j * 8 + 1] = -1 + j * yIncrement;
			newPoints[i * 50 * 8 + j * 8 + 2] = -1.9 + i * xIncrement;
			newPoints[i * 50 * 8 + j * 8 + 3] = -1 + (j+1) * yIncrement;
			newPoints[i * 50 * 8 + j * 8 + 4] = -1.9 + (i+1) * xIncrement;
			newPoints[i * 50 * 8 + j * 8 + 5] = -1 + (j+1) * yIncrement;
			newPoints[i * 50 * 8 + j * 8 + 6] = -1.9 + (i+1) * xIncrement;
			newPoints[i * 50 * 8 + j * 8 + 7] = -1 + j * yIncrement;


			newTexCoords[i * 50 * 8 + j * 8] =  i * uIncrement;
			newTexCoords[i * 50 * 8 + j * 8 + 1] = j * vIncrement;
			newTexCoords[i * 50 * 8 + j * 8 + 2] = i * uIncrement;
			newTexCoords[i * 50 * 8 + j * 8 + 3] = (j + 1) * vIncrement;
			newTexCoords[i * 50 * 8 + j * 8 + 4] = (i + 1) * uIncrement;
			newTexCoords[i * 50 * 8 + j * 8 + 5] = (j + 1) * vIncrement;
			newTexCoords[i * 50 * 8 + j * 8 + 6] = (i + 1) * uIncrement;
			newTexCoords[i * 50 * 8 + j * 8 + 7] = j * vIncrement;


		}
	}

	glEnable(GL_TEXTURE_2D);

	/* generate vertex buffer object */
	glGenBuffers(1, &points_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glBufferData(GL_ARRAY_BUFFER, 50 * 50 * 8 * sizeof(float), newPoints, GL_STATIC_DRAW);

	glGenBuffers(1, &texcoords_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, texcoords_vbo);
	glBufferData(GL_ARRAY_BUFFER, 50 * 50 * 8 * sizeof(float), newTexCoords, GL_STATIC_DRAW);

	glGenBuffers(1, &points_vbo1);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo1);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), points, GL_STATIC_DRAW);

	glGenBuffers(1, &texcoords_vbo1);
	glBindBuffer(GL_ARRAY_BUFFER, texcoords_vbo1);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), tex_coords, GL_STATIC_DRAW);

	/* generate vertex arrays */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, texcoords_vbo);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);


	glGenVertexArrays(1, &vao1);
	glBindVertexArray(vao1);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo1);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, texcoords_vbo1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenFramebuffers(1, &depthbo);
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadow_width, shadow_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, depthbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	/* load shaders */
	shader_program = LoadShader("flagVert.glsl", "flagFrag.glsl");
	shader_program1 = LoadShader("grassVert.glsl", "grassFrag.glsl");
	simple_shader = LoadShader("flagSimple.glsl", "test.frag");
	simple_shader1 = LoadShader("grassSimple.glsl", "test.frag");
//	matrix_ID = glGetUniformLocation(shader_program, "MVP");


	
	
	CImage image;
	HRESULT h = image.Load((LPCTSTR)("grass1-albedo3.png"));

	int width, height;
	width = image.GetWidth();
	height = image.GetHeight();

	GLubyte* data;

	data = new GLubyte[width * height * 3];

	int i, j;
	int pitch = image.GetPitch();
	u08* ptr_src = (u08*)image.GetBits();
	u08* ptr_dst = data + (height - 1) * width * 3;


	/* note that CImage is upside down, so we start reading the framebuffer
	   from the top of the screen */

	for (j = 0; j < height; j++) {
		for (i = 0; i < width; i++) {
			*(ptr_dst++) = *(ptr_src + 2);
			*(ptr_dst++) = *(ptr_src + 1);
			*(ptr_dst++) = *(ptr_src);

			ptr_src += 3;
		}

		ptr_dst += -(width * 3) * 2;
		ptr_src += pitch - (width * 3);
	}

	image.Destroy();
	glGenTextures(1, &texid);
	glBindTexture(GL_TEXTURE_2D, texid);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	

	delete[] data;

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