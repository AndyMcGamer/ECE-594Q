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

Vector4 interpolate_coords(float alpha, float beta, float gamma, Vector4 vert1, Vector4 vert2, Vector4 vert3)
{
	return (alpha * vert1 + beta * vert2 + gamma * vert3);
}

std::tuple<float, float, float> barycentricTexcoords(Vector4 a, Vector4 b, Vector4 c, Vector4 p) {
	Vector4 v0 = b - a;
	Vector4 v1 = c - a;
	Vector4 v2 = p - a;
	float d00 = v0.dot(v0);
	float d01 = v0.dot(v1);
	float d11 = v1.dot(v1);
	float d20 = v2.dot(v0);
	float d21 = v2.dot(v1);

	float d = (d00 * d11) - (d01 * d01);
	
	float beta = (d11 * d20 - d01 * d21) / d;
	float gamma = (d00 * d21 - d01 * d20) / d;
	float alpha = 1.0f - beta - gamma;

	return { alpha, beta, gamma };
}

void rasterizeTriangle(Triangle t, Vector4* clipCoords) {
	Vector4 ndcCoords[3];
	float depths[3];
	Vector4 ssCoords[3];
	Vector4 texCoords[3];
	for (size_t i = 0; i < 3; i++)
	{
		ndcCoords[i] << clipCoords[i].x() / clipCoords[i].w(), clipCoords[i].y() / clipCoords[i].w(), clipCoords[i].z() / clipCoords[i].w(), 1;
		depths[i] = clipCoords[i].w();
	}

	for (size_t i = 0; i < 3; i++)
	{
		ssCoords[i] << (ndcCoords[i].x() + 1) * window_width * 0.5f, (ndcCoords[i].y() + 1)* window_height * 0.5f, 0, 0;
		texCoords[i] = t.getCoords(i);
	}

	int minX = (floor(min(ssCoords[0].x(), min(ssCoords[1].x(), ssCoords[2].x()))));
	int maxX = (ceil(max(ssCoords[0].x(), max(ssCoords[1].x(), ssCoords[2].x()))));

	int minY = (floor(min(ssCoords[0].y(), min(ssCoords[1].y(), ssCoords[2].y()))));
	int maxY = (ceil(max(ssCoords[0].y(), max(ssCoords[1].y(), ssCoords[2].y()))));

	minX = max((int)ceil(minX - 0.5f), 0);
	minY = max((int)ceil(minY - 0.5f), 0);
	maxX = min((int)ceil(maxX - 0.5f), window_width);
	maxY = min((int)ceil(maxY - 0.5f), window_height);

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

				auto texCoord = interpolate_coords(alpha / depths[0] * interpolated_depth, beta / depths[1] * interpolated_depth, gamma / depths[2] * interpolated_depth, t.getCoords(0), t.getCoords(1), t.getCoords(2));
				t.getTexture()->getBilinearColor(texCoord.x(), texCoord.y(), colorptr);
				/*colorptr[0] = 255;
				colorptr[1] = 0;
				colorptr[2] = 0;*/
				depthptr[0] = interpolated_depth;
			}

		}
	}
	return;
}

void rasterizeEdgeWalk(Triangle t, Vector4* clipCoords) {
	Vector4 ndcCoords[3];
	float depths[3];
	Vector4 ssCoords[3];
	Vector4 origCoords[3];
	for (size_t i = 0; i < 3; i++)
	{
		ndcCoords[i] << clipCoords[i].x() / clipCoords[i].w(), clipCoords[i].y() / clipCoords[i].w(), clipCoords[i].z() / clipCoords[i].w(), 1;
		depths[i] = clipCoords[i].w();
		
	}

	for (size_t i = 0; i < 3; i++)
	{
		ssCoords[i] << (ndcCoords[i].x() + 1) * window_width * 0.5f, (ndcCoords[i].y() + 1)* window_height * 0.5f, 0, 0;
		origCoords[i] = ssCoords[i];
	}


	if (ssCoords[2].y() < ssCoords[0].y()) std::swap(ssCoords[0], ssCoords[2]);
	if (ssCoords[1].y() < ssCoords[0].y()) std::swap(ssCoords[1], ssCoords[0]);
	if (ssCoords[2].y() < ssCoords[1].y()) std::swap(ssCoords[2], ssCoords[1]);


	//top triangle
	if (fabs(ssCoords[1].y() - ssCoords[0].y()) <= 0.0001f) {
		if (ssCoords[1].x() < ssCoords[0].x()) std::swap(ssCoords[0], ssCoords[1]);
		
		// Draw top
		drawTriangleTop(t, ssCoords, depths, origCoords);
	}
	// bottom triangle
	else if (fabs(ssCoords[1].y() - ssCoords[2].y()) <= 0.0001f) {
		if (ssCoords[2].x() < ssCoords[1].x()) std::swap(ssCoords[2], ssCoords[1]);
		// Draw bottom
		drawTriangleBottom(t, ssCoords, depths, origCoords);
	}
	else {
		
		// draw both
		float midpt_alpha = (ssCoords[1].y() - ssCoords[0].y()) / (ssCoords[2].y() - ssCoords[0].y());
		auto midpt = lerp(ssCoords[0], ssCoords[2], midpt_alpha);
		if (ssCoords[1].x() < midpt.x()) {
			// right side
			Vector4 topCoords[3];
			Vector4 botCoords[3];
			topCoords[0] = ssCoords[1];
			topCoords[1] = midpt;
			topCoords[2] = ssCoords[2];

			drawTriangleTop(t, topCoords, depths, origCoords);

			botCoords[0] = ssCoords[0];
			botCoords[1] = ssCoords[1];
			botCoords[2] = midpt;

			drawTriangleBottom(t, botCoords, depths, origCoords);

		}
		else {
			// left side
			Vector4 topCoords[3];
			Vector4 botCoords[3];
			topCoords[0] = midpt;
			topCoords[1] = ssCoords[1];
			topCoords[2] = ssCoords[2];

			drawTriangleTop(t, topCoords, depths, origCoords);

			botCoords[0] = ssCoords[0];
			botCoords[1] = midpt;
			botCoords[2] = ssCoords[1];

			drawTriangleBottom(t, botCoords, depths, origCoords);
		}
	}
}

