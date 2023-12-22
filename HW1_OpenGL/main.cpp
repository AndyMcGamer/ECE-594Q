#include <stdio.h>
#include <windows.h>
#include <atlstr.h>
#include <atlimage.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <tuple>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/norm.hpp>
#include <ctime>


/* include gl and glut files */
#include "gl.h"
#include "glu.h"
#include "glut.h"

#include "globals.h"


int dimensions[2];
int mazeHeight;
int cellSize;
std::vector<std::string> textures;
int numTex;
std::vector<std::pair<std::tuple<glm::vec4, glm::vec4, glm::vec4>, int>> triangles;

float aspect = (float)window_width / window_height;
float fov = 45.0f;

glm::vec3 eye = { 0, 5, 0 };
glm::vec3 center = { 0,0,1 };
glm::vec3 up = { 0,1,0 };
glm::mat4 modelview;
glm::mat4 projection;

glm::mat4 mvp;
glm::vec3 transform = { 0,0,0 };

glm::vec2 mpos;
float vertSpeed = 1.0f;
float horizSpeed = 2.0f;
float pitch, yaw = -90.0f;
bool frame1 = 0;

std::vector<GLuint> textureIds;

float moveSpeed = 0.5f;

int timeLimit = 60 * CLOCKS_PER_SEC;
clock_t startTime;
bool win = false;
bool gameOver = false;
int secs = 0;

/* this function checks for GL errors that might have cropped up and 
   breaks the program if they have */
void checkGLErrors(char *prefix) {
	GLenum error;
	if ((error = glGetError()) != GL_NO_ERROR) {
		fprintf(stderr,"%s: found gl error: %s\n",prefix, gluErrorString(error));
		//exit(-1);
	}
	
}

void readTextures(void) {
	
	textureIds.resize(numTex+1);
	
	for (int i = 1; i <= numTex; ++i)
	{
		glEnable(GL_TEXTURE_2D);

		glGenTextures(1, &textureIds[i]);

		CImage image;
		HRESULT h = image.Load((LPCTSTR)(textures[i].c_str()));

		if (!SUCCEEDED(h)) {
			std::cout << "failed" << std::endl;
			continue;
		}
		else {
			std::cout << "success" << std::endl;

		}

		

		int width, height;
		width = image.GetWidth();
		height = image.GetHeight();

		GLubyte* data;

		data = new GLubyte[width * height * 3+3];

		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				COLORREF pix = image.GetPixel(x, y);
				data[((height - 1) - y) * 3 * width + x * 3] = (GLubyte)GetRValue(pix);
				data[((height - 1) - y) * 3 * width + x * 3 + 1] = (GLubyte)GetGValue(pix);
				data[((height - 1) - y) * 3 * width + x * 3 + 2] = (GLubyte)GetBValue(pix);
			}
		}

		
		glBindTexture(GL_TEXTURE_2D, textureIds[i]);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
		delete[] data;
		

		
	}
}

void computeMVP(void) {
	modelview = glm::lookAt(eye, eye + center, up);
	projection = glm::perspective(fov, aspect, 0.01f, 1000.0f);
	mvp = projection * modelview;
}

void addTriangles(void) {
	int i = 0;
	for (std::pair<std::tuple<glm::vec4, glm::vec4, glm::vec4>, int> p : triangles) {
		std::tuple<glm::vec4, glm::vec4, glm::vec4> verts = p.first;
		glm::vec4 v1 = std::get<0>(verts);
		glm::vec4 v2 = std::get<1>(verts);
		glm::vec4 v3 = std::get<2>(verts);

		glLoadIdentity();
		gluPerspective(fov, aspect, 0.01, 1000.0);
		gluLookAt(eye.x, eye.y, eye.z, eye.x + center.x, eye.y + center.y, eye.z + center.z, up.x, up.y, up.z);
		
		
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, textureIds[p.second]);
		
		//glColor3f(1.0, 0, 0);
		glBegin(GL_TRIANGLES);
		if ((i % 2)) {
			glTexCoord2f(0, 0);
			//glColor3f(1.0, 0, 0);
			glVertex3f(v1.x, v1.y, v1.z);
			glTexCoord2f(1, 1);
			//glColor3f(0, 1.0, 0);
			glVertex3f(v2.x, v2.y, v2.z);
			glTexCoord2f(1, 0);
			//glColor3f(0, 0, 1.0);
			glVertex3f(v3.x, v3.y, v3.z);
		}
		else {
			glTexCoord2f(0,0);
			//glColor3f(1.0, 0, 0);
			glVertex3f(v1.x, v1.y, v1.z);
			glTexCoord2f(0, 1);
			//glColor3f(0, 1.0, 0);
			glVertex3f(v2.x, v2.y, v2.z);
			glTexCoord2f(1, 1);
			//glColor3f(0, 0, 1.0);
			glVertex3f(v3.x, v3.y, v3.z);
		}
		glEnd();
		++i;
	}
}


