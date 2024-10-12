#ifndef MATRIX_H_INCLUDED
#define MATRIX_H_INCLUDED

#include <ostream>

template <class t> struct Vec4;
typedef Vec4<float> Vec4f;

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

	Vec4f operator*(const Vec4f& vec);

	Mat4 operator*(const Mat4& mat);
};

std::ostream& operator<<(std::ostream& s, Mat4& m);

#endif