void drawTriangleTop(Triangle t, Vector4* ssCoords, float* depths, Vector4* origCoords) {
	
	float dy = ssCoords[2].y() - ssCoords[0].y();
	// 1 = left
	float dx1 = (ssCoords[2].x() - ssCoords[0].x()) / dy;
	float dx2 = (ssCoords[2].x() - ssCoords[1].x()) / dy;

	//fprintf(stderr, "%f %f\n", dx1, dx2);

	float xStart = max((int)ceil(ssCoords[0].x() - 0.5f), 0);
	float xEnd = min((int)ceil(ssCoords[1].x()), window_width);

	int yStart = max((int)ceil(ssCoords[0].y() - 0.5f), 0);
	int yEnd = min((int)ceil(ssCoords[2].y()), window_height);

	for (int y = yStart; y < yEnd; y++)
	{
		int start = max((int)ceil(xStart - 0.5f), 0);
		int end = min((int)ceil(xEnd), window_width);
		for (int x = start; x < end; x++) {
			Vector4 pixel;
			pixel << x + 0.5, y + 0.5, 0, 0;
			auto [alpha, beta, gamma] = computeBarycentric(pixel.x(), pixel.y(), origCoords);
			float interpolated_depth = 1 / (alpha / depths[0] + beta / depths[1] + gamma / depths[2]);
			float* depthptr = fb.getDepthPtr(x, y);
			if (depthptr[0] > interpolated_depth) {
				u08* colorptr = fb.getColorPtr(x, y);

				auto texCoord = interpolate_coords(alpha / depths[0] * interpolated_depth, beta / depths[1] * interpolated_depth, gamma / depths[2] * interpolated_depth, t.getCoords(0), t.getCoords(1), t.getCoords(2));
				t.getTexture()->getBilinearColor(texCoord.x(), texCoord.y(), colorptr);
				/*colorptr[0] = 255;
				colorptr[1] = 0;
				colorptr[2] = 0;*/
				depthptr[0] = interpolated_depth;
			}
		}
		xStart += dx1;
		xEnd += dx2;
	}

}

void drawTriangleBottom(Triangle t, Vector4* ssCoords, float* depths, Vector4* origCoords) {
	float dy = ssCoords[1].y() - ssCoords[0].y();
	
	float dx1 = (ssCoords[1].x() - ssCoords[0].x()) / dy;
	float dx2 = (ssCoords[2].x() - ssCoords[0].x()) / dy;

	float xStart = max((int)ceil(ssCoords[0].x() - 0.5f), 0);
	float xEnd = min((int)ceil(ssCoords[0].x()), window_width);

	int yStart = max((int)ceil(ssCoords[0].y() - 0.5f), 0);
	int yEnd = min((int)ceil(ssCoords[2].y()), window_height);

	for (int y = yStart; y < yEnd; y++)
	{
		int start = max((int)ceil(xStart - 0.5f), 0);
		int end = min((int)ceil(xEnd), window_width);
		for (int x = start; x < end; x++) {
			Vector4 pixel;
			pixel << x + 0.5, y + 0.5, 0, 0;
			auto [alpha, beta, gamma] = computeBarycentric(pixel.x(), pixel.y(), origCoords);
			float interpolated_depth = 1 / (alpha / depths[0] + beta / depths[1] + gamma / depths[2]);
			float* depthptr = fb.getDepthPtr(x, y);
			if (depthptr[0] > interpolated_depth) {
				u08* colorptr = fb.getColorPtr(x, y);

				auto texCoord = interpolate_coords(alpha / depths[0] * interpolated_depth, beta / depths[1] * interpolated_depth, gamma / depths[2] * interpolated_depth, t.getCoords(0), t.getCoords(1), t.getCoords(2));
				t.getTexture()->getBilinearColor(texCoord.x(), texCoord.y(), colorptr);
				/*colorptr[0] = 255;
				colorptr[1] = 0;
				colorptr[2] = 0;*/
				depthptr[0] = interpolated_depth;
			}
		}
		xStart += dx1;
		xEnd += dx2;
	}
}

Vector4 lerp(Vector4 a, Vector4 b, float t) {
	return a + (b - a) * t;
}