void display(void) {
	glutSetWindowTitle("MazeViewer");
	glEnable(GL_TEXTURE_2D);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	
	addTriangles();
	
	glBindTexture(GL_TEXTURE_2D, textureIds[numTex]);

	glBegin(GL_QUADS);
	glTexCoord2f(-dimensions[0] / 2.0, -dimensions[1] / 2.0);
	glVertex3f(-dimensions[0] * cellSize / 2.0, 0, -dimensions[1] * cellSize / 2.0);
	glTexCoord2f(-dimensions[0] / 2.0, dimensions[1] / 2.0);
	glVertex3f(-dimensions[0] * cellSize / 2.0, 0, dimensions[1] * cellSize / 2.0);
	glTexCoord2f(dimensions[0] / 2.0, dimensions[1] / 2.0);
	glVertex3f(dimensions[0] * cellSize / 2.0, 0, dimensions[1] * cellSize / 2.0);
	glTexCoord2f(dimensions[0] / 2.0, -dimensions[1] / 2.0);
	glVertex3f(dimensions[0] * cellSize / 2.0, 0, -dimensions[1] * cellSize / 2.0);
	glEnd();

	checkGLErrors("Errors in display()!\n");
	glutSwapBuffers();


	if (clock() - startTime > timeLimit && frame1) {
		if (!gameOver) {
			std::cout << "LOSE" << std::endl;
			startTime = clock();
			timeLimit = 3 * CLOCKS_PER_SEC;
			win = false;
			gameOver = true;
		}
		else {
			exit(1);
		}
		
	}
	else {
		if (!frame1) {
			startTime = clock();
		}
		else {
			if (clock() - startTime > CLOCKS_PER_SEC * secs) {
				std::cout << secs << std::endl;
				secs++;
			}
		}
	}
}

void redisplay(void) {
    glutPostRedisplay();
	if (clock() - startTime > timeLimit && frame1) {
		if (!gameOver) {
			std::cout << "LOSE" << std::endl;
			startTime = clock();
			timeLimit = 3 * CLOCKS_PER_SEC;
			win = false;
			gameOver = true;
		}
		else {
			exit(1);
		}

	}
	else {
		if (!frame1) {
			startTime = clock();
		}
		else {
			if (clock() - startTime > CLOCKS_PER_SEC * secs) {
				std::cout << secs << std::endl;
				secs++;
			}
		}
	}
	return;
}


void keyboard(unsigned char key, int x, int y) {
	switch (key) {
		case 27:
		case 'q':
		case 'Q':
			/* quit the program */
			exit(0);
			break;
		case 32:
			eye.y += 1;
			break;
		case 'w':
		case 'W':
			eye += glm::normalize(center - (glm::dot(center, up) * up)) * moveSpeed;
			break;
		case 's':
		case 'S':
			eye -= glm::normalize(center - (glm::dot(center, up) * up)) * moveSpeed;
			break;
		case 'a':
		case 'A':
			eye -= glm::normalize(glm::cross(center - (glm::dot(center, up) * up), up)) * moveSpeed;
			break;
		case 'd':
		case 'D':
			eye += glm::normalize(glm::cross(center - (glm::dot(center, up) * up), up)) * moveSpeed;
			break;
		case 'c':
		case 'C':
			eye.y -= 1;

			break;
		default:
			break;

	}

	if (!gameOver && (eye.x < -(dimensions[0]) * cellSize || eye.x >(dimensions[0]) * cellSize || eye.z < -(dimensions[1]) * cellSize || eye.z >(dimensions[1]) * cellSize)) {
		win = true;
		gameOver = true;
		std::cout << "WIN" << std::endl;
		startTime = clock();
		timeLimit = 3 * CLOCKS_PER_SEC;

	}

	return;
}


/* handle mouse interaction */
void mouseInput(int button, int state, int x, int y) {
    switch(button) {
		case GLUT_LEFT_BUTTON:
			if (state == GLUT_DOWN) {
				/* this is called when the button is first pushed down */
				//fprintf(stderr,"%d\t%d\n", x, y);
				glutPostRedisplay();
			}
			else {
				/* this is called when the button is released */
				//fprintf(stderr,"%d\t%d\n", x, y);
				glutPostRedisplay();
			}
			break;
		case GLUT_MIDDLE_BUTTON:
			break;
		case GLUT_RIGHT_BUTTON:
			break;
	}

	return;
}

