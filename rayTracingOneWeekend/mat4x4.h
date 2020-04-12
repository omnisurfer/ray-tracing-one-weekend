#pragma once

/*
My attempt to make a matrix class. Not likely the best implementation but I want to think this through to understand it 
better.
***WARNING*** All the operations are not defined fo this type yet per the recomendations of
 https://stackoverflow.com/questions/4421706/what-are-the-basic-rules-and-idioms-for-operator-overloading

 Just the bare minimum is implemented to get a feel for what the operations will do to the lookAt vector and to understand what is going
 on without just glossing over it. At some point using a real library is probably the right path forward.
*/
#include "vec4.h"
//#include "quaternion.h"
#include "debug.h"

class mat4x4 {

public:
	mat4x4() {}
	mat4x4(const vec4 matrix[4]) {
		m[0] = matrix[0];
		m[1] = matrix[1];
		m[2] = matrix[2];		
		m[3] = matrix[3];
	};

	//https://stackoverflow.com/questions/4421706/what-are-the-basic-rules-and-idioms-for-operator-overloading
	inline const mat4x4& operator+() const { return *this; }

	vec4 m[4];

};

inline std::ostream& operator<<(std::ostream &os, const mat4x4 &matrix) {
	os << "\n" << matrix.m[0] << "\n" << matrix.m[1] << "\n" << matrix.m[2] << "\n" << matrix.m[3];
	return os;
}

//https://stackoverflow.com/questions/35943537/error-c2804-binary-operator-has-too-many-parameters-compiling-with-vc-120
inline mat4x4 operator+(const mat4x4 &m1, const mat4x4 &m2) {

	vec4 a(
		m1.m[0][0] + m2.m[0][0], 
		m1.m[0][1] + m2.m[0][1],
		m1.m[0][2] + m2.m[0][2],
		m1.m[0][3] + m2.m[0][3]
	);

	vec4 b(
		m1.m[1][0] + m2.m[1][0],
		m1.m[1][1] + m2.m[1][1],
		m1.m[1][2] + m2.m[1][2],
		m1.m[1][3] + m2.m[1][3]
	);

	vec4 c(
		m1.m[2][0] + m2.m[2][0],
		m1.m[2][1] + m2.m[2][1],
		m1.m[2][2] + m2.m[2][2],
		m1.m[2][3] + m2.m[2][3]
	);

	vec4 d(
		m1.m[3][0] + m2.m[3][0],
		m1.m[3][1] + m2.m[3][1],
		m1.m[3][2] + m2.m[3][2],
		m1.m[3][3] + m2.m[3][3]
	);

	vec4 res[4] = { a, b, c, d };

	return mat4x4(res);
}

inline mat4x4 operator-(const mat4x4 &m1, const mat4x4 &m2) {

	vec4 a(
		m1.m[0][0] - m2.m[0][0],
		m1.m[0][1] - m2.m[0][1],
		m1.m[0][2] - m2.m[0][2],
		m1.m[0][3] - m2.m[0][3]
	);

	vec4 b(
		m1.m[1][0] - m2.m[1][0],
		m1.m[1][1] - m2.m[1][1],
		m1.m[1][2] - m2.m[1][2],
		m1.m[1][3] - m2.m[1][3]
	);

	vec4 c(
		m1.m[2][0] - m2.m[2][0],
		m1.m[2][1] - m2.m[2][1],
		m1.m[2][2] - m2.m[2][2],
		m1.m[2][3] - m2.m[2][3]
	);

	vec4 d(
		m1.m[3][0] - m2.m[3][0],
		m1.m[3][1] - m2.m[3][1],
		m1.m[3][2] - m2.m[3][2],
		m1.m[3][3] - m2.m[3][3]
	);

	vec4 res[4] = { a, b, c, d };

	return mat4x4(res);
}

