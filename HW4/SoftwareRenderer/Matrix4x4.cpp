#include "Matrix4x4.h"

Matrix4x4::MatrixProxy Matrix4x4::operator<<(float val) {
	matrix[0][0] = val;
	return MatrixProxy(*this, 1);
}

Matrix4x4& Matrix4x4::operator=(const Matrix4x4& m) {
	for (size_t i = 0; i < 4; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			matrix[i][j] = m.matrix[i][j];
		}
	}
	return *this;
}

Matrix4x4& Matrix4x4::operator *(const Matrix4x4& other) {
	Vector4 r1, r2, r3, r4, c1, c2, c3, c4;
	r1 << matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3];
	r2 << matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3];
	r3 << matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3];
	r4 << matrix[3][0], matrix[3][1], matrix[3][2], matrix[3][3];

	c1 << other.matrix[0][0], other.matrix[1][0], other.matrix[2][0], other.matrix[3][0];
	c2 << other.matrix[0][1], other.matrix[1][1], other.matrix[2][1], other.matrix[3][1];
	c3 << other.matrix[0][2], other.matrix[1][2], other.matrix[2][2], other.matrix[3][2];
	c4 << other.matrix[0][3], other.matrix[1][3], other.matrix[2][3], other.matrix[3][3];

	Matrix4x4 result;
	result << r1.dot(c1), r1.dot(c2), r1.dot(c3), r1.dot(c4), r2.dot(c1), r2.dot(c2), r2.dot(c3), r2.dot(c4),
		r3.dot(c1), r3.dot(c2), r3.dot(c3), r3.dot(c4), r4.dot(c1), r4.dot(c2), r4.dot(c3), r4.dot(c4);
	
	*this = result;
	return *this;
}

Vector4& Matrix4x4::operator *(Vector4& other) {
	Vector4 r1, r2, r3, r4;
	r1 << matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3];
	r2 << matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3];
	r3 << matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3];
	r4 << matrix[3][0], matrix[3][1], matrix[3][2], matrix[3][3];

	Vector4 result;

	result << r1.dot(other), r2.dot(other), r3.dot(other), r4.dot(other);

	other = result;
	return other;
}