void mouseMotion(int x, int y) {
	//fprintf(stderr,"%d\t%d\n", x, y);
	//glutWarpPointer(window_width / 2, window_height / 2);
	if (!frame1) {
		mpos = glm::vec2(x, y);
		startTime = clock();
		frame1 = 1;
	}

	float dx = x - mpos.x;
	float dy = mpos.y - y;
	
	

	dx *= horizSpeed;
	dy *= vertSpeed;

	yaw += dx;
	pitch += dy;

	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;

	

	center.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	center.y = sin(glm::radians(pitch));
	center.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	center = glm::normalize(center);

	//computeMVP();

	mpos.x = x;
	mpos.y = y;
	
	glutPostRedisplay();
}

int main(int argc, char **argv) {
	
	if (argc != 2) return 0;
	
	std::string line;
	std::string word;
	std::ifstream mazeFile(argv[1]);
	if (!mazeFile) return 0;
	while (std::getline(mazeFile, line)) 
	{
		line = line.substr(0, line.find("#"));
		if (line.find_first_not_of(' ') == std::string::npos) continue;
		
		std::stringstream stream(line);
		stream >> word;

		if (word == "DIMENSIONS") {
			
			stream >> dimensions[0] >> dimensions[1];
			
			std::cout << dimensions[0] << " " << dimensions[1] << std::endl;
		}

		if (word == "HEIGHT") {
			stream >> mazeHeight;
			std::cout << mazeHeight << std::endl;
			eye.y = mazeHeight / 2;
			transform = eye;
		}

		if (word == "CELL") {
			stream >> cellSize;
			std::cout << cellSize << std::endl;
		}

		if (word == "TEXTURES") {
			stream >> numTex;
			std::cout << numTex << std::endl;
			textures.resize(numTex + 1);
			for (int i = 0; i < numTex; ++i)
			{
				std::getline(mazeFile, line);
				line = line.substr(0, line.find("#"));
				if (line.find_first_not_of(' ') == std::string::npos) {
					--i;
					continue;
				}
				stream.clear();
				stream.str(std::string());
				stream << line;
				
				int index;
				stream >> index;
				stream >> textures[index];
				std::cout << index << " " << textures[index] << std::endl;
			}

		}
		if (word == "FLOORPLAN") {
			
			for (int i = 0; i < dimensions[0]*2; ++i)
			{
				std::getline(mazeFile, line);
				line = line.substr(0, line.find("#"));
				if (line.find_first_not_of(' ') == std::string::npos) {
					--i;
					continue;
				}
				stream.clear();
				stream.str(std::string());
				stream << line;
				//std::cout << line << std::endl;
				for (int j = 0; j < dimensions[1]; ++j)
				{
					
					if (i % 2) {
						// vert
						int tex;
						stream >> tex;
						if (!tex) continue;
						
						float x = (j - (dimensions[1] / 2.0)) * cellSize;
						float y = ((dimensions[0] - (i-1)) / 2.0) * cellSize;

						triangles.push_back(std::make_pair(std::make_tuple(glm::vec4(x, 0, y - cellSize, 1), glm::vec4(x, mazeHeight, y - cellSize, 1), glm::vec4(x, mazeHeight, y, 1)), tex));
						triangles.push_back(std::make_pair(std::make_tuple(glm::vec4(x, 0, y - cellSize, 1), glm::vec4(x, mazeHeight, y, 1), glm::vec4(x, 0, y, 1)), tex));
					}
					else {
						// horiz
						int tex;
						stream >> tex;
						if (!tex) continue;
						float x = (j - (dimensions[1] / 2.0)) * cellSize;
						float y = ((dimensions[0] - i) / 2.0) * cellSize;
						

						triangles.push_back(std::make_pair(std::make_tuple(glm::vec4(x, 0, y, 1), glm::vec4(x, mazeHeight, y, 1), glm::vec4(x + cellSize, mazeHeight, y, 1)), tex));
						triangles.push_back(std::make_pair(std::make_tuple(glm::vec4(x, 0, y, 1), glm::vec4(x + cellSize, mazeHeight, y, 1), glm::vec4(x + cellSize, 0, y, 1)), tex));
					}
				}
			}
		}
	}

    /* Initialize GLUT */
    glutInit(&argc, argv);
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );
    glutInitWindowSize(window_width, window_height);
	glutCreateWindow("window_name");
    glutInitWindowPosition(100,50);

	glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(NULL);
	
	/* set an idle function */
	glutIdleFunc(redisplay);

	/* init the mouse glut callback functions */
    glutMouseFunc(mouseInput);
    glutMotionFunc(mouseMotion);
    glutPassiveMotionFunc(mouseMotion);

	computeMVP();
	readTextures();
	glEnable(GL_DEPTH_TEST);

	
	std::cout << "================GAME==============" << std::endl;
	std::cout << "============TIME LIMIT============" << std::endl;
	std::cout << "                " << timeLimit/CLOCKS_PER_SEC << "              " << std::endl;
    /* Enter main loop */
	glutMainLoop();
	

	return 1;
}