inline mat4x4 operator*(const mat4x4 &m1, const mat4x4 &m2) {

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

	vec4 a(
		/*row0,0*/ m1.m[0][0] * m2.m[0][0] + m1.m[0][1] * m2.m[1][0] + m1.m[0][2] * m2.m[2][0] + m1.m[0][3] * m2.m[3][0],
		/*row0,1*/ m1.m[0][0] * m2.m[0][1] + m1.m[0][1] * m2.m[1][1] + m1.m[0][2] * m2.m[2][1] + m1.m[0][3] * m2.m[3][1],
		/*row0,2*/ m1.m[0][0] * m2.m[0][2] + m1.m[0][1] * m2.m[1][2] + m1.m[0][2] * m2.m[2][2] + m1.m[0][3] * m2.m[3][2],
		/*row0,3*/ m1.m[0][0] * m2.m[0][3] + m1.m[0][1] * m2.m[1][3] + m1.m[0][2] * m2.m[2][3] + m1.m[0][3] * m2.m[3][3]
	);

	vec4 b(
		/*row1,0*/ m1.m[1][0] * m2.m[0][0] + m1.m[1][1] * m2.m[1][0] + m1.m[1][2] * m2.m[2][0] + m1.m[1][3] * m2.m[3][0],
		/*row1,1*/ m1.m[1][0] * m2.m[0][1] + m1.m[1][1] * m2.m[1][1] + m1.m[1][2] * m2.m[2][1] + m1.m[1][3] * m2.m[3][1],
		/*row1,2*/ m1.m[1][0] * m2.m[0][2] + m1.m[1][1] * m2.m[1][2] + m1.m[1][2] * m2.m[2][2] + m1.m[1][3] * m2.m[3][2],
		/*row1,3*/ m1.m[1][0] * m2.m[0][3] + m1.m[1][1] * m2.m[1][3] + m1.m[1][2] * m2.m[2][3] + m1.m[1][3] * m2.m[3][3]
	);

	vec4 c(
		/*row2,0*/ m1.m[2][0] * m2.m[0][0] + m1.m[2][1] * m2.m[1][0] + m1.m[2][2] * m2.m[2][0] + m1.m[2][3] * m2.m[3][0],
		/*row2,1*/ m1.m[2][0] * m2.m[0][1] + m1.m[2][1] * m2.m[1][1] + m1.m[2][2] * m2.m[2][1] + m1.m[2][3] * m2.m[3][1],
		/*row2,2*/ m1.m[2][0] * m2.m[0][2] + m1.m[2][1] * m2.m[1][2] + m1.m[2][2] * m2.m[2][2] + m1.m[2][3] * m2.m[3][2],
		/*row2,3*/ m1.m[2][0] * m2.m[0][3] + m1.m[2][1] * m2.m[1][3] + m1.m[2][2] * m2.m[2][3] + m1.m[2][3] * m2.m[3][3]
	);

	vec4 d(
		/*row3,0*/ m1.m[3][0] * m2.m[0][0] + m1.m[3][1] * m2.m[1][0] + m1.m[3][2] * m2.m[2][0] + m1.m[3][3] * m2.m[3][0],
		/*row3,1*/ m1.m[3][0] * m2.m[0][1] + m1.m[3][1] * m2.m[1][1] + m1.m[3][2] * m2.m[2][1] + m1.m[3][3] * m2.m[3][1],
		/*row3,2*/ m1.m[3][0] * m2.m[0][2] + m1.m[3][1] * m2.m[1][2] + m1.m[3][2] * m2.m[2][2] + m1.m[3][3] * m2.m[3][2],
		/*row3,3*/ m1.m[3][0] * m2.m[0][3] + m1.m[3][1] * m2.m[1][3] + m1.m[3][2] * m2.m[2][3] + m1.m[3][3] * m2.m[3][3]
	);
	
	vec4 res[4] = { a, b, c, d};

	return mat4x4(res);
}

inline vec4 operator*(const mat4x4 &m1, vec4 &v1) {

	float x, y, z, w;

	x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;

	x = m1.m[0][0] * v1[0] + m1.m[0][1] * v1[1] + m1.m[0][2] * v1[2] + m1.m[0][3] * v1[3];
	y = m1.m[1][0] * v1[0] + m1.m[1][1] * v1[1] + m1.m[1][2] * v1[2] + m1.m[1][3] * v1[3];
	z = m1.m[2][0] * v1[0] + m1.m[2][1] * v1[1] + m1.m[2][2] * v1[2] + m1.m[2][3] * v1[3];
	w = m1.m[3][0] * v1[0] + m1.m[3][1] * v1[1] + m1.m[3][2] * v1[2] + m1.m[3][3] * v1[3];

	vec4 res = { x, y, z, w };

	return res;
}

inline vec4 operator*(vec4 &v1, const mat4x4 &m1) {

	float x, y, z, w;

	x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;

	x = m1.m[0][0] * v1[0] + m1.m[0][1] * v1[1] + m1.m[0][2] * v1[2] + m1.m[0][3] * v1[3];
	y = m1.m[1][0] * v1[0] + m1.m[1][1] * v1[1] + m1.m[1][2] * v1[2] + m1.m[1][3] * v1[3];
	z = m1.m[2][0] * v1[0] + m1.m[2][1] * v1[1] + m1.m[2][2] * v1[2] + m1.m[2][3] * v1[3];
	w = m1.m[3][0] * v1[0] + m1.m[3][1] * v1[1] + m1.m[3][2] * v1[2] + m1.m[3][3] * v1[3];

	vec4 res = { x, y, z, w };

	return res;
}