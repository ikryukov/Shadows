//
//  MathUtils.h
//  Shadows
//
//  Created by Ilya Kryukov on 15.10.12.
//  Copyright (c) 2012 Ilya Kryukov. All rights reserved.
//

#ifndef Shadows_MathUtils_h
#define Shadows_MathUtils_h

#include "Vector.hpp"
#include "Matrix.hpp"

mat4 VerticalFieldOfView(float degrees, float aspectRatio, float near, float far)
{
	float top = near * std::tan(degrees * Pi / 360.0f);
	float bottom = -top;
	float left = bottom * aspectRatio;
	float right = top * aspectRatio;
	return mat4::Frustum(left, right, bottom, top, near, far);
}

static mat4 LookAt(const vec3& eye, const vec3& target, const vec3& up)
{
	vec3 z = (eye - target).Normalized();
	vec3 x = up.Cross(z).Normalized();
	vec3 y = z.Cross(x).Normalized();
	mat4 m;
	m.x = vec4(x.x, x.y, x.z, 0);
	m.y = vec4(y.x, y.y, y.z, 0);
	m.z = vec4(z.x, z.y, z.z, 0);
	m.w = vec4(0, 0, 0, 1);
	vec4 eyePrime = m * vec4(-eye.x , -eye.y, -eye.z, 1.0);
	m = m.Transposed();
	m.w = eyePrime;
	return m;
}


#endif
