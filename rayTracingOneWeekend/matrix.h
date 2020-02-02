#pragma once

/*
My attempt to make a matrix class. Not likely the best implementation but I want to think this through to understand it 
better.
***WARNING*** All the operations are not defined fo this type yet per the recomendations of
 https://stackoverflow.com/questions/4421706/what-are-the-basic-rules-and-idioms-for-operator-overloading

 Just the bare minimum is implemented to get a feel for what the operations will do to the lookAt vector and to understand what is going
 on without just glossing over it. At some point using a real library is probably the right path forward.
*/
#include "vec3.h"
#include "quaternion.h"
#include "debug.h"

class mat3x3 {

public:
	mat3x3() {}
	mat3x3(const vec3 matrix[3]) {
		m[0] = matrix[0];
		m[1] = matrix[1];
		m[2] = matrix[2];		
	};

	//https://stackoverflow.com/questions/4421706/what-are-the-basic-rules-and-idioms-for-operator-overloading
	inline const mat3x3& operator+() const { return *this; }

	vec3 m[3];

};

//https://stackoverflow.com/questions/35943537/error-c2804-binary-operator-has-too-many-parameters-compiling-with-vc-120
inline mat3x3 operator+(const mat3x3 &m1, const mat3x3 &m2) {

	vec3 a(
		m1.m[0][0] + m2.m[0][0], 
		m1.m[0][1] + m2.m[0][1],
		m1.m[0][2] + m2.m[0][2]
	);

	vec3 b(
		m1.m[1][0] + m2.m[1][0],
		m1.m[1][1] + m2.m[1][1],
		m1.m[1][2] + m2.m[1][2]
	);

	vec3 c(
		m1.m[2][0] + m2.m[2][0],
		m1.m[2][1] + m2.m[2][1],
		m1.m[2][2] + m2.m[2][2]
	);

	vec3 res[3] = { a, b, c };

	return mat3x3(res);
}

inline mat3x3 operator-(const mat3x3 &m1, const mat3x3 &m2) {

	vec3 a(
		m1.m[0][0] - m2.m[0][0],
		m1.m[0][1] - m2.m[0][1],
		m1.m[0][2] - m2.m[0][2]
	);

	vec3 b(
		m1.m[1][0] - m2.m[1][0],
		m1.m[1][1] - m2.m[1][1],
		m1.m[1][2] - m2.m[1][2]
	);

	vec3 c(
		m1.m[2][0] - m2.m[2][0],
		m1.m[2][1] - m2.m[2][1],
		m1.m[2][2] - m2.m[2][2]
	);

	vec3 res[3] = { a, b, c };

	return mat3x3(res);
}

inline mat3x3 operator*(const mat3x3 &m1, const mat3x3 &m2) {

	/*
		//Wolframs matrix multiplication answer
		[3, 2, 1]	 [0, 0, 1]   [1, 2, 3]
		[5, 4, 3]  X [0, 1, 0] = [3, 4, 5]
		[9, 8, 7]	 [1, 0, 0]   [7, 8, 9]

		[0, 0, 1]   [3, 2, 1]	[9, 8, 7]
		[0, 1, 0] X [5, 4, 3] = [5, 4, 3]
		[1, 0, 0]   [9, 8, 7]	[3, 2, 1]

		(row, column)

		row0,0: (0,0) * (0,0)	+	(0,1) * (0,1)	+	(0,2) * (0,2)
		row0,1: (0,0) *	(1,0)   +	(0,1) * (1,1)	+	(0,2) * (1,2)
		row0,2: (0,0) *	(2,0)   +	(0,1) * (2,1)	+	(0,2) * (2,2)
	*/

	//std::cout << "0,0: " << m1.m[0][0] * m2.m[0][0] << " + " << m1.m[0][1] * m2.m[0][1] << " + " << m1.m[0][2] * m2.m[0][2] << "\n";
	//std::cout << "0,1: " << m1.m[0][0] * m2.m[1][0] << " + " << m1.m[0][1] * m2.m[1][1] << " + " << m1.m[0][2] * m2.m[1][2] << "\n";
	//std::cout << "0,2: " << m1.m[0][0] * m2.m[2][0] << " + " << m1.m[0][1] * m2.m[2][1] << " + " << m1.m[0][2] * m2.m[2][2] << "\n";

	vec3 a(
		/*row0,0*/ m1.m[0][0] * m2.m[0][0] + m1.m[0][1] * m2.m[1][0] + m1.m[0][2] * m2.m[2][0],
		/*row0,1*/ m1.m[0][0] * m2.m[0][1] + m1.m[0][1] * m2.m[1][1] + m1.m[0][2] * m2.m[2][1],
		/*row0,2*/ m1.m[0][0] * m2.m[0][2] + m1.m[0][1] * m2.m[1][2] + m1.m[0][2] * m2.m[2][2]
	);

	vec3 b(
		/*row1,0*/ m1.m[1][0] * m2.m[0][0] + m1.m[1][1] * m2.m[1][0] + m1.m[1][2] * m2.m[2][0],
		/*row1,1*/ m1.m[1][0] * m2.m[0][1] + m1.m[1][1] * m2.m[1][1] + m1.m[1][2] * m2.m[2][1],
		/*row1,2*/ m1.m[1][0] * m2.m[0][2] + m1.m[1][1] * m2.m[1][2] + m1.m[1][2] * m2.m[2][2]
	);

	vec3 c(
		/*row2,0*/ m1.m[2][0] * m2.m[0][0] + m1.m[2][1] * m2.m[1][0] + m1.m[2][2] * m2.m[2][0],
		/*row2,1*/ m1.m[2][0] * m2.m[0][1] + m1.m[2][1] * m2.m[1][1] + m1.m[2][2] * m2.m[2][1],
		/*row2,2*/ m1.m[2][0] * m2.m[0][2] + m1.m[2][1] * m2.m[1][2] + m1.m[2][2] * m2.m[2][2]
	);

	vec3 res[3] = { a, b, c };

	return mat3x3(res);
}

inline vec3 operator*(const mat3x3 &m1, vec3 &v1) {

	float x, y, z;

	x = 0.0f, y = 0.0f, z = 0.0f;

	x = m1.m[0][0] * v1[0] + m1.m[0][1] * v1[1] + m1.m[0][2] * v1[2];
	y = m1.m[1][0] * v1[0] + m1.m[1][1] * v1[1] + m1.m[1][2] * v1[2];
	z = m1.m[2][0] * v1[0] + m1.m[2][1] * v1[1] + m1.m[2][2] * v1[2];

	vec3 res = { x, y, z };

	return res;
}

inline vec3 operator*(vec3 &v1, const mat3x3 &m1) {

	float x, y, z;

	x = 0.0f, y = 0.0f, z = 0.0f;

	x = m1.m[0][0] * v1[0] + m1.m[0][1] * v1[1] + m1.m[0][2] * v1[2];
	y = m1.m[1][0] * v1[0] + m1.m[1][1] * v1[1] + m1.m[1][2] * v1[2];
	z = m1.m[2][0] * v1[0] + m1.m[2][1] * v1[1] + m1.m[2][2] * v1[2];

	vec3 res = { x, y, z };

	return res;
}

class qmat4x4{
	
public:
	qmat4x4() {}
	qmat4x4(const  quaternion qMatrix[4]) {
		qm[0] = qMatrix[0];
		qm[1] = qMatrix[1];
		qm[2] = qMatrix[2];
		qm[3] = qMatrix[3];
	}

	inline const qmat4x4& operator+() const { return *this; }

	quaternion qm[4];
};