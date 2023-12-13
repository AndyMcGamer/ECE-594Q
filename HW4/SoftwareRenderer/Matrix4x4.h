#ifndef MATRIX4X4_H
#define MATRIX4X4_H

#include "Vector4.h"

class Matrix4x4 {
private:
	float matrix[4][4];
	struct MatrixProxy {
		Matrix4x4& m;
		int n;
		MatrixProxy(Matrix4x4& m, int n) : m(m), n(n) {}
		MatrixProxy operator , (float val) {
			m.matrix[n / 4][n % 4] = val;
			return MatrixProxy(m, n + 1);
		}
	};
public:
	Matrix4x4() {
		memset(matrix, 0, sizeof(matrix));
	}


	// identity << 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1;


	MatrixProxy operator <<(float val);

	Matrix4x4& operator=(const Matrix4x4& m);

	Matrix4x4& operator *(const Matrix4x4& other);

	Vector4& operator *(Vector4& other);

	void print(void) {
		fprintf(stderr, "Matrix:\n");
		for (size_t i = 0; i < 4; i++)
		{
			fprintf(stderr, "[%f %f %f %f]\n", matrix[i][0], matrix[i][1], matrix[i][2], matrix[i][3]);
		}
		return;
	};
};

#endif