#pragma once

#if defined (PLATFORM_WIN) && PLATFORM_WIN == 1
#define _USE_MATH_DEFINES
#endif

//little worried if these may conflict with each other?
#include <cmath>
#include <math.h>
/*
- http://www.chrobotics.com/library/understanding-quaternions
- https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
- https://stackoverflow.com/questions/52413464/look-at-quaternion-using-up-vector
- https://math.stackexchange.com/questions/40164/how-do-you-rotate-a-vector-by-a-unit-quaternion
- https://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation
- https://www.vcalc.com/equation/?uuid=6c9fa386-75f1-11e6-9770-bc764e2038f2

Axes Conventions (following space navigation, body frame)
- https://en.wikipedia.org/wiki/Axes_conventions
*/

/*
 Basic class to help with using quaternions to handle the camera rotations. 
 ***WARNING*** All the operations are not defined for this type yet per the recomendations of
 https://stackoverflow.com/questions/4421706/what-are-the-basic-rules-and-idioms-for-operator-overloading

 Just the bare minimum is implemented to get a feel for what the operations will do to the lookAt vector and to understand what is going
 on without just glossing over it. At some point using a real library is probably the right path forward. Emphasis on clarity of the math 
 probably impacts performance.
*/

class quaternion {

	/*
	w x y z
	*/

public:
	quaternion() {}
	quaternion(const float a, const float bi, const float cj, const float dk) {
		components.w = a;		
		components.x = bi;
		components.y = cj;
		components.z = dk;
	}

	inline float a() const { return components.a; }
	inline float b() const { return components.b; }
	inline float c() const { return components.c; }
	inline float d() const { return components.d; }

	inline float w() const { return components.w; }
	inline float x() const { return components.x; }
	inline float y() const { return components.y; }
	inline float z() const { return components.z; }	

	inline void operator=(const quaternion &q2) {
		components.w = q2.w();
		components.x = q2.x();
		components.y = q2.y();
		components.z = q2.z();
	}
		
	//using a union to make things look cleaner...
	//https://stackoverflow.com/questions/2310483/purpose-of-unions-in-c-and-c
	struct componentValues {
		union {
			//drowan_MOD_20200202: changed order to match 3D Math Primer book order. Don't think operations will change
			//need to remember that e[4] is now w/a
			//drowan_MOD_20200208: changed back since this would be a non-standard way to represent quaternions. Just going to make a mat4x4 instead of repurposing
			//the quaternion.
			struct { float w, x, y, z; };
			struct { float a, b, c, d; };
			float e[4];
		};
	};

	componentValues components;

	static inline quaternion eulerToQuaternion(const float yawAngle_Z, const float pitchAngle_Y, const float rollAngle_X) {
		float cy = cos(yawAngle_Z * 0.5);
		float sy = sin(yawAngle_Z * 0.5);
		float cp = cos(pitchAngle_Y * 0.5);
		float sp = sin(pitchAngle_Y * 0.5);
		float cr = cos(rollAngle_X * 0.5);
		float sr = sin(rollAngle_X * 0.5);

		quaternion result;
		result.components.w = cy * cp * cr + sy * sp * sr;
		result.components.x = cy * cp * sr - sy * sp * cr;
		result.components.y = sy * cp * sr + cy * sp * cr;
		result.components.z = sy * cp * cr - cy * sp * sr;

		return result;
	}

	static inline void quaternionToEuler(const quaternion &q, float &yawAngle_Z, float &pitchAngle_Y, float &rollAngle_X) {		
		//roll (x-axis rotation)
		float sinrCosp = 2 * (q.w() * q.x() + q.y() * q.z());
		float cosrCosp = 1 - 2 * (q.x() * q.x() + q.y() * q.y());

		rollAngle_X = std::atan2(sinrCosp, cosrCosp);

		//pitch (y-axis rotation)
		float sinp = 2 * (q.w() * q.y() - q.z() * q.x());
		if (std::abs(sinp) >= 1) {
			pitchAngle_Y = std::copysign(M_PI / 2, sinp);
		}
		else {
			pitchAngle_Y = std::asin(sinp);
		}

		//yaw (z-axis rotation)
		float sinyCosp = 2 * (q.w() * q.z() + q.x() * q.y());
		float cosyCosp = 1 - 2 * (q.y() * q.y() + q.z() * q.z());

		yawAngle_Z = std::atan2(sinyCosp, cosyCosp);
	}

