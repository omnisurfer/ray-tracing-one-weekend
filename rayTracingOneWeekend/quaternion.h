#pragma once

/*
- http://www.chrobotics.com/library/understanding-quaternions
- https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
- https://stackoverflow.com/questions/52413464/look-at-quaternion-using-up-vector
*/

/*
 Basic class to help with using quaternions to handle the camera rotations. 
 ***WARNING*** All the operations are not defined fo this type yet per the recomendations of
 https://stackoverflow.com/questions/4421706/what-are-the-basic-rules-and-idioms-for-operator-overloading

 Just the bare minimum is implemented to get a feel for what the operations will do to the lookAt vector and to understand what is going
 on without just glossing over it. At some point using a real library is probably the right path forward.
*/

class quaternion {

public:
	quaternion() {}
	quaternion(const float a, const float bi, const float cj, const float dk) {

		e[0] = a;
		e[1] = bi;
		e[2] = cj;
		e[3] = dk;
	}

	inline float a() const { return e[0]; }
	inline float b() const { return e[1]; }
	inline float c() const { return e[2]; }
	inline float d() const { return e[3]; }

	inline float w() const { return e[0]; }
	inline float x() const { return e[1]; }
	inline float y() const { return e[2]; }
	inline float z() const { return e[3]; }	

	inline const quaternion& operator+() const { return *this; }
	
	float e[4];
};

inline std::ostream& operator<<(std::ostream &os, const quaternion &q) {

	os << q.a() << " + " << q.b() << "i + " << q.c() << "j + " << q.d() << "k ";
	return os;
}

inline quaternion operator+(const quaternion &q1, const quaternion &q2) {

	quaternion result;
	
	result.e[0] = q1.a() + q2.a();
	result.e[1] = q1.b() + q2.b();
	result.e[2] = q1.c() + q2.c();
	result.e[3] = q1.d() + q2.d();

	return result;
}

inline quaternion operator-(const quaternion &q1, const quaternion &q2) {
	
	quaternion result;

	result.e[0] = q1.a() - q2.a();
	result.e[1] = q1.b() - q2.b();
	result.e[2] = q1.c() - q2.c();
	result.e[3] = q1.d() - q2.d();

	return result;
}

inline quaternion operator*(const quaternion &q1, const quaternion &q2) {
	quaternion result;

	//https://en.wikipedia.org/wiki/Quaternion
	//straight copy of the Hamilton product section in the article
	//w			= a1a2		  - b1b2		- c1c2		  - d1d2
	result.e[0] = q1.a() * q2.a() -q1.b() * q2.b() - q1.c() * q2.c() - q1.d() * q2.d();

	//i			= a1b2		  + b1a2		+ c1d2		  - d1c2
	result.e[1] = q1.a() * q2.b() + q1.b() * q2.a() + q1.c() * q2.d() - q1.d() * q2.c();

	//j			= a1c2		  - b1d2		+ c1a2		  + d1b2
	result.e[2] = q1.a() * q2.c() - q1.b() * q2.d() + q1.c() * q2.a() + q1.d() * q2.b();
	
	//k			= a1d2		  + b1c2		- c1b2		  + d1a2
	result.e[3] = q1.a() * q2.d() + q1.b() * q2.c() - q1.c() * q2.b() + q1.d() * q2.a();
	
	return result;
}