/* this implements the software rendering of the scene */ 
void Scene :: renderSceneSoftware(void) {
/* this is the function you will write.  you will need to take the linked list of triangles
   given by *original_head and draw them to the framebuffer fb */
	TriangleList* ptr;
	
	Vector4 verts[3];
	Vector4 clipCoords[3];
	//clipTriangles();
	destroyList(&clipped_head, &clipped_tail);
	for (ptr = original_head; ptr;) {
		for (size_t i = 0; i < 3; i++)
		{
			Vertex v = ptr->t->getVertex(i);
			verts[i] << v.x, v.y, v.z, v.w;
		}
		for (size_t i = 0; i < 3; i++)
		{
			Matrix4x4 identity;
			identity << 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1;
			clipCoords[i] = identity * projection * modelView * verts[i];
		}
		
		if (clipCoords[0].x() < -clipCoords[0].w() && clipCoords[1].x() < -clipCoords[1].w() && clipCoords[2].x() < -clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].y() < -clipCoords[0].w() && clipCoords[1].y() < -clipCoords[1].w() && clipCoords[2].y() < -clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].z() < -clipCoords[0].w() && clipCoords[1].z() < -clipCoords[1].w() && clipCoords[2].z() < -clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].x() > clipCoords[0].w() && clipCoords[1].x() > clipCoords[1].w() && clipCoords[2].x() > clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].y() > clipCoords[0].w() && clipCoords[1].y() > clipCoords[1].w() && clipCoords[2].y() > clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].z() > clipCoords[0].w() && clipCoords[1].z() > clipCoords[1].w() && clipCoords[2].z() > clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}
		Vertex v0(clipCoords[0].x(), clipCoords[0].y(), clipCoords[0].z(), clipCoords[0].w());
		Vertex v1(clipCoords[1].x(), clipCoords[1].y(), clipCoords[1].z(), clipCoords[1].w());
		Vertex v2(clipCoords[2].x(), clipCoords[2].y(), clipCoords[2].z(), clipCoords[2].w());
		Triangle* t = new Triangle(&v0, &(v1), &(v2));
		t->setCoords(0, ptr->t->getCoords(0).x(), ptr->t->getCoords(0).y());
		t->setCoords(1, ptr->t->getCoords(1).x(), ptr->t->getCoords(1).y());
		t->setCoords(2, ptr->t->getCoords(2).x(), ptr->t->getCoords(2).y());

		t->setTexture(ptr->t->getTexture());

		addTriangle(&clipped_head, &clipped_tail, t);

		ptr = ptr->next;
	}

	TriangleList* temphead = clipped_head;
	TriangleList* temptail = clipped_tail;

	clipped_head = NULL;
	clipped_tail = NULL;

	// v0 is the outside coord, v1 is after v0 and v2 is after v1
	auto clip1_posx = [this](Vector4 v0, Vector4 v1, Vector4 v2, Vector4 tex0, Vector4 tex1, Vector4 tex2, Texture* tex) {
		float t1 = (v0.w() - v0.x()) / (v1.x() + v0.w() - v0.x() - v1.w());
		float t2 = (v0.w() - v0.x()) / (v2.x() + v0.w() - v0.x() - v2.w());

		Vector4 v0v1 = lerp(v0, v1, t1);
		Vector4 v0v2 = lerp(v0, v2, t2);

		Vector4 tex01 = lerp(tex0, tex1, t1);
		Vector4 tex02 = lerp(tex0, tex2, t2);

		Vertex v01(v0v1.x(), v0v1.y(), v0v1.z(), v0v1.w());
		Vertex v02(v0v2.x(), v0v2.y(), v0v2.z(), v0v2.w());
		Vertex v1_vert(v1.x(), v1.y(), v1.z(), v1.w());
		Vertex v2_vert(v2.x(), v2.y(), v2.z(), v2.w());
		Triangle* tri1 = new Triangle(&v01, &v1_vert, &v02);
		Triangle* tri2 = new Triangle(&v02, &v1_vert, &v2_vert);

		tri1->setTexture(tex);
		tri2->setTexture(tex);

		tri1->setCoords(0, tex01);
		tri1->setCoords(1, tex1);
		tri1->setCoords(2, tex02);

		tri2->setCoords(0, tex02);
		tri2->setCoords(1, tex1);
		tri2->setCoords(2, tex2);

		addTriangle(&clipped_head, &clipped_tail, tri1);
		addTriangle(&clipped_head, &clipped_tail, tri2);

	};

	// v0 is the inside coord, v1 is after v0 and v2 is after v1
	auto clip2_posx = [this](Vector4 v0, Vector4 v1, Vector4 v2, Vector4 tex0, Vector4 tex1, Vector4 tex2, Texture* tex) {
		float t1 = (v1.w() - v1.x()) / (v0.x() + v1.w() - v1.x() - v0.w());
		float t2 = (v2.w() - v2.x()) / (v0.x() + v1.w() - v2.x() - v0.w());

		Vector4 v1v0 = lerp(v1, v0, t1);
		Vector4 v2v0 = lerp(v2, v0, t2);

		Vector4 tex10 = lerp(tex1, tex0, t1);
		Vector4 tex20 = lerp(tex2, tex0, t2);

		Vertex v10(v1v0.x(), v1v0.y(), v1v0.z(), v1v0.w());
		Vertex v20(v2v0.x(), v2v0.y(), v2v0.z(), v2v0.w());
		Vertex v0_vert(v0.x(), v0.y(), v0.z(), v0.w());

		Triangle* tri1 = new Triangle(&v0_vert, &v10, &v20);

		tri1->setTexture(tex);

		tri1->setCoords(0, tex0);
		tri1->setCoords(1, tex10);
		tri1->setCoords(2, tex20);

		addTriangle(&clipped_head, &clipped_tail, tri1);

	};

	// +x plane
	for (ptr = temphead; ptr;) {
		for (size_t i = 0; i < 3; i++)
		{
			Vertex v = ptr->t->getVertex(i);
			clipCoords[i] << v.x, v.y, v.z, v.w;
		}
		
		if (clipCoords[0].x() < -clipCoords[0].w() && clipCoords[1].x() < -clipCoords[1].w() && clipCoords[2].x() < -clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].y() < -clipCoords[0].w() && clipCoords[1].y() < -clipCoords[1].w() && clipCoords[2].y() < -clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].z() < -clipCoords[0].w() && clipCoords[1].z() < -clipCoords[1].w() && clipCoords[2].z() < -clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].x() > clipCoords[0].w() && clipCoords[1].x() > clipCoords[1].w() && clipCoords[2].x() > clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].y() > clipCoords[0].w() && clipCoords[1].y() > clipCoords[1].w() && clipCoords[2].y() > clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].z() > clipCoords[0].w() && clipCoords[1].z() > clipCoords[1].w() && clipCoords[2].z() > clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].x() > clipCoords[0].w()) {
			if (clipCoords[1].x() > clipCoords[1].w()) {
				clip2_posx(clipCoords[2], clipCoords[0], clipCoords[1], ptr->t->getCoords(2), ptr->t->getCoords(0), ptr->t->getCoords(1), ptr->t->getTexture());
			}
			else if (clipCoords[2].x() > clipCoords[2].w()) {
				clip2_posx(clipCoords[1], clipCoords[2], clipCoords[0], ptr->t->getCoords(1), ptr->t->getCoords(2), ptr->t->getCoords(0), ptr->t->getTexture());
			}
			else {
				clip1_posx(clipCoords[0], clipCoords[1], clipCoords[2], ptr->t->getCoords(0), ptr->t->getCoords(1), ptr->t->getCoords(2), ptr->t->getTexture());
			}
		}
		else if (clipCoords[1].x() > clipCoords[1].w()) {
			if (clipCoords[2].x() > clipCoords[2].w()) {
				clip2_posx(clipCoords[0], clipCoords[1], clipCoords[2], ptr->t->getCoords(0), ptr->t->getCoords(1), ptr->t->getCoords(2), ptr->t->getTexture());
			}
			else {
				clip1_posx(clipCoords[1], clipCoords[2], clipCoords[0], ptr->t->getCoords(1), ptr->t->getCoords(2), ptr->t->getCoords(0), ptr->t->getTexture());
			}
		}
		else if (clipCoords[2].x() > clipCoords[2].w()) {
			clip1_posx(clipCoords[2], clipCoords[0], clipCoords[1], ptr->t->getCoords(2), ptr->t->getCoords(0), ptr->t->getCoords(1), ptr->t->getTexture());
		}
		else {
			Vertex v0(ptr->t->getVertex(0));
			Vertex v1(ptr->t->getVertex(1));
			Vertex v2(ptr->t->getVertex(2));

			Triangle* t = new Triangle(&v0, &v1, &v2);

			t->setTexture(ptr->t->getTexture());
			t->setCoords(0, ptr->t->getCoords(0));
			t->setCoords(1, ptr->t->getCoords(1));
			t->setCoords(2, ptr->t->getCoords(2));

			addTriangle(&clipped_head, &clipped_tail, t);
		}
		ptr = ptr->next;
	}

	destroyList(&temphead, &temptail);
	temphead = clipped_head;
	temptail = clipped_tail;

	clipped_head = NULL;
	clipped_tail = NULL;

	auto clip1_negx = [this](Vector4 v0, Vector4 v1, Vector4 v2, Vector4 tex0, Vector4 tex1, Vector4 tex2, Texture* tex) {
		float t1 = (-v0.w() - v0.x()) / (v1.x() + v1.w() - v0.x() - v0.w());
		float t2 = (-v0.w() - v0.x()) / (v2.x() + v2.w() - v0.x() - v0.w());

		Vector4 v0v1 = lerp(v0, v1, t1);
		Vector4 v0v2 = lerp(v0, v2, t2);

		Vector4 tex01 = lerp(tex0, tex1, t1);
		Vector4 tex02 = lerp(tex0, tex2, t2);

		Vertex v01(v0v1.x(), v0v1.y(), v0v1.z(), v0v1.w());
		Vertex v02(v0v2.x(), v0v2.y(), v0v2.z(), v0v2.w());
		Vertex v1_vert(v1.x(), v1.y(), v1.z(), v1.w());
		Vertex v2_vert(v2.x(), v2.y(), v2.z(), v2.w());
		Triangle* tri1 = new Triangle(&v01, &v1_vert, &v02);
		Triangle* tri2 = new Triangle(&v02, &v1_vert, &v2_vert);

		tri1->setTexture(tex);
		tri2->setTexture(tex);

		tri1->setCoords(0, tex01);
		tri1->setCoords(1, tex1);
		tri1->setCoords(2, tex02);

		tri2->setCoords(0, tex02);
		tri2->setCoords(1, tex1);
		tri2->setCoords(2, tex2);

		addTriangle(&clipped_head, &clipped_tail, tri1);
		addTriangle(&clipped_head, &clipped_tail, tri2);

	};

	// v0 is the inside coord, v1 is after v0 and v2 is after v1
	auto clip2_negx = [this](Vector4 v0, Vector4 v1, Vector4 v2, Vector4 tex0, Vector4 tex1, Vector4 tex2, Texture* tex) {
		float t1 = (-v1.w() - v1.x()) / (v0.x() + v0.w() - v1.x() - v1.w());
		float t2 = (-v2.w() - v2.x()) / (v0.x() + v0.w() - v2.x() - v2.w());

		Vector4 v1v0 = lerp(v1, v0, t1);
		Vector4 v2v0 = lerp(v2, v0, t2);

		Vector4 tex10 = lerp(tex1, tex0, t1);
		Vector4 tex20 = lerp(tex2, tex0, t2);

		Vertex v10(v1v0.x(), v1v0.y(), v1v0.z(), v1v0.w());
		Vertex v20(v2v0.x(), v2v0.y(), v2v0.z(), v2v0.w());
		Vertex v0_vert(v0.x(), v0.y(), v0.z(), v0.w());

		Triangle* tri1 = new Triangle(&v0_vert, &v10, &v20);

		tri1->setTexture(tex);

		tri1->setCoords(0, tex0);
		tri1->setCoords(1, tex10);
		tri1->setCoords(2, tex20);

		addTriangle(&clipped_head, &clipped_tail, tri1);

	};

	// -x plane
	for (ptr = temphead; ptr;) {
		for (size_t i = 0; i < 3; i++)
		{
			Vertex v = ptr->t->getVertex(i);
			clipCoords[i] << v.x, v.y, v.z, v.w;
		}

		if (clipCoords[0].x() < -clipCoords[0].w() && clipCoords[1].x() < -clipCoords[1].w() && clipCoords[2].x() < -clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].y() < -clipCoords[0].w() && clipCoords[1].y() < -clipCoords[1].w() && clipCoords[2].y() < -clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].z() < -clipCoords[0].w() && clipCoords[1].z() < -clipCoords[1].w() && clipCoords[2].z() < -clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].x() > clipCoords[0].w() && clipCoords[1].x() > clipCoords[1].w() && clipCoords[2].x() > clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].y() > clipCoords[0].w() && clipCoords[1].y() > clipCoords[1].w() && clipCoords[2].y() > clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].z() > clipCoords[0].w() && clipCoords[1].z() > clipCoords[1].w() && clipCoords[2].z() > clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].x() < -clipCoords[0].w()) {
			if (clipCoords[1].x() < -clipCoords[1].w()) {
				clip2_negx(clipCoords[2], clipCoords[0], clipCoords[1], ptr->t->getCoords(2), ptr->t->getCoords(0), ptr->t->getCoords(1), ptr->t->getTexture());
			}
			else if (clipCoords[2].x() < -clipCoords[2].w()) {
				clip2_negx(clipCoords[1], clipCoords[2], clipCoords[0], ptr->t->getCoords(1), ptr->t->getCoords(2), ptr->t->getCoords(0), ptr->t->getTexture());
			}
			else {
				clip1_negx(clipCoords[0], clipCoords[1], clipCoords[2], ptr->t->getCoords(0), ptr->t->getCoords(1), ptr->t->getCoords(2), ptr->t->getTexture());
			}
		}
		else if (clipCoords[1].x() < -clipCoords[1].w()) {
			if (clipCoords[2].x() < -clipCoords[2].w()) {
				clip2_negx(clipCoords[0], clipCoords[1], clipCoords[2], ptr->t->getCoords(0), ptr->t->getCoords(1), ptr->t->getCoords(2), ptr->t->getTexture());
			}
			else {
				clip1_negx(clipCoords[1], clipCoords[2], clipCoords[0], ptr->t->getCoords(1), ptr->t->getCoords(2), ptr->t->getCoords(0), ptr->t->getTexture());
			}
		}
		else if (clipCoords[2].x() < -clipCoords[2].w()) {
			clip1_negx(clipCoords[2], clipCoords[0], clipCoords[1], ptr->t->getCoords(2), ptr->t->getCoords(0), ptr->t->getCoords(1), ptr->t->getTexture());
		}
		else {
			Vertex v0(ptr->t->getVertex(0));
			Vertex v1(ptr->t->getVertex(1));
			Vertex v2(ptr->t->getVertex(2));

			Triangle* t = new Triangle(&v0, &v1, &v2);

			t->setTexture(ptr->t->getTexture());
			t->setCoords(0, ptr->t->getCoords(0));
			t->setCoords(1, ptr->t->getCoords(1));
			t->setCoords(2, ptr->t->getCoords(2));

			addTriangle(&clipped_head, &clipped_tail, t);
		}
		ptr = ptr->next;
	}

	destroyList(&temphead, &temptail);
	temphead = clipped_head;
	temptail = clipped_tail;

	clipped_head = NULL;
	clipped_tail = NULL;

	// v0 is the outside coord, v1 is after v0 and v2 is after v1
	auto clip1_posy = [this](Vector4 v0, Vector4 v1, Vector4 v2, Vector4 tex0, Vector4 tex1, Vector4 tex2, Texture* tex) {
		float t1 = (v0.w() - v0.y()) / (v1.y() + v0.w() - v0.y() - v1.w());
		float t2 = (v0.w() - v0.y()) / (v2.y() + v0.w() - v0.y() - v2.w());

		Vector4 v0v1 = lerp(v0, v1, t1);
		Vector4 v0v2 = lerp(v0, v2, t2);

		Vector4 tex01 = lerp(tex0, tex1, t1);
		Vector4 tex02 = lerp(tex0, tex2, t2);

		Vertex v01(v0v1.x(), v0v1.y(), v0v1.z(), v0v1.w());
		Vertex v02(v0v2.x(), v0v2.y(), v0v2.z(), v0v2.w());
		Vertex v1_vert(v1.x(), v1.y(), v1.z(), v1.w());
		Vertex v2_vert(v2.x(), v2.y(), v2.z(), v2.w());
		Triangle* tri1 = new Triangle(&v01, &v1_vert, &v02);
		Triangle* tri2 = new Triangle(&v02, &v1_vert, &v2_vert);

		tri1->setTexture(tex);
		tri2->setTexture(tex);

		tri1->setCoords(0, tex01);
		tri1->setCoords(1, tex1);
		tri1->setCoords(2, tex02);

		tri2->setCoords(0, tex02);
		tri2->setCoords(1, tex1);
		tri2->setCoords(2, tex2);

		addTriangle(&clipped_head, &clipped_tail, tri1);
		addTriangle(&clipped_head, &clipped_tail, tri2);

	};

	// v0 is the inside coord, v1 is after v0 and v2 is after v1
	auto clip2_posy = [this](Vector4 v0, Vector4 v1, Vector4 v2, Vector4 tex0, Vector4 tex1, Vector4 tex2, Texture* tex) {
		float t1 = (v1.w() - v1.y()) / (v0.y() + v1.w() - v1.y() - v0.w());
		float t2 = (v2.w() - v2.y()) / (v0.y() + v1.w() - v2.y() - v0.w());

		Vector4 v1v0 = lerp(v1, v0, t1);
		Vector4 v2v0 = lerp(v2, v0, t2);

		Vector4 tex10 = lerp(tex1, tex0, t1);
		Vector4 tex20 = lerp(tex2, tex0, t2);

		Vertex v10(v1v0.x(), v1v0.y(), v1v0.z(), v1v0.w());
		Vertex v20(v2v0.x(), v2v0.y(), v2v0.z(), v2v0.w());
		Vertex v0_vert(v0.x(), v0.y(), v0.z(), v0.w());

		Triangle* tri1 = new Triangle(&v0_vert, &v10, &v20);

		tri1->setTexture(tex);

		tri1->setCoords(0, tex0);
		tri1->setCoords(1, tex10);
		tri1->setCoords(2, tex20);

		addTriangle(&clipped_head, &clipped_tail, tri1);

	};

	// +y plane
	for (ptr = temphead; ptr;) {
		for (size_t i = 0; i < 3; i++)
		{
			Vertex v = ptr->t->getVertex(i);
			clipCoords[i] << v.x, v.y, v.z, v.w;
		}
		
		if (clipCoords[0].x() < -clipCoords[0].w() && clipCoords[1].x() < -clipCoords[1].w() && clipCoords[2].x() < -clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].y() < -clipCoords[0].w() && clipCoords[1].y() < -clipCoords[1].w() && clipCoords[2].y() < -clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].z() < -clipCoords[0].w() && clipCoords[1].z() < -clipCoords[1].w() && clipCoords[2].z() < -clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].x() > clipCoords[0].w() && clipCoords[1].x() > clipCoords[1].w() && clipCoords[2].x() > clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].y() > clipCoords[0].w() && clipCoords[1].y() > clipCoords[1].w() && clipCoords[2].y() > clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].z() > clipCoords[0].w() && clipCoords[1].z() > clipCoords[1].w() && clipCoords[2].z() > clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].y() > clipCoords[0].w()) {
			if (clipCoords[1].y() > clipCoords[1].w()) {
				clip2_posy(clipCoords[2], clipCoords[0], clipCoords[1], ptr->t->getCoords(2), ptr->t->getCoords(0), ptr->t->getCoords(1), ptr->t->getTexture());
			}
			else if (clipCoords[2].y() > clipCoords[2].w()) {
				clip2_posy(clipCoords[1], clipCoords[2], clipCoords[0], ptr->t->getCoords(1), ptr->t->getCoords(2), ptr->t->getCoords(0), ptr->t->getTexture());
			}
			else {
				clip1_posy(clipCoords[0], clipCoords[1], clipCoords[2], ptr->t->getCoords(0), ptr->t->getCoords(1), ptr->t->getCoords(2), ptr->t->getTexture());
			}
		}
		else if (clipCoords[1].y() > clipCoords[1].w()) {
			if (clipCoords[2].y() > clipCoords[2].w()) {
				clip2_posy(clipCoords[0], clipCoords[1], clipCoords[2], ptr->t->getCoords(0), ptr->t->getCoords(1), ptr->t->getCoords(2), ptr->t->getTexture());
			}
			else {
				clip1_posy(clipCoords[1], clipCoords[2], clipCoords[0], ptr->t->getCoords(1), ptr->t->getCoords(2), ptr->t->getCoords(0), ptr->t->getTexture());
			}
		}
		else if (clipCoords[2].y() > clipCoords[2].w()) {
			clip1_posy(clipCoords[2], clipCoords[0], clipCoords[1], ptr->t->getCoords(2), ptr->t->getCoords(0), ptr->t->getCoords(1), ptr->t->getTexture());
		}
		else {
			Vertex v0(ptr->t->getVertex(0));
			Vertex v1(ptr->t->getVertex(1));
			Vertex v2(ptr->t->getVertex(2));

			Triangle* t = new Triangle(&v0, &v1, &v2);

			t->setTexture(ptr->t->getTexture());
			t->setCoords(0, ptr->t->getCoords(0));
			t->setCoords(1, ptr->t->getCoords(1));
			t->setCoords(2, ptr->t->getCoords(2));

			addTriangle(&clipped_head, &clipped_tail, t);
		}
		ptr = ptr->next;
	}

	destroyList(&temphead, &temptail);
	temphead = clipped_head;
	temptail = clipped_tail;

	clipped_head = NULL;
	clipped_tail = NULL;

	// v0 is the outside coord, v1 is after v0 and v2 is after v1
	auto clip1_negy = [this](Vector4 v0, Vector4 v1, Vector4 v2, Vector4 tex0, Vector4 tex1, Vector4 tex2, Texture* tex) {
		float t1 = (-v0.w() - v0.y()) / (v1.y() + v1.w() - v0.y() - v0.w());
		float t2 = (-v0.w() - v0.y()) / (v2.y() + v2.w() - v0.y() - v0.w());

		Vector4 v0v1 = lerp(v0, v1, t1);
		Vector4 v0v2 = lerp(v0, v2, t2);

		Vector4 tex01 = lerp(tex0, tex1, t1);
		Vector4 tex02 = lerp(tex0, tex2, t2);

		Vertex v01(v0v1.x(), v0v1.y(), v0v1.z(), v0v1.w());
		Vertex v02(v0v2.x(), v0v2.y(), v0v2.z(), v0v2.w());
		Vertex v1_vert(v1.x(), v1.y(), v1.z(), v1.w());
		Vertex v2_vert(v2.x(), v2.y(), v2.z(), v2.w());
		Triangle* tri1 = new Triangle(&v01, &v1_vert, &v02);
		Triangle* tri2 = new Triangle(&v02, &v1_vert, &v2_vert);

		tri1->setTexture(tex);
		tri2->setTexture(tex);

		tri1->setCoords(0, tex01);
		tri1->setCoords(1, tex1);
		tri1->setCoords(2, tex02);

		tri2->setCoords(0, tex02);
		tri2->setCoords(1, tex1);
		tri2->setCoords(2, tex2);

		addTriangle(&clipped_head, &clipped_tail, tri1);
		addTriangle(&clipped_head, &clipped_tail, tri2);

	};

	// v0 is the inside coord, v1 is after v0 and v2 is after v1
	auto clip2_negy = [this](Vector4 v0, Vector4 v1, Vector4 v2, Vector4 tex0, Vector4 tex1, Vector4 tex2, Texture* tex) {
		float t1 = (-v1.w() - v1.y()) / (v0.y() + v0.w() - v1.y() - v1.w());
		float t2 = (-v2.w() - v2.y()) / (v0.y() + v0.w() - v2.y() - v2.w());

		Vector4 v1v0 = lerp(v1, v0, t1);
		Vector4 v2v0 = lerp(v2, v0, t2);

		Vector4 tex10 = lerp(tex1, tex0, t1);
		Vector4 tex20 = lerp(tex2, tex0, t2);

		Vertex v10(v1v0.x(), v1v0.y(), v1v0.z(), v1v0.w());
		Vertex v20(v2v0.x(), v2v0.y(), v2v0.z(), v2v0.w());
		Vertex v0_vert(v0.x(), v0.y(), v0.z(), v0.w());

		Triangle* tri1 = new Triangle(&v0_vert, &v10, &v20);

		tri1->setTexture(tex);

		tri1->setCoords(0, tex0);
		tri1->setCoords(1, tex10);
		tri1->setCoords(2, tex20);

		addTriangle(&clipped_head, &clipped_tail, tri1);

	};

	// -y plane
	for (ptr = temphead; ptr;) {
		for (size_t i = 0; i < 3; i++)
		{
			Vertex v = ptr->t->getVertex(i);
			clipCoords[i] << v.x, v.y, v.z, v.w;
		}

		if (clipCoords[0].x() < -clipCoords[0].w() && clipCoords[1].x() < -clipCoords[1].w() && clipCoords[2].x() < -clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].y() < -clipCoords[0].w() && clipCoords[1].y() < -clipCoords[1].w() && clipCoords[2].y() < -clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].z() < -clipCoords[0].w() && clipCoords[1].z() < -clipCoords[1].w() && clipCoords[2].z() < -clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].x() > clipCoords[0].w() && clipCoords[1].x() > clipCoords[1].w() && clipCoords[2].x() > clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].y() > clipCoords[0].w() && clipCoords[1].y() > clipCoords[1].w() && clipCoords[2].y() > clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].z() > clipCoords[0].w() && clipCoords[1].z() > clipCoords[1].w() && clipCoords[2].z() > clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].y() < -clipCoords[0].w()) {
			if (clipCoords[1].y() < -clipCoords[1].w()) {
				clip2_negy(clipCoords[2], clipCoords[0], clipCoords[1], ptr->t->getCoords(2), ptr->t->getCoords(0), ptr->t->getCoords(1), ptr->t->getTexture());
			}
			else if (clipCoords[2].y() < -clipCoords[2].w()) {
				clip2_negy(clipCoords[1], clipCoords[2], clipCoords[0], ptr->t->getCoords(1), ptr->t->getCoords(2), ptr->t->getCoords(0), ptr->t->getTexture());
			}
			else {
				clip1_negy(clipCoords[0], clipCoords[1], clipCoords[2], ptr->t->getCoords(0), ptr->t->getCoords(1), ptr->t->getCoords(2), ptr->t->getTexture());
			}
		}
		else if (clipCoords[1].y() < -clipCoords[1].w()) {
			if (clipCoords[2].y() < -clipCoords[2].w()) {
				clip2_negy(clipCoords[0], clipCoords[1], clipCoords[2], ptr->t->getCoords(0), ptr->t->getCoords(1), ptr->t->getCoords(2), ptr->t->getTexture());
			}
			else {
				clip1_negy(clipCoords[1], clipCoords[2], clipCoords[0], ptr->t->getCoords(1), ptr->t->getCoords(2), ptr->t->getCoords(0), ptr->t->getTexture());
			}
		}
		else if (clipCoords[2].y() < -clipCoords[2].w()) {
			clip1_negy(clipCoords[2], clipCoords[0], clipCoords[1], ptr->t->getCoords(2), ptr->t->getCoords(0), ptr->t->getCoords(1), ptr->t->getTexture());
		}
		else {
			Vertex v0(ptr->t->getVertex(0));
			Vertex v1(ptr->t->getVertex(1));
			Vertex v2(ptr->t->getVertex(2));

			Triangle* t = new Triangle(&v0, &v1, &v2);

			t->setTexture(ptr->t->getTexture());
			t->setCoords(0, ptr->t->getCoords(0));
			t->setCoords(1, ptr->t->getCoords(1));
			t->setCoords(2, ptr->t->getCoords(2));

			addTriangle(&clipped_head, &clipped_tail, t);
		}
		ptr = ptr->next;
	}

	destroyList(&temphead, &temptail);
	temphead = clipped_head;
	temptail = clipped_tail;

	clipped_head = NULL;
	clipped_tail = NULL;

	// v0 is the outside coord, v1 is after v0 and v2 is after v1
	auto clip1_far = [this](Vector4 v0, Vector4 v1, Vector4 v2, Vector4 tex0, Vector4 tex1, Vector4 tex2, Texture* tex) {
		float t1 = (v0.w() - v0.z()) / (v1.z() + v0.w() - v0.z() - v1.w());
		float t2 = (v0.w() - v0.z()) / (v2.z() + v0.w() - v0.z() - v2.w());

		Vector4 v0v1 = lerp(v0, v1, t1);
		Vector4 v0v2 = lerp(v0, v2, t2);

		Vector4 tex01 = lerp(tex0, tex1, t1);
		Vector4 tex02 = lerp(tex0, tex2, t2);

		Vertex v01(v0v1.x(), v0v1.y(), v0v1.z(), v0v1.w());
		Vertex v02(v0v2.x(), v0v2.y(), v0v2.z(), v0v2.w());
		Vertex v1_vert(v1.x(), v1.y(), v1.z(), v1.w());
		Vertex v2_vert(v2.x(), v2.y(), v2.z(), v2.w());
		Triangle* tri1 = new Triangle(&v01, &v1_vert, &v02);
		Triangle* tri2 = new Triangle(&v02, &v1_vert, &v2_vert);

		tri1->setTexture(tex);
		tri2->setTexture(tex);

		tri1->setCoords(0, tex01);
		tri1->setCoords(1, tex1);
		tri1->setCoords(2, tex02);

		tri2->setCoords(0, tex02);
		tri2->setCoords(1, tex1);
		tri2->setCoords(2, tex2);

		addTriangle(&clipped_head, &clipped_tail, tri1);
		addTriangle(&clipped_head, &clipped_tail, tri2);

	};

	// v0 is the inside coord, v1 is after v0 and v2 is after v1
	auto clip2_far = [this](Vector4 v0, Vector4 v1, Vector4 v2, Vector4 tex0, Vector4 tex1, Vector4 tex2, Texture* tex) {
		float t1 = (v1.w() - v1.z()) / (v0.z() + v1.w() - v1.z() - v0.w());
		float t2 = (v2.w() - v2.z()) / (v0.z() + v1.w() - v2.z() - v0.w());

		Vector4 v1v0 = lerp(v1, v0, t1);
		Vector4 v2v0 = lerp(v2, v0, t2);

		Vector4 tex10 = lerp(tex1, tex0, t1);
		Vector4 tex20 = lerp(tex2, tex0, t2);

		Vertex v10(v1v0.x(), v1v0.y(), v1v0.z(), v1v0.w());
		Vertex v20(v2v0.x(), v2v0.y(), v2v0.z(), v2v0.w());
		Vertex v0_vert(v0.x(), v0.y(), v0.z(), v0.w());

		Triangle* tri1 = new Triangle(&v0_vert, &v10, &v20);

		tri1->setTexture(tex);

		tri1->setCoords(0, tex0);
		tri1->setCoords(1, tex10);
		tri1->setCoords(2, tex20);

		addTriangle(&clipped_head, &clipped_tail, tri1);

	};

	// far plane
	for (ptr = temphead; ptr;) {
		for (size_t i = 0; i < 3; i++)
		{
			Vertex v = ptr->t->getVertex(i);
			clipCoords[i] << v.x, v.y, v.z, v.w;
		}

		if (clipCoords[0].x() < -clipCoords[0].w() && clipCoords[1].x() < -clipCoords[1].w() && clipCoords[2].x() < -clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].y() < -clipCoords[0].w() && clipCoords[1].y() < -clipCoords[1].w() && clipCoords[2].y() < -clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].z() < -clipCoords[0].w() && clipCoords[1].z() < -clipCoords[1].w() && clipCoords[2].z() < -clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].x() > clipCoords[0].w() && clipCoords[1].x() > clipCoords[1].w() && clipCoords[2].x() > clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].y() > clipCoords[0].w() && clipCoords[1].y() > clipCoords[1].w() && clipCoords[2].y() > clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].z() > clipCoords[0].w() && clipCoords[1].z() > clipCoords[1].w() && clipCoords[2].z() > clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].z() > clipCoords[0].w()) {
			if (clipCoords[1].z() > clipCoords[1].w()) {
				clip2_far(clipCoords[2], clipCoords[0], clipCoords[1], ptr->t->getCoords(2), ptr->t->getCoords(0), ptr->t->getCoords(1), ptr->t->getTexture());
			}
			else if (clipCoords[2].z() > clipCoords[2].w()) {
				clip2_far(clipCoords[1], clipCoords[2], clipCoords[0], ptr->t->getCoords(1), ptr->t->getCoords(2), ptr->t->getCoords(0), ptr->t->getTexture());
			}
			else {
				clip1_far(clipCoords[0], clipCoords[1], clipCoords[2], ptr->t->getCoords(0), ptr->t->getCoords(1), ptr->t->getCoords(2), ptr->t->getTexture());
			}
		}
		else if (clipCoords[1].z() > clipCoords[1].w()) {
			if (clipCoords[2].z() > clipCoords[2].w()) {
				clip2_far(clipCoords[0], clipCoords[1], clipCoords[2], ptr->t->getCoords(0), ptr->t->getCoords(1), ptr->t->getCoords(2), ptr->t->getTexture());
			}
			else {
				clip1_far(clipCoords[1], clipCoords[2], clipCoords[0], ptr->t->getCoords(1), ptr->t->getCoords(2), ptr->t->getCoords(0), ptr->t->getTexture());
			}
		}
		else if (clipCoords[2].z() > clipCoords[2].w()) {
			clip1_far(clipCoords[2], clipCoords[0], clipCoords[1], ptr->t->getCoords(2), ptr->t->getCoords(0), ptr->t->getCoords(1), ptr->t->getTexture());
		}
		else {
			Vertex v0(ptr->t->getVertex(0));
			Vertex v1(ptr->t->getVertex(1));
			Vertex v2(ptr->t->getVertex(2));

			Triangle* t = new Triangle(&v0, &v1, &v2);

			t->setTexture(ptr->t->getTexture());
			t->setCoords(0, ptr->t->getCoords(0));
			t->setCoords(1, ptr->t->getCoords(1));
			t->setCoords(2, ptr->t->getCoords(2));

			addTriangle(&clipped_head, &clipped_tail, t);
		}
		ptr = ptr->next;
	}

	destroyList(&temphead, &temptail);
	temphead = clipped_head;
	temptail = clipped_tail;

	clipped_head = NULL;
	clipped_tail = NULL;

	// v0 is the outside coord, v1 is after v0 and v2 is after v1
	auto clip1_near = [this](Vector4 v0, Vector4 v1, Vector4 v2, Vector4 tex0, Vector4 tex1, Vector4 tex2, Texture* tex) {
		float t1 = (-v0.w() - v0.z()) / (v1.z() + v1.w() - v0.z() - v0.w());
		float t2 = (-v0.w() - v0.z()) / (v2.z() + v2.w() - v0.z() - v0.w());

		Vector4 v0v1 = lerp(v0, v1, t1);
		Vector4 v0v2 = lerp(v0, v2, t2);
		Vector4 tex01 = lerp(tex0, tex1, t1);
		Vector4 tex02 = lerp(tex0, tex2, t2);

		Vertex v01(v0v1.x(), v0v1.y(), v0v1.z(), v0v1.w());
		Vertex v02(v0v2.x(), v0v2.y(), v0v2.z(), v0v2.w());
		Vertex v1_vert(v1.x(), v1.y(), v1.z(), v1.w());
		Vertex v2_vert(v2.x(), v2.y(), v2.z(), v2.w());
		Triangle* tri1 = new Triangle(&v01, &v1_vert, &v02);
		Triangle* tri2 = new Triangle(&v02, &v1_vert, &v2_vert);

		tri1->setTexture(tex);
		tri2->setTexture(tex);

		tri1->setCoords(0, tex01);
		tri1->setCoords(1, tex1);
		tri1->setCoords(2, tex02);

		tri2->setCoords(0, tex02);
		tri2->setCoords(1, tex1);
		tri2->setCoords(2, tex2);

		addTriangle(&clipped_head, &clipped_tail, tri1);
		addTriangle(&clipped_head, &clipped_tail, tri2);

	};

	// v0 is the inside coord, v1 is after v0 and v2 is after v1
	auto clip2_near = [this](Vector4 v0, Vector4 v1, Vector4 v2, Vector4 tex0, Vector4 tex1, Vector4 tex2, Texture* tex) {
		float t1 = (-v1.w() - v1.z()) / (v0.z() + v0.w() - v1.z() - v1.w());
		float t2 = (-v2.w() - v2.z()) / (v0.z() + v0.w() - v2.z() - v2.w());

		Vector4 v1v0 = lerp(v1, v0, t1);
		Vector4 v2v0 = lerp(v2, v0, t2);

		Vector4 tex10 = lerp(tex1, tex0, t1);
		Vector4 tex20 = lerp(tex2, tex0, t2);

		Vertex v10(v1v0.x(), v1v0.y(), v1v0.z(), v1v0.w());
		Vertex v20(v2v0.x(), v2v0.y(), v2v0.z(), v2v0.w());
		Vertex v0_vert(v0.x(), v0.y(), v0.z(), v0.w());

		Triangle* tri1 = new Triangle(&v0_vert, &v10, &v20);

		tri1->setTexture(tex);

		tri1->setCoords(0, tex0);
		tri1->setCoords(1, tex10);
		tri1->setCoords(2, tex20);

		addTriangle(&clipped_head, &clipped_tail, tri1);

	};

	// Near Plane
	for (ptr = temphead; ptr;) {
		for (size_t i = 0; i < 3; i++)
		{
			Vertex v = ptr->t->getVertex(i);
			clipCoords[i] << v.x, v.y, v.z, v.w;
		}

		if (clipCoords[0].x() < -clipCoords[0].w() && clipCoords[1].x() < -clipCoords[1].w() && clipCoords[2].x() < -clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].y() < -clipCoords[0].w() && clipCoords[1].y() < -clipCoords[1].w() && clipCoords[2].y() < -clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].z() < -clipCoords[0].w() && clipCoords[1].z() < -clipCoords[1].w() && clipCoords[2].z() < -clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].x() > clipCoords[0].w() && clipCoords[1].x() > clipCoords[1].w() && clipCoords[2].x() > clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].y() > clipCoords[0].w() && clipCoords[1].y() > clipCoords[1].w() && clipCoords[2].y() > clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].z() > clipCoords[0].w() && clipCoords[1].z() > clipCoords[1].w() && clipCoords[2].z() > clipCoords[2].w()) {
			ptr = ptr->next;
			continue;
		}

		if (clipCoords[0].z() < -clipCoords[0].w()) {
			if (clipCoords[1].z() < -clipCoords[1].w()) {
				clip2_near(clipCoords[2], clipCoords[0], clipCoords[1], ptr->t->getCoords(2), ptr->t->getCoords(0), ptr->t->getCoords(1), ptr->t->getTexture());
			}
			else if (clipCoords[2].z() < -clipCoords[2].w()) {
				clip2_near(clipCoords[1], clipCoords[2], clipCoords[0], ptr->t->getCoords(1), ptr->t->getCoords(2), ptr->t->getCoords(0), ptr->t->getTexture());
			}
			else {
				clip1_near(clipCoords[0], clipCoords[1], clipCoords[2], ptr->t->getCoords(0), ptr->t->getCoords(1), ptr->t->getCoords(2), ptr->t->getTexture());
			}
		}
		else if (clipCoords[1].z() < -clipCoords[1].w()) {
			if (clipCoords[2].z() < -clipCoords[2].w()) {
				clip2_near(clipCoords[0], clipCoords[1], clipCoords[2], ptr->t->getCoords(0), ptr->t->getCoords(1), ptr->t->getCoords(2), ptr->t->getTexture());
			}
			else {
				clip1_near(clipCoords[1], clipCoords[2], clipCoords[0], ptr->t->getCoords(1), ptr->t->getCoords(2), ptr->t->getCoords(0), ptr->t->getTexture());
			}
		}
		else if (clipCoords[2].z() < -clipCoords[2].w()) {
			clip1_near(clipCoords[2], clipCoords[0], clipCoords[1], ptr->t->getCoords(2), ptr->t->getCoords(0), ptr->t->getCoords(1), ptr->t->getTexture());
		}
		else {
			Vertex v0(ptr->t->getVertex(0));
			Vertex v1(ptr->t->getVertex(1));
			Vertex v2(ptr->t->getVertex(2));

			Triangle* t = new Triangle(&v0, &v1, &v2);

			t->setTexture(ptr->t->getTexture());
			t->setCoords(0, ptr->t->getCoords(0));
			t->setCoords(1, ptr->t->getCoords(1));
			t->setCoords(2, ptr->t->getCoords(2));

			addTriangle(&clipped_head, &clipped_tail, t);
		}
		ptr = ptr->next;
	}

	destroyList(&temphead, &temptail);

	for (ptr = clipped_head; ptr;) {
		for (size_t i = 0; i < 3; i++)
		{

			Vertex v = ptr->t->getVertex(i);
			clipCoords[i] << v.x, v.y, v.z, v.w;
			
		}
		
		/*for (size_t i = 0; i < 3; i++)
		{
			Matrix4x4 identity;
			identity << 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1;
			clipCoords[i] = identity * projection * modelView * verts[i];
		}*/


		//rasterizeTriangle(*(ptr->t), clipCoords);	

		rasterizeEdgeWalk(*(ptr->t), clipCoords);

		ptr = ptr->next;
	}

	
	return;
}
