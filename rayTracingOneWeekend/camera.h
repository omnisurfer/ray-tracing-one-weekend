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
		_lookFrom(lookFrom), _lookAt(lookAt), _upDirection(upDirection), _vFoV(vFoV), _aspect(aspect), _aperture(aperture), _focusDistance(focusDistance), _time0(t0), _time1(t1) {

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
		
		vec4 basisOrientationMatrix[4] = {
			{1.0f,2.0f,3.0f,4.0f},		//x, roll basis
			{4.0f,1.0f,2.0f,3.0f},		//y, pitch basis
			{3.0f,4.0f,-1.0f,2.0f},		//z, yaw basis
			{2.0f,3.0f,4.0f,1.0f}		//displacement basis? not sure yet...			
		};

		vec4 basisPositionMatrix[4] = {
			{1.0f,2.0f,3.0f,4.0f},		//x, roll basis
			{4.0f,1.0f,2.0f,3.0f},		//y, pitch basis
			{3.0f,5.0f,-1.0f,4.0f},		//z, yaw basis
			{3.0f,0.0f,7.0f,10.0f}		//displacement basis? not sure yet...			
		};
		
		quaternion qOrientation = basisOrientationMatrix[0];
		quaternion qPosition = basisPositionMatrix[0];

		orientationMatrix = basisOrientationMatrix;
		positionMatrix = basisPositionMatrix;

		std::cout << "orientationMatrix: \n" 
			<< basisOrientationMatrix[0] << "\n" 
			<< basisOrientationMatrix[1] << "\n"
			<< basisOrientationMatrix[2] << "\n"
			<< basisOrientationMatrix[3] << "\n";

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

	void setLookFrom(vec3 lookFrom) {
		_lookFrom = lookFrom;

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

	vec3 getLookFrom() {
		return _lookFrom;
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

	void setOrientation(float roll, float pitch, float yaw) {
		/*
		quaternion qRoll = quaternion::eulerToQuaternion(
			yaw * qOrientationMatrix.qm[0].x(),
			yaw * qOrientationMatrix.qm[0].y(),
			yaw * qOrientationMatrix.qm[0].z()
		);

		quaternion qPitch = quaternion::eulerToQuaternion(
			yaw * qOrientationMatrix.qm[2].x(),
			yaw * qOrientationMatrix.qm[2].y(),
			yaw * qOrientationMatrix.qm[2].z()
		);

		quaternion qYaw = quaternion::eulerToQuaternion(
			yaw * qOrientationMatrix.qm[3].x(),
			yaw * qOrientationMatrix.qm[3].y(),
			yaw * qOrientationMatrix.qm[3].z()
		);

		qOrientationMatrix.qm[0] = qRoll;
		qOrientationMatrix.qm[1] = qPitch;
		qOrientationMatrix.qm[2] = qPitch;
		/**/
	}

	void setPosition(float x, float y, float z) {

	}

protected:

	void setCamera() {

		_lensRadius = _aperture / 2;

		float theta = _vFoV * M_PI / 180;
		float half_height = tan(theta / 2);
		float half_width = _aspect * half_height;

		_origin = _lookFrom;

		_w = unit_vector(_lookFrom - _lookAt);
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

private:

	vec3 _lookFrom;
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