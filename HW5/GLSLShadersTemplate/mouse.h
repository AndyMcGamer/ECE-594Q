#ifndef MOUSE_H
#define MOUSE_H

void reshape(int w, int h);
void computeModelViewMatrix(void);
void computeLightMatrix(void);
void initMouse(void);

void debugDrawEyeCoordinateSystem(void);

#endif		/* MOUSE_H */