	inline quaternion conjugate() {
		quaternion result;

		result.components.w = components.w;
		result.components.x = -components.x;
		result.components.y = -components.y;
		result.components.z = -components.z;

		return result;
	}

	inline float norm() {
		float result;		

		result = components.w * components.w + components.x * components.x + components.y * components.y + components.z * components.z;
		result = sqrt(result);

		return result;
	}

	inline quaternion normalizeOrVersor() {
		quaternion result;		

		float norm = components.w * components.w + components.x * components.x + components.y * components.y + components.z * components.z;
		norm = sqrt(norm);

		result.components.w = components.w / norm;
		result.components.x = components.x / norm;
		result.components.y = components.y / norm;
		result.components.z = components.z / norm;

		return result;
	}

	inline quaternion inverse() {
		quaternion result;
		quaternion conjugate;

		conjugate.components.w = components.w;
		conjugate.components.x = -components.x;
		conjugate.components.y = -components.y;
		conjugate.components.z = -components.z;

		float normMagnitude = components.w * components.w + components.x * components.x + components.y * components.y + components.z * components.z;		 

		result.components.w = conjugate.components.w / normMagnitude;

		result.components.x = conjugate.components.x / normMagnitude;
		result.components.y = conjugate.components.y / normMagnitude;
		result.components.z = conjugate.components.z / normMagnitude;

		return result;
	}

private:
	//not sure yet how to divide a quaternion
	inline quaternion operator/(const quaternion &q);
};

inline std::ostream& operator<<(std::ostream &os, const quaternion &q) {

	os << q.w() << "w + " << q.x() << "i + " << q.y() << "j + " << q.z() << "k";
	return os;
}

inline quaternion operator+(const quaternion &q1, const quaternion &q2) {

	quaternion result;
	
	result.components.w = q1.w() + q2.w();
	result.components.x = q1.x() + q2.x();
	result.components.y = q1.y() + q2.y();
	result.components.z = q1.z() + q2.z();

	return result;
}

inline quaternion operator-(const quaternion &q1, const quaternion &q2) {
	
	quaternion result;

	result.components.w = q1.w() - q2.w();
	result.components.x = q1.x() - q2.x();
	result.components.y = q1.y() - q2.y();
	result.components.z = q1.z() - q2.z();

	return result;
}

inline quaternion operator*(const quaternion &q1, const quaternion &q2) {
	quaternion result;

	//https://en.wikipedia.org/wiki/Quaternion
	//straight copy of the Hamilton product section in the article
	//w			= a1a2		  - b1b2		- c1c2		  - d1d2
	result.components.w = q1.a() * q2.a() -q1.b() * q2.b() - q1.c() * q2.c() - q1.d() * q2.d();

	//i			= a1b2		  + b1a2		+ c1d2		  - d1c2
	result.components.x = q1.a() * q2.b() + q1.b() * q2.a() + q1.c() * q2.d() - q1.d() * q2.c();

	//j			= a1c2		  - b1d2		+ c1a2		  + d1b2
	result.components.y = q1.a() * q2.c() - q1.b() * q2.d() + q1.c() * q2.a() + q1.d() * q2.b();
	
	//k			= a1d2		  + b1c2		- c1b2		  + d1a2
	result.components.z = q1.a() * q2.d() + q1.b() * q2.c() - q1.c() * q2.b() + q1.d() * q2.a();
	
	return result;
}

inline quaternion operator*(float t, const quaternion &q1) {
	return quaternion(t * q1.w(), t * q1.x(), t * q1.y(), t * q1.z());
}

inline quaternion operator*(const quaternion &q1, float t) {
	return quaternion(t * q1.w(), t * q1.x(), t * q1.y(), t * q1.z());
}

inline quaternion operator/(float t, const quaternion &q1) {
	return quaternion(q1.w() / t, q1.x() / t, q1.y() / t, q1.z() / t);
}

inline quaternion operator/(const quaternion &q1, float t) {
	return quaternion(q1.w() / t, q1.x() / t, q1.y() / t, q1.z() / t);
}