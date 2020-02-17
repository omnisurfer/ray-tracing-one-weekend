#pragma once

#include "vec3.h"
#include "vec4.h"
#include "quaternion.h"
#include "mat4x4.h"
#include "defines.h"
#include "mathUtilities.h"

class Camera {

public:
	Camera(vec3 lookFrom, vec3 lookAt, vec3 upDirection, float vFoV, float aspect, float aperture, float focusDistance, float t0, float t1) :
		_lookFromPoint(lookFrom), _lookAt(lookAt), _upDirection(upDirection), _vFoV(vFoV), _aspect(aspect), _aperture(aperture), _focusDistance(focusDistance), _time0(t0), _time1(t1) {

		/*drowan_NOTE_20200202
		On page 182 of 3D Math Primer for Graphics and Game Development, 2nd Ed.
		Trying to replicate the following matrix for rotation and translate ops

		T = 1 0 0 0
			0 1 0 0
			0 0 1 0
			0 0 0 1

		T = r r r 0
			r r r 0
			r r r 0
			p p p 1

		Where r is the rotation and p is the point by which rotation occurs about
		*/

		//drowan_TODO_20200213: I think long term I should create an abstract pose class
		//that the camera can add to itself to be able to move using composition? Or inheritance?
		/*
		Need to follow NED convention for "world frame" of [air vehicles](https://en.wikipedia.org/wiki/Axes_conventions)
		Following [Right Hand coordinate system](https://www.oreilly.com/library/view/learn-arcore-/9781788830409/03e5338d-02f1-4461-a57a-ef46a976f96b.xhtml)

						  -z Zenith, Up, Middle Finger, up/down on monitor
						  |
						  |	  +x, North, Thumb, into monitor
						  |   /
						  |	 /
						  |	/
						  |/
		 -y --------------/------------------- +y, East, Index Finger, left/right on monitor
						 /|
						/ |
					   /  |
					  /	  |
					 /    |
					/     |
				  -x	  +z, Nadir, Down into Earth's Core.
		*/

		/* Need to follow RPY angles for "body frame" of air vehicles. Right Hand coordiante system make your thumb the airplane nose, index finger the wing(s) and middle
		point towards the ground (or mabye landing gear?).

						 -z (yaw), Zenith, Vehicle Up, up/down on monitor
						  |
						  |	  +x (roll), Vehicle Front, into monitor.
						  |   /
						  |	 /
						  |	/
						  |/
			-y -----------/------------------- +y (pitch), Vehicle Right, left/right on monitor
						 /|
						/ |
					   /  |
					  /	  |
					 /    |
					/     |
				  -x	  +z, Vehicle wheels?
		*/

		vec4 nedWorldBasisOrientationMatrix[4] = {
			{-1.0f,0.0f,0.0f,0.0f},		//x, roll
			{0.0f,1.0f,0.0f,0.0f},		//y, pitch
			{0.0f,0.0f,-1.0f,0.0f},		//z, yaw
			{0.0f,0.0f,0.0f,1.0f}		//displacement basis? not sure yet...			
		};

		vec4 basisPositionMatrix[4] = {
			{0.0f,0.0f,0.0f,0.0f},		//x, roll basis
			{0.0f,0.0f,0.0f,0.0f},		//y, pitch basis
			{0.0f,0.0f,0.0f,0.0f},		//z, yaw basis
			{-500.0f,0.0f,0.0f,0.0f}		//displacement basis? not sure yet... this line is an origin of 0,0,0	
		};

		quaternion qOrientation = nedWorldBasisOrientationMatrix[0];
		quaternion qPosition = basisPositionMatrix[0];

		orientationMatrix = nedWorldBasisOrientationMatrix;
		positionMatrix = basisPositionMatrix;

		std::cout << "basisOrientationMatrix: \n"
			<< nedWorldBasisOrientationMatrix[0] << "\n"
			<< nedWorldBasisOrientationMatrix[1] << "\n"
			<< nedWorldBasisOrientationMatrix[2] << "\n"
			<< nedWorldBasisOrientationMatrix[3] << "\n";

		std::cout << "basisPositionMatrix: \n"
			<< basisPositionMatrix[0] << "\n"
			<< basisPositionMatrix[1] << "\n"
			<< basisPositionMatrix[2] << "\n"
			<< basisPositionMatrix[3] << "\n";		

		/*
		std::cout
			<< "o[0] dot o[1] = " << dot(basisOrientationMatrix[0], basisOrientationMatrix[1]) << "\n"
			<< "o[0] times p[0] = " << orientationMatrix.m[0] << " * " << positionMatrix.m[0] << " = " << orientationMatrix.m[0] * positionMatrix.m[0] << "\n"
			<< "ori times pos = " << orientationMatrix << " *\n" << positionMatrix << " =\n" << orientationMatrix * positionMatrix << "\n";

		std::cout
			//<< "qOri dot qPos: " << dot(qOrientation, qPosition) << "\n"
			<< "qOri times qPos: " << qOrientation << " * " << qPosition << " = " << qOrientation * qPosition << "\n";

		/**/
		setCamera();		
	}

	ray getRay(float s, float t) {
		//DEBUG - COMMNETED OUT SO THAT I CAN RENDER WITHOUT DOF BLUR
#if CAMERA_DOF_EN == 1
		vec3 rd = _lensRadius * randomInUnitDisk() + randomInUnitSphere();
#else 
		vec3 rd = _lensRadius * vec3(1.0, 1.0, 1.0);
#endif
		vec3 offset = _u * rd.x() + _v * rd.y();

		float time = _time0 + unifRand(randomNumberGenerator) * (_time1 - _time0);

		return ray(_origin + offset, _lowerLeftCorner + s * _horizontal + t * _vertical - _origin - offset, time);
	}

