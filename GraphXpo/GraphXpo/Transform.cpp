#include "Transform.h"

using namespace DirectX;

Transform::Transform()
{
	//set default values for the transform data
	position = XMFLOAT3(0, 0, 0); //default position is the origin
	rotation = XMFLOAT3(0, 0, 0); //default rotation is 0 about each axis
	scale = XMFLOAT3(1, 1, 1); //default scale is 1, uniform

	CalculateLocalDirections();

	//now construct the default world matrix
	CalculateWorldMatrix();
}


Transform::~Transform()
{
}

//Relative transformations

///<summary>
///Adjust the current position by vector quantity.
///</summary>
void Transform::Translate(XMFLOAT3 translation)
{
	//first, store the current postiion and parameter as XMVECTORs so we can use SIMD math operations

	XMVECTOR transVec = XMLoadFloat3(&translation);
	XMVECTOR posVec = XMLoadFloat3(&position);

	XMStoreFloat3(&position, XMVectorAdd(transVec, posVec)); //add the translation to the current position and then store it
	
	matrixOutdated = true;
}
///<summary>
///Adjust the current position by the specified float quantities.
///</summary>
void Transform::Translate(float x, float y, float z)
{
	//package the float quanitites as a vector and pass it in to the standard translation function
	XMFLOAT3 translation = XMFLOAT3(x, y, z);
	Translate(translation);
}


///<summary>
///Adjust the current position relative to the entity's local axes.
///</summary>
void Transform::TranslateLocal(XMFLOAT3 translation)
{
	// Rotate desired movement by our rotation
	XMVECTOR dir = XMVector3Rotate(XMLoadFloat3(&translation), XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&rotation)));

	// Add to position and store
	XMStoreFloat3(&position, XMVectorAdd(XMLoadFloat3(&position), dir));

	matrixOutdated = true;
}
///<summary>
///Adjust the current position relative to the entity's local axes.
///</summary>
void Transform::TranslateLocal(float x, float y, float z)
{
	// Rotate desired movement by our rotation
	XMVECTOR dir = XMVector3Rotate(XMVectorSet(x, y, z, 0), XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&rotation)));

	// Add to position and store
	XMStoreFloat3(&position, XMVectorAdd(XMLoadFloat3(&position), dir));

	matrixOutdated = true;
}


///<summary>
///Adjust the current position along the entity's forward vector.
///</summary>
void Transform::TranslateForward(float magnitude)
{
	// Rotate desired movement by our rotation
	XMVECTOR dir = XMVector3Rotate(XMVectorSet(0, 0, magnitude, 0), XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&rotation)));

	// Add to position and store
	XMStoreFloat3(&position, XMVectorAdd(XMLoadFloat3(&position), dir));

	matrixOutdated = true;
}


///<summary>
///Adjust the current rotation by some specified quantity.
///</summary>
void Transform::Rotate(XMFLOAT3 eulers)
{
	XMStoreFloat3(&rotation, XMVectorAdd(XMLoadFloat3(&eulers), XMLoadFloat3(&rotation)));
	CalculateLocalDirections();

	matrixOutdated = true;
}
///<summary>
///Adjust the current rotation by some specified quantity.
///</summary>
void Transform::Rotate(float xAngle, float yAngle, float zAngle)
{
	XMStoreFloat3(&rotation, XMVectorAdd(XMLoadFloat3(&rotation), XMVectorSet(xAngle, yAngle, zAngle, 0)));
	CalculateLocalDirections();

	matrixOutdated = true;
}
///<summary>
///Adjust the current rotation by some specified angle around a given axis.
///</summary>
void Transform::Rotate(XMFLOAT3 axis, float angle)
{
	//get the normalized direction of the axis
	XMVECTOR rot = XMVector3Normalize(XMLoadFloat3(&axis));

	//apply the magnitude of the rotation to the rotation direction
	rot = XMVectorScale(rot, angle);

	// Rotate adjust current rotation by the new rotation
	XMStoreFloat3(&rotation, XMVectorAdd(rot, XMLoadFloat3(&rotation)));

	CalculateLocalDirections();

	matrixOutdated = true;
}


//Accessors and Mutators

//world matrix
XMFLOAT4X4 Transform::GetWorldMatrix()
{
	return worldMatrix;
}
XMFLOAT4X4 Transform::GetInverseTranspose()
{
	return inverseTransposeWorld;
}

//position
XMFLOAT3 Transform::GetPosition()
{
	return position;
}
void Transform::SetPosition(XMFLOAT3 pos)
{
	XMStoreFloat3(&position, XMLoadFloat3(&pos));

	matrixOutdated = true;
}
void Transform::SetPosition(float x, float y, float z)
{
	XMStoreFloat3(&position, XMVectorSet(x, y, z, 0));

	matrixOutdated = true;
}

