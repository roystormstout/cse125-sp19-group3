////////////////////////////////////////
// Camera.h
////////////////////////////////////////

#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>

////////////////////////////////////////////////////////////////////////////////

// The Camera class provides a simple means to controlling the 3D camera. It could
// be extended to support more interactive controls. Ultimately. the camera sets the
// GL projection and viewing matrices.

class Camera {
public:
	Camera();

	void Update();
	void Reset();

	// Access functions
	void SetAspect(float a)					{Aspect=a;}
	void SetDistance(float d)				{Distance=d;}
	void SetAzimuth(float a)				{Azimuth=a;}
	void SetIncline(float i)				{Incline=i;}

	float GetDistance()						{return Distance;}
	float GetAzimuth()						{return Azimuth;}
	float GetIncline()						{return Incline;}
	float GetDepth();
	void setDestination(glm::vec3 dest);

	const glm::mat4 &GetViewMtx() { return  glm::lookAt(cam_pos, cam_look_at, cam_up); }
	const glm::mat4 &GetViewProjectMtx()	{return ViewProjectMtx;}

	// Default camera parameters
	glm::vec3 cam_pos;		// e  | Position of camera
	glm::vec3 cam_look_at;	// d  | This is where the camera looks at
	glm::vec3 cam_up;			// up | What orientation "up" is

private:
	// Perspective controls
	float FOV;			// Field of View Angle (degrees)
	float Aspect;		// Aspect Ratio
	float NearClip;		// Near clipping plane distance
	float FarClip;		// Far clipping plane distance

	// Polar controls
	float Distance;		// Distance of the camera eye position to the origin (meters)
	float Azimuth;		// Rotation of the camera eye position around the Y axis (degrees)
	float Incline;		// Angle of the camera eye position over the XZ plane (degrees)

	float speed;
	glm::vec3 direction;
	glm::vec3 destination;
	// Computed data
	glm::mat4 ViewProjectMtx;
};

////////////////////////////////////////////////////////////////////////////////
