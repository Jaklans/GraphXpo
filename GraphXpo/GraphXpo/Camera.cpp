#include "Camera.h"

using namespace DirectX;


Camera::Camera()
{
	//init the transform data
	transform = Transform();
	transform.SetPosition(0, 0, -5);//default position is just behind the origin

	xRotLimit = XMConvertToRadians(75.0f);

	Update(0); //set up the view matrix
}


Camera::~Camera()
{
}

XMFLOAT4X4 Camera::GetViewMatrix()
{
	return viewMatrix;
}
XMFLOAT4X4 Camera::GetProjectionMatrix()
{
	return projectionMatrix;
}

///<Summary>
///Calculate the camera's view matrix. Move the camera.
///</Summary>
void Camera::Update(float deltaTime)
{

	//calculate the camera's view matrix

	XMVECTOR forward = XMLoadFloat3(&transform.GetForwardVector());

	//use the new vector to create the view matrix
	XMMATRIX view = XMMatrixLookToLH(XMLoadFloat3(&transform.GetPosition()), forward, XMVectorSet(0, 1, 0, 0));

	//store the resultant view matrix
	XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(view));
}

///<Summary>
///Rotate the camera by the specified angles, wrapping and limiting as necessary.
///</Summary>
void Camera::RotateCamera(float xAngle, float yAngle)
{

	float xRotation = transform.GetRotation().x + xAngle;
	float yRotation = transform.GetRotation().y + yAngle;

	//wrap (perhaps unnecessary because we are using radians) and limit

	//rotations around the x-axis need to be limited so the camera doesn't flip
	//this implementation limits them to looking straight up and straight down,w but never beyond
	if (xRotation > xRotLimit)
		xRotation = xRotLimit;
	else if(xRotation < -xRotLimit)
		xRotation = -xRotLimit;

	//rotations about the y-axis just need to be wrapped. They don't flip the camera
	if (yRotation > XM_2PI)
		yRotation -= XM_2PI;
	else if (yRotation < 0)
		yRotation += XM_2PI;

	//now apply the rotations
	transform.SetRotation(xRotation, yRotation, 0);
}

///<Summary>
///Given an aspect ratio, recreate the projection matrix
///</Summary>
void Camera::UpdateProjectionMatrix(unsigned int width, unsigned int height)
{
	// Create the Projection matrix
	// - This should match the window's aspect ratio, and also update anytime
	//    the window resizes (which is already happening in OnResize() below)
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,		// Field of View Angle
		(float)width / (float)height,		// Aspect ratio
		0.1f,						// Near clip plane distance
		100.0f);					// Far clip plane distance
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P)); // Transpose for HLSL!
}