//rotation
XMFLOAT3 Transform::GetRotation()
{
	return rotation;
}
void Transform::SetRotation(XMFLOAT3 rot)
{
	XMStoreFloat3(&rotation, XMLoadFloat3(&rot));
	CalculateLocalDirections();

	matrixOutdated = true;
}
void Transform::SetRotation(float xAngle, float yAngle, float zAngle)
{
	XMStoreFloat3(&rotation, XMVectorSet(xAngle, yAngle, zAngle, 0));
	CalculateLocalDirections();

	matrixOutdated = true;
}

//scale
XMFLOAT3 Transform::GetScale()
{
	return scale;
}
void Transform::SetScale(XMFLOAT3 scl)
{
	XMStoreFloat3(&scale, XMLoadFloat3(&scl));

	matrixOutdated = true;
}
void Transform::SetScale(float x, float y, float z)
{
	XMStoreFloat3(&scale, XMVectorSet(x, y, z, 0));

	matrixOutdated = true;
}

//local direction vectors
XMFLOAT3 Transform::GetForwardVector()
{
	return forward;
}
XMFLOAT3 Transform::GetRightVector()
{
	return right;
}
XMFLOAT3 Transform::GetUpVector()
{
	return up;
}

//Individual component setting functions

//position
void Transform::SetPositionX(float x)
{
	XMStoreFloat3(&position, XMVectorSet(x, position.y, position.z, 0));

	matrixOutdated = true;
}
void Transform::SetPositionY(float y)
{
	XMStoreFloat3(&position, XMVectorSet(position.x, y, position.z, 0));

	matrixOutdated = true;
}
void Transform::SetPositionZ(float z)
{
	XMStoreFloat3(&position, XMVectorSet(position.x, position.y, z, 0));

	matrixOutdated = true;
}


//rotation
void Transform::SetRotationX(float xAngle)
{
	XMStoreFloat3(&rotation, XMVectorSet(xAngle, rotation.y, rotation.z, 0));
	CalculateLocalDirections();

	matrixOutdated = true;
}
void Transform::SetRotationY(float yAngle)
{
	XMStoreFloat3(&rotation, XMVectorSet(rotation.x, yAngle, rotation.z, 0));
	CalculateLocalDirections();

	matrixOutdated = true;
}
void Transform::SetRotationZ(float zAngle)
{
	XMStoreFloat3(&rotation, XMVectorSet(rotation.x, rotation.y, zAngle, 0));
	CalculateLocalDirections();

	matrixOutdated = true;
}

//scale
void Transform::SetScaleX(float x)
{
	XMStoreFloat3(&scale, XMVectorSet(x, scale.y, scale.z, 0));

	matrixOutdated = true;
}
void Transform::SetScaleY(float y)
{
	XMStoreFloat3(&scale, XMVectorSet(scale.x, y, scale.z, 0));

	matrixOutdated = true;
}
void Transform::SetScaleZ(float z)
{
	XMStoreFloat3(&scale, XMVectorSet(scale.x, scale.y, z, 0));

	matrixOutdated = true;
}


///<summary>
///Combines position, rotation, and scale data into a single matrix.
///This combined transformation, when applied, will put the GameEntity into world coordinates
///</summary>
void Transform::CalculateWorldMatrix()
{
	//convert all 3 transform data vectors XMFLoat3s into XMVECTORs
	//(this is done on the stack as the conversion is intended to be temporary)
	XMVECTOR pos = XMLoadFloat3(&position);
	XMVECTOR rot = XMLoadFloat3(&rotation);
	XMVECTOR scl = XMLoadFloat3(&scale);

	XMMATRIX world; //the core matrix type we will be combining with
					//the resultant matrix should scale, then rotate, then translate
					//this ensures that rotations dont influence scaling, and translations don't influence rotations

	world = XMMatrixScalingFromVector(scl);
	world = XMMatrixMultiply(world, XMMatrixRotationRollPitchYawFromVector(rot));
	world = XMMatrixMultiply(world, XMMatrixTranslationFromVector(pos));

	//store the results of the transpose of the operation
	//the transpose is used because directX shaders operate on column-major storage
	//and at this point we are done using row-major math to calculate the matrix
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(world));
	XMStoreFloat4x4(&inverseTransposeWorld, XMMatrixInverse(nullptr, world));

	matrixOutdated = false; //we have just updated the matrix, no need to recalculate yet
}


///<summary>
///Computes and stores the entity's local forward, right, and up vectors
///</summary>
void Transform::CalculateLocalDirections()
{
	// Direction vectors can be found by rotating their global analogs
	XMStoreFloat3(&forward, XMVector3Rotate(XMVectorSet(0, 0, 1, 0), XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&rotation))));
	XMStoreFloat3(&right, XMVector3Rotate(XMVectorSet(1, 0, 0, 0), XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&rotation))));
	XMStoreFloat3(&up, XMVector3Rotate(XMVectorSet(0, 1, 0, 0), XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&rotation))));
}
