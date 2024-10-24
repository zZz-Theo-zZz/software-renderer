#include "matrix.h"
#include "geometry.h"
#include <ostream>
#include <iostream>

void Mat4::MoveTo(const Vec3f& pos)
{
	Set(3, 0, pos.x);
	Set(3, 1, pos.y);
	Set(3, 2, pos.z);
}

void Mat4::Scale(const Vec3f& scale)
{
	Set(0, 0, scale.x);
	Set(1, 1, scale.y);
	Set(2, 2, scale.z);
}

Vec4f Mat4::operator*(const Vec4f& vec) const
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

Mat4 Mat4::operator*(const Mat4& mat) const
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

float Mat3::Determinant() const
{
	return Get(0, 0) * Get(1, 1) * Get(2, 2) +
		   Get(1, 0) * Get(2, 1) * Get(0, 2) +
		   Get(2, 0) * Get(0, 1) * Get(1, 2) -
		   Get(2, 0) * Get(1, 1) * Get(0, 2) -
		   Get(1, 0) * Get(0, 1) * Get(2, 2) -
		   Get(0, 0) * Get(2, 1) * Get(1, 2);
}

float Mat3::Get(int x, int y) const
{
	return data[y * 3 + x];
}

void Mat3::Set(int x, int y, float value)
{
	data[y * 3 + x] = value;
}

void Mat3::Dump()
{
	for (int y = 0; y < 3; ++y)
	{
		std::cout << "| ";
		for (int x = 0; x < 3; ++x)
		{
			std::cout << Get(x, y) << " | ";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

Mat3 Cofactor(int column, int line, const Mat4& mat)
{
	Mat3 ret;
	int i = 0;

	for (int y = 0; y < 4; ++y)
	{
		if (y != line)
		{
			for (int x = 0; x < 4; ++x)
			{
				if (x != column)
				{
					ret.data[i] = mat.Get(x, y);
					++i;
				}
			}
		}
	}

	return ret;
}

float Mat4::Determinant() const
{
	float ret = 0.0f;
	float sign = 1.0;
	for (int i = 0; i < 4; ++i)
	{
		float value = sign * Get(0, i) * Cofactor(0, i, *this).Determinant();
		ret += value;
		sign *= -1.0f;
	}

	return ret;
}

float sign(int i)
{
	if (i % 2 == 0)
		return 1.0f;
	else
		return -1.0f;
}

Mat4 Mat4::Inverse() const
{
	float determinant = Determinant();
	
	Mat4 mat;

	for (int y = 0; y < 4; ++y)
	{
		for (int x = 0; x < 4; ++x)
		{
			mat.Set(x, y, sign(x + y) * Cofactor(x, y, *this).Determinant());
		}
	}

	mat = Transpose(mat);

	for (int y = 0; y < 4; ++y)
	{
		for (int x = 0; x < 4; ++x)
		{
			mat.Set(x, y, mat.Get(x, y) / determinant);
		}
	}
	
	return mat;
}

Mat4 Mat4::Transpose(const Mat4& mat)
{
	Mat4 res;

	for (int y = 0; y < 4; ++y)
	{
		for (int x = 0; x < 4; ++x)
		{
			res.Set(y, x, mat.Get(x, y));
		}
	}

	return res;
}

Mat4 Mat4::GetViewport(int x, int y, int w, int h, float depth)
{
	Mat4 m;
	m.Set(3, 0 , x + w / 2.0f);
	m.Set(3, 1, y + h / 2.0f);
	m.Set(3, 2, depth / 2.0f);

	m.Set(0, 0, w / 2.0f);
	m.Set(1, 1, h / 2.0f);
	m.Set(2, 2, depth / 2.0f);

	return m;
}

Mat4 Mat4::GetProjection(float targetDistance)
{
	float projectionData[16] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, -1.0f / targetDistance, 1.0f
	};

	return Mat4(projectionData);
}

Mat4 Mat4::LookAt(const Vec3f& position, const Vec3f& target, const Vec3f& up)
{
	Vec3f z = (position - target).normalize();
	Vec3f x = (up ^ z).normalize();
	Vec3f y = (z ^ x).normalize();

	Mat4 inv;
	Mat4 tr;

	for (int i = 0; i < 3; ++i)
	{
		inv.Set(i, 0, x.raw[i]);
		inv.Set(i, 1, y.raw[i]);
		inv.Set(i, 2, z.raw[i]);
		inv.Set(3, i, -target.raw[i]);
	}

	return inv;
}

std::ostream& operator<<(std::ostream& s, Mat4& m)
{
	for (int y = 0; y < 4; ++y)
	{
		s << "| ";
		for (int x = 0; x < 4; ++x)
		{
			s << m.Get(x, y) << " | ";
		}
		s << "\n";
	}

	return s;
}