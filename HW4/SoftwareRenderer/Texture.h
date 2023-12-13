#ifndef TEXTURE_H
#define TEXTURE_H

#include <windows.h>
#include <atlimage.h>

#include "globals.h"

#if OPENGL_TEST
/* this includes opengl calls in the code */

#include <stdio.h>

#include "gl.h"
#include "glu.h"
#include "glut.h"
#include "gl_ext.h"
#include "Vector4.h"

#endif


#include <math.h>


/* implements the texture mapping class so that the triangles
   can be textured in our software renderer */
class Texture {
	private:
		int width, height;	/* width and height for the texture */
		u08 *data;			/* contains the color data for the texture */

		/* for opengl only */
		GLuint	tex;		/* handle to opengl texture (only used by opengl */
		void copyTextureData(CImage *image); 

		void getFloatColor(int u, int v, float* color) {
			u08* ptr = data + (((v * width) + u) * 3);
			color[0] = (float)*(ptr);
			color[1] = (float)*(ptr + 1);
			color[2] = (float)*(ptr + 2);
		}

	public:
		/* constructor takes the name of the file to use as a texture */
		Texture(char *name);

		/* destructor */
		~Texture() {
			if (data)
				free(data);
		};

		void getBilinearColor(float x, float y, u08* color) {
			while (x > 1) x--;
			while (y > 1) y--;
			while (x < 0) x++;
			while (y < 0) y++;
			x = x * width;
			y = y * height;

			if (x < 0) x = 0;
			if (y < 0) y = 0;
			if (x > width) x = width;
			if (y > height) y = height;

			int u = floor(x);
			int v = floor(y);

			int u1 = ceil(x);
			int v1 = ceil(y);

			
			u08* u00 = data + (((v * width) + u) * 3);
			u08* u10 = data + (((v * width) + u1) * 3);
			u08* u01 = data + (((v1 * width) + u) * 3);
			u08* u11 = data + (((v1 * width) + u1) * 3);

			float a, b, c, d;
			a = u1 - x;
			b = x - u;
			c = v1 - y;
			d = y - v;

			Vector4 botleft, botright, topleft, topright, result;

			botleft << u00[0], u00[1], u00[2], 0;
			botright << u10[0], u10[1], u10[2], 0;
			topleft << u01[0], u01[1], u01[2], 0;
			topright << u11[0], u11[1], u11[2], 0;

			result = ((a * c) * botleft) + ((b * c) * botright) + ((a * d) * topleft) + ((b * d) * topright);

			color[0] = result.x();
			color[1] = result.y();
			color[2] = result.z();
		}

		
		/* switches between nearest neighbor and bilinear interpolation */
		void switchTextureFiltering(bool flag);


		/* for opengl only */
		void bind(void) {
			glBindTexture(GL_TEXTURE_2D, tex);
			glEnable(GL_TEXTURE_2D);
		};

		void release(void) {
			glDisable(GL_TEXTURE_2D);
		};

};

#endif		/* TEXTURE_H */