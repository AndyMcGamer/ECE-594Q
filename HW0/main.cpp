#include <stdio.h>
#include <windows.h>
#include <atlstr.h>
#include <atlimage.h>

/* include gl and glut files */
#include "gl.h"
#include "glu.h"
#include "glut.h"

#include "globals.h"

struct Color {
	GLubyte r, g, b;
};

GLubyte framebuffer[window_height * window_width * 3];
Color colors[10] = { {0,0,0}, {255,255,255}, {255,0,0}, {0,255,0}, {0,0,255}, {255,255,0}, {255,0,255}, {0,255,255}, {150,150,150}, {80, 25, 145} };
Color current = {255,255,255};

/* this function checks for GL errors that might have cropped up and 
   breaks the program if they have */
void checkGLErrors(char *prefix) {
	GLenum error;
	if ((error = glGetError()) != GL_NO_ERROR) {
		fprintf(stderr,"%s: found gl error: %s\n",prefix, gluErrorString(error));
		exit(-1);
	}
}

void display(void) {
	glutSetWindowTitle("SimpleDraw");
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawPixels(window_width, window_height, GL_RGB, GL_UNSIGNED_BYTE, framebuffer);

	checkGLErrors("Errors in display()!\n");
	glutSwapBuffers();
}

void redisplay(void) {
    glutPostRedisplay();
	return;
}

void readImage(void) {
	CImage image;
	image.Load(_T("wall1.tif"));
	unsigned w = min(window_width, image.GetWidth());
	unsigned h = min(window_height, image.GetHeight());

	for (size_t y = 0; y < h; y++)
	{
		for (size_t x = 0; x < w; x++)
		{
			COLORREF pixel = image.GetPixel(x, y);
			framebuffer[(window_height - y) * 3 * window_width + x * 3] = (GLubyte)GetRValue(pixel);
			framebuffer[(window_height - y) * 3 * window_width + x * 3 + 1] = (GLubyte)GetGValue(pixel);
			framebuffer[(window_height - y) * 3 * window_width + x * 3 + 2] = (GLubyte)GetBValue(pixel);
		}
	}

}

void saveImage(void) {
	CImage image;
	image.Create(window_width, window_height, 16);
	for (size_t y = 0; y < window_height; y++)
	{
		for (size_t x = 0; x < window_width; x++)
		{
			image.SetPixelRGB(x, y, (byte)framebuffer[(window_height - y) * 3 * window_width + x * 3], (byte)framebuffer[(window_height - y) * 3 * window_width + x * 3 + 1], (byte)framebuffer[(window_height - y) * 3 * window_width + x * 3 + 2]);
		}
	}
	image.Save(_T("newcat.png"));
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
		case 27:
		case 'q':
		case 'Q':
			/* quit the program */
			exit(0);
			break;
		case 48:
		case 49:
		case 50:
		case 51:
		case 52:
		case 53:
		case 54:
		case 55:
		case 56:
		case 57:
			current = colors[(int)key - 48];
			fprintf(stderr, "%d\t%d\t%d\n", current.r, current.g, current.b);
			break;
		case 32:
			memset(framebuffer, 0, sizeof(framebuffer));
			break;
		case 'w':
		case 'W':
			saveImage();
			break;
		case 'r':
		case 'R':
			readImage();
			break;
		default:
			break;
	}

	return;
}


/* handle mouse interaction */
void mouseInput(int button, int state, int x, int y) {
    switch(button) {
		case GLUT_LEFT_BUTTON:
			if (state == GLUT_DOWN) {
				/* this is called when the button is first pushed down */
				fprintf(stderr,"%d\t%d\n", x, y);
				
				glutPostRedisplay();
			}
			else {
				/* this is called when the button is released */
				fprintf(stderr,"%d\t%d\n", x, y);
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
	fprintf(stderr,"%d\t%d\n", x, y);
	framebuffer[(window_height - y) * 3 * window_width + x * 3] = current.r;
	framebuffer[(window_height - y) * 3 * window_width + x * 3 + 1] = current.g;
	framebuffer[(window_height - y) * 3 * window_width + x * 3 + 2] = current.b;
    glutPostRedisplay();
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
    glutReshapeFunc(NULL);
	
	/* set an idle function */
	glutIdleFunc(redisplay);

	/* init the mouse glut callback functions */
    glutMouseFunc(mouseInput);
    glutMotionFunc(mouseMotion);
    glutPassiveMotionFunc(NULL);

	readImage();

    /* Enter main loop */
	glutMainLoop();

	return 1;
}