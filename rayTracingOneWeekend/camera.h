#pragma once

#include "vec3.h"
#include "mathUtilities.h"

class Camera {

public:
	Camera(vec3 lookFrom, vec3 lookAt, vec3 upDirection, float vFoV, float aspect, float aperture, float focusDistance, float t0, float t1) : 
		_lookFrom(lookFrom), _lookAt(lookAt), _upDirection(upDirection), _vFoV(vFoV), _aspect(aspect), _aperture(aperture), _focusDistance(focusDistance), time0(t0), time1(t1) {

		setCamera();
	}

	ray getRay(float s, float t) {
		//DEBUG - COMMNETED OUT SO THAT I CAN RENDER WITHOUT DOF BLUR
#if 0
		vec3 rd = _lensRadius * randomInUnitDisk() + randomInUnitSphere();
#else 
		vec3 rd = _lensRadius * vec3(1, 1, 0); // randomInUnitDisk();
#endif
		vec3 offset = _u * rd.x() + _v * rd.y();

		float time = time0 + unifRand(randomNumberGenerator) * (time1 - time0);

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
		time0 = t0;
		time1 = t1;

		setCamera();
	}

	void getTime(float &t0, float &t1) {
		t0 = time0;
		t1 = time1;
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

protected:

	void setCamera() {

		_lensRadius = _aperture / 2;

		float theta = _vFoV * M_PI / 180;
		float half_height = tan(theta / 2);
		float half_width = _aspect * half_height;

		_origin = _lookFrom;

		_w = unit_vector(_lookFrom - _lookAt);
		_u = unit_vector(cross(_upDirection, _w));
		_v = cross(_w, _u);

		_lowerLeftCorner = _origin -
			half_width * _focusDistance * _u -
			half_height * _focusDistance * _v -
			_focusDistance * _w;

		_horizontal = 2 * half_width * _focusDistance * _u;
		_vertical = 2 * half_height * _focusDistance * _v;
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

	float time0, time1;
};