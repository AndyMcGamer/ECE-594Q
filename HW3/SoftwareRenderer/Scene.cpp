#include "Scene.h"

extern FrameBuffer fb;

Matrix4x4 modelView, projection;

/* set the perspective projection matrix given the following values */
void setPerspectiveProjection(float eye_fov, float aspect_ratio, float znear, float zfar) {
	projection << 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1;
	
	Matrix4x4 perspective;

	float cothalf = 1.0f / tan((eye_fov * MY_PI / 180.0f) / 2.0f);

	perspective << cothalf / aspect_ratio, 0, 0, 0,
		0, cothalf, 0, 0,
		0, 0, (zfar + znear) / (znear - zfar), 2 * (znear * zfar) / (znear - zfar),
		0, 0, -1, 0;

	projection = perspective * projection;

	//projection.print();
}


/* set the modelview matrix with the given parameters */
void setModelviewMatrix(float *eye_pos, float eye_theta, float eye_phi) {
	modelView << 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1;
	
	eye_phi = -eye_phi * MY_PI / 180.0f;
	eye_theta = -eye_theta * MY_PI / 180.0f;

	Matrix4x4 rotyz, rotxy, translate;

	rotyz << 1, 0, 0, 0,
		0, cos(eye_phi), -sin(eye_phi), 0,
		0, sin(eye_phi), cos(eye_phi), 0,
		0, 0, 0, 1;

	rotxy << cos(eye_theta), -sin(eye_theta), 0, 0,
		sin(eye_theta), cos(eye_theta), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1;

	translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1,
		-eye_pos[2], 0, 0, 0, 1;
	
	
	modelView = rotyz * rotxy * translate * modelView;
	//modelView.print();
	
}

bool insideTriangle(Vector4& p, Vector4* _ssCoords) {
	bool insidepos = true;
	bool insideneg = true;

	for (size_t i = 0; i < 3; i++)
	{
		Vector4 pv, vv, ss, ss1;
		ss = _ssCoords[i];
		ss1 = _ssCoords[(i + 1) % 3];
		vv = ss1 - ss;
		pv = ss - p;
		
		//p.print();
		//pv.print();
		//vv.print();
		//pv.cross3(vv).print();
		Vector4 pv1;
		pv1 = pv;
		insidepos &= pv.cross3(vv).z() >= 0;
		insideneg &= pv1.cross3(vv).z() <= 0;
	}
	return insidepos || insideneg;
}

std::tuple<float, float, float> computeBarycentric(float x, float y, Vector4* v) {
	float c1 = (x * (v[1].y() - v[2].y()) + (v[2].x() - v[1].x()) * y + v[1].x() * v[2].y() - v[2].x() * v[1].y()) / (v[0].x() * (v[1].y() - v[2].y()) + (v[2].x() - v[1].x()) * v[0].y() + v[1].x() * v[2].y() - v[2].x() * v[1].y());
	float c2 = (x * (v[2].y() - v[0].y()) + (v[0].x() - v[2].x()) * y + v[2].x() * v[0].y() - v[0].x() * v[2].y()) / (v[1].x() * (v[2].y() - v[0].y()) + (v[0].x() - v[2].x()) * v[1].y() + v[2].x() * v[0].y() - v[0].x() * v[2].y());
	float c3 = 1 - c1 - c2;
	return { c1,c2,c3 };
}

Vector4 interpolate(float alpha, float beta, float gamma, Vector4 vert1, Vector4 vert2, Vector4 vert3)
{
	return (alpha * vert1 + beta * vert2 + gamma * vert3);
}

/* this implements the software rendering of the scene */ 
void Scene :: renderSceneSoftware(void) {
/* this is the function you will write.  you will need to take the linked list of triangles
   given by *original_head and draw them to the framebuffer fb */
	TriangleList* ptr;
	Vector4 verts[3];
	Vector4 ndcCoords[3];
	float depths[3];
	Vector4 ssCoords[3];
	Vector4 texCoords[3];
	int n = 0;
	for (ptr = original_head; ptr;) {
		n++;
		//ptr->t->renderOpenGL();
		
		for (size_t i = 0; i < 3; i++)
		{
			Vertex v = ptr->t->getVertex(i);
			verts[i] << v.x, v.y, v.z, v.w;
			texCoords[i] = ptr->t->getCoords(i);
		}

		
		for (size_t i = 0; i < 3; i++)
		{
			Matrix4x4 identity;
			identity << 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1;
			ndcCoords[i] = identity * projection * modelView * verts[i];
			depths[i] = ndcCoords[i].w();
		}

		for (auto& vert : ndcCoords) 
		{
			vert = vert / vert.w();
		}

		bool cull = false;
		for (auto vert : ndcCoords) {
			if (abs(vert.x()) > 1 || abs(vert.y()) > 1 || abs(vert.z()) > 1) {
				cull = true;
				break;
			}
		}

		if (cull) {
			ptr = ptr->next;
			continue;
		}

		
		for (size_t i = 0; i < 3; i++)
		{
			Vector4 ndc, ss;
			ndc = ndcCoords[i];
			ss << (ndc.x() + 1) * window_width * 0.5f, (ndc.y() + 1)* window_height * 0.5f, 0, 0;
			ssCoords[i] = ss;
		}

		int minX = (floor(min(ssCoords[0].x(), min(ssCoords[1].x(), ssCoords[2].x()))));
		int maxX = (ceil(max(ssCoords[0].x(), max(ssCoords[1].x(), ssCoords[2].x()))));

		int minY = (floor(min(ssCoords[0].y(), min(ssCoords[1].y(), ssCoords[2].y()))));
		int maxY = (ceil(max(ssCoords[0].y(), max(ssCoords[1].y(), ssCoords[2].y()))));

		if (minX < 0 || minY < 0 || maxX > window_width || maxY > window_height) {
			ptr = ptr->next;
			continue;
		}


		for (int y = minY; y < maxY; y++)
		{
			for (int x = minX; x < maxX; x++)
			{
				
				Vector4 pixel;
				pixel << x + 0.5, y + 0.5, 0, 0;
				if (!insideTriangle(pixel, ssCoords)) continue;
				auto [alpha, beta, gamma] = computeBarycentric(pixel.x(), pixel.y(), ssCoords);
				float interpolated_depth = 1 / (alpha / depths[0] + beta / depths[1] + gamma / depths[2]);
				float* depthptr = fb.getDepthPtr(x, y);
				if (depthptr[0] > interpolated_depth) {
					u08* colorptr = fb.getColorPtr(x, y);
					/*colorptr[0] = 255 * (n % 2);
					colorptr[1] = 255 * ((n + 1) % 2);
					colorptr[2] = 0;*/
					
					auto texCoord = interpolate(alpha / depths[0] * interpolated_depth, beta / depths[1] * interpolated_depth, gamma / depths[2] * interpolated_depth, texCoords[0], texCoords[1], texCoords[2]);
					ptr->t->getTexture()->getBilinearColor(texCoord.x(), texCoord.y(), colorptr);
					depthptr[0] = interpolated_depth;
				}
				
			}
		}

		

		ptr = ptr->next;
	}



	return;
}