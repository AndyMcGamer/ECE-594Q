#include "Vector4.h"

Vector4::VecProxy Vector4::operator<<(float val) {
	vec[0] = val;
	return VecProxy(*this, 1);
}

Vector4& Vector4::operator=(const Vector4& v) {
	for (size_t i = 0; i < 4; i++)
	{
		vec[i] = v.vec[i];
	}
	return *this;
}

Vector4& Vector4::operator =(Vector4& v) {
	for (size_t i = 0; i < 4; i++)
	{
		vec[i] = v.vec[i];
	}
	return *this;
}

Vector4& Vector4::operator+(const Vector4& other) {
	for (size_t i = 0; i < 4; i++)
	{
		vec[i] += other.vec[i];
	}
	return *this;
}

Vector4& Vector4::operator+(const float scalar) {
	for (size_t i = 0; i < 4; i++)
	{
		vec[i] += scalar;
	}
	return *this;
}

Vector4& Vector4::operator-(const Vector4& other) {
	for (size_t i = 0; i < 4; i++)
	{
		vec[i] -= other.vec[i];
	}
	return *this;
}

Vector4& Vector4::operator *(float scalar) {
	for (size_t i = 0; i < 4; i++)
	{
		vec[i] *= scalar;
	}
	return *this;
}

Vector4& Vector4::operator /(float scalar) {
	for (size_t i = 0; i < 4; i++)
	{
		vec[i] /= scalar;
	}
	return *this;
}

const float Vector4::x(void) {
	return vec[0];
}

const float Vector4::y(void) {
	return vec[1];
}

const float Vector4::z(void) {
	return vec[2];
}

const float Vector4::w(void) {
	return vec[3];
}

float Vector4::dot(Vector4& other) {
	
	return x() * other.x() + y() * other.y() + z() * other.z() + w() * other.w();
}

Vector4 Vector4::cross3(Vector4& other) {
	Vector4 result;
	result << (y() * other.z() - z() * other.y()), -(x() * other.z() - z() * other.x()), (x() * other.y() - y() * other.x()), 0;
	return result;
}