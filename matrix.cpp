#include "matrix.h"
#include "geometry.h"
#include <ostream>

Vec4f Mat4::operator*(const Vec4f& vec)
{
	Vec4f result;
	for (int y = 0; y < 4; ++y)
	{
		for (int x = 0; x < 4; ++x)
		{
			result.raw[y] += Get(x, y) * vec.raw[x];
		}
	}

	return result;
}

Mat4 Mat4::operator*(const Mat4& mat)
{
	Mat4 result;
	for (int x = 0; x < 4; ++x)
	{
		for (int y = 0; y < 4; ++y)
		{
			float value = 0.0f;

			for (int i = 0; i < 4; ++i)
				value += Get(i, y) * mat.Get(x, i);

			result.Set(x, y, value);
		}
	}

	return result;
}

std::ostream& operator<<(std::ostream& s, Mat4& m)
{
	for (int x = 0; x < 4; ++x)
	{
		s << "| ";
		for (int y = 0; y < 4; ++y)
		{
			s << m.Get(x, y) << " | ";
		}
		s << "\n";
	}

	return s;
}