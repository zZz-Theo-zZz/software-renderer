#ifndef MATRIX_H_INCLUDED
#define MATRIX_H_INCLUDED

#include <ostream>

template <class t> struct Vec4;
template <class t> struct Vec3;
typedef Vec4<float> Vec4f;
typedef Vec3<float> Vec3f;

class Mat4
{
public:
	float data[16];

	Mat4() 
	{
		Identity();
	};

	Mat4(float init[16])
	{
		for (int i = 0; i < 16; ++i)
			data[i] = init[i];
	}

	inline void Identity()
	{
		for (int i = 0; i < 16; ++i)
		{
			if (i % 5 == 0)
				data[i] = 1.0f;
			else
				data[i] = 0.0f;
		}
	}

	inline void Set(int x, int y, float value)
	{
		data[y * 4 + x] = value;
	}

	inline float Get(int x, int y) const
	{
		return data[y * 4 + x];
	}

	void MoveTo(const Vec3f& pos);

	void Scale(const Vec3f& scale);

	Vec4f operator*(const Vec4f& vec) const;

	Mat4 operator*(const Mat4& mat) const;

	static Mat4 GetViewport(int x, int y, int w, int h, float depth);

	static Mat4 GetProjection(float center);

	static Mat4 LookAt(const Vec3f& position, const Vec3f& target, const Vec3f& up);
};

std::ostream& operator<<(std::ostream& s, Mat4& m);

#endif
