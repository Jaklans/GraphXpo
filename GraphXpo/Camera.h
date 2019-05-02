#pragma once
#include <DirectXMath.h>
#include <Windows.h>

#include "Transform.h"

class Camera
{
public:
	Camera();
	~Camera();

	DirectX::XMFLOAT4X4 GetViewMatrix();
	DirectX::XMFLOAT4X4 GetProjectionMatrix();

	DirectX::XMFLOAT4X4 GetInverseViewMatrix();
	DirectX::XMFLOAT4X4 GetInverseProjectionMatrix();

	///<Summary>
	///Calculate the camera's view matrix. Move the camera.
	///</Summary>
	void Update(float deltaTime);

	///<Summary>
	///Rotate the camera by the specified angles, wrapping and limiting as necessary.
	///</Summary>
	void RotateCamera(float xAngle, float yAngle);

	///<Summary>
	///Given an aspect ratio, recreate the projection matrix
	///</Summary>
	void UpdateProjectionMatrix(unsigned int width, unsigned int height);

	Transform transform;

private:

	

	float xRotLimit;

	DirectX::XMFLOAT4X4 viewMatrix;			//transforms by the camera's position
	DirectX::XMFLOAT4X4 projectionMatrix;	//projects 3D scene onto a 2D plane

	DirectX::XMFLOAT4X4 inverseViewMatrix;			
	DirectX::XMFLOAT4X4 inverseProjectionMatrix;	
};