	void setLookAt(vec3 lookAt) {
		_lookAt = lookAt;

		setCamera();
	}

	void setLookFromPoint(vec3 lookFromPoint) {
		_lookFromPoint = lookFromPoint;

		setCamera();
	}

	void setUpDirection(vec3 upDirection) {
		_upDirection = upDirection;

		setCamera();
	}

	void setVerticalFoV(float vFoV) {
		_vFoV = vFoV;

		setCamera();
	}

	void setAspect(float aspect) {
		_aspect = aspect;

		setCamera();
	}

	void setAperture(float aperture) {
		_aperture = aperture;

		setCamera();
	}

	void setFocusDistance(float focusDistance) {
		_focusDistance = focusDistance;

		setCamera();
	}

	void setTime(float t0, float t1) {
		_time0 = t0;
		_time1 = t1;

		setCamera();
	}

	void getTime(float &t0, float &t1) {
		t0 = _time0;
		t1 = _time1;
	}

	vec3 getLookAt() {
		return _lookAt;
	}

	vec3 getLookFromPoint() {
		return _lookFromPoint;
	}

	vec3 getUpDirection() {
		return _upDirection;
	}

	vec3 getU() {
		return _u;
	}

	vec3 getV() {
		return _v;
	}

	vec3 getW() {
		return _w;
	}

	float setVerticalFoV() {
		return _vFoV;
	}

	float setAspect() {
		return _aspect;
	}

	float setAperture() {
		return _aperture;
	}

	float setFocusDistance() {
		return _focusDistance;
	}

	void setOrientationMatrix(mat4x4 orientationMatrix) {
		orientationMatrix = orientationMatrix;
	}

	mat4x4 getOrientationMatrix() {
		return orientationMatrix;
	}

	void setPositionMatrix(mat4x4 positionMatrix) {
		positionMatrix = positionMatrix;
	}

	mat4x4 getPositionMatrix() {
		return positionMatrix;
	}

	void setPosition(vec3 position) {
		positionMatrix.m[3][0] = position.x();
		positionMatrix.m[3][1] = position.y();
		positionMatrix.m[3][2] = position.z();
	}

	vec3 getPosition() {
		return vec3(positionMatrix.m[0][4], positionMatrix.m[1][4], positionMatrix.m[2][4]);
	}

protected:

	void setCamera_ORIGINAL() {

		//lookAt vector contains camera orientation relative to world space but centered on it's origin
		//origin of the camera is set by lookFrom

		//need to take move camera origin back to 0,0
		//perform rotation
		//move camera to new or original displacement
		//modify lookAt to relect this?

		_lensRadius = _aperture / 2;

		float theta = _vFoV * M_PI / 180;
		float half_height = tan(theta / 2);
		float half_width = _aspect * half_height;

		//lookFromPoint is taken from the position matrix
		_origin = _lookFromPoint;

		_w = unit_vector(_lookFromPoint - _lookAt);
		_u = unit_vector(cross(_upDirection, _w));
		//drowan_DEBUG_20200104: swaping x and y may create a line perpendicular to the _w
		//_u = vec3(_w.y(), -1.0 * _w.x(), _w.z());
		_v = cross(_w, _u);

#if 1
		_lowerLeftCorner = _origin -
			half_width * _focusDistance * _u -
			half_height * _focusDistance * _v -
			_focusDistance * _w;

		_horizontal = 2 * half_width * _focusDistance * _u;
		_vertical = 2 * half_height * _focusDistance * _v;
#else
		_lowerLeftCorner = _origin -
			half_width * _focusDistance * _w -
			half_height * _focusDistance * _u -
			_focusDistance * _v;

		_horizontal = 2 * half_width * _focusDistance * _w;
		_vertical = 2 * half_height * _focusDistance * _u;
#endif
	}

	void setCamera() {

		_lensRadius = _aperture / 2;

		float theta = _vFoV * M_PI / 180;
		float half_height = tan(theta / 2);
		float half_width = _aspect * half_height;

		_origin = _lookFromPoint;
		//_origin = vec3(positionMatrix.m[3][0], positionMatrix.m[3][1], positionMatrix.m[3][2]);

		//_w = unit_vector(_lookFromPoint - _lookAt);
		//looking along the +x axis (vehicle front) into the monitor (traditionally "z")
		_w = vec3(orientationMatrix.m[0][0], orientationMatrix.m[0][1], orientationMatrix.m[0][2]);
		//moving "left and right" along the +y axis
		_u = vec3(orientationMatrix.m[1][0], orientationMatrix.m[1][1], orientationMatrix.m[1][2]);
		//moving "up and down" along the +z axis, up down the monitor (traditionally "y")
		_v = vec3(orientationMatrix.m[2][0], orientationMatrix.m[2][1], orientationMatrix.m[2][2]); 

		_lowerLeftCorner = _origin -
			half_width * _focusDistance * _u -
			half_height * _focusDistance * _v -
			_focusDistance * _w;

		_horizontal = 2 * half_width * _focusDistance * _u;
		_vertical = 2 * half_height * _focusDistance * _v;

	}

private:

	vec3 _lookFromPoint;
	vec3 _lookAt;
	vec3 _upDirection;
	//vFov is top to bottom in degrees
	float _vFoV;
	float _aspect;
	float _aperture;
	float _focusDistance;

	vec3 _origin;
	vec3 _lowerLeftCorner;
	vec3 _horizontal;
	vec3 _vertical;
	//orthonormal basis vectors that the camera is aligned to?
	vec3 _u, _v, _w;
	float _lensRadius;

	float _time0, _time1;

	mat4x4 orientationMatrix;
	mat4x4 positionMatrix;
};