#ifndef VECTOR4_H
#define VECTOR4_H

#include <algorithm>

class Vector4 {
private:
	float vec[4];
	struct VecProxy {
		Vector4& v;
		int n;
		VecProxy(Vector4& v, int n) : v(v), n(n) {}
		VecProxy operator , (float val) {
			v.vec[n%4] = val;
			return VecProxy(v, n + 1);
		}
	};
public:
	Vector4() {
		memset(vec, 0, sizeof(vec));
	}

	VecProxy operator << (float val);

	Vector4& operator =(const Vector4& v);

	Vector4& operator =(Vector4& v);

	Vector4& operator +(const Vector4& other);

	Vector4& operator -(const Vector4& other);

	Vector4& operator *(float scalar);

	Vector4& operator /(float scalar);

	float x(void);
	float y(void);
	float z(void);
	float w(void);

	float dot(Vector4& other);

	Vector4 cross3(Vector4& other);

	void print(void) {
		fprintf(stderr, "Vector:\n");
		
		fprintf(stderr, "[%f %f %f %f]\n", vec[0], vec[1], vec[2], vec[3]);

		return;
	};
};

inline Vector4 operator* (float scalar, Vector4& v) {
	return v * scalar;
}

#endif
