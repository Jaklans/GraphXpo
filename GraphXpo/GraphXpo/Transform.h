#pragma once
#include <DirectXMath.h>

class Transform
{
public:
	Transform();
	~Transform();


	//Relative transformations

	///<summary>
	///Adjust the current position by some specified quantity.
	///</summary>
	void Translate(DirectX::XMFLOAT3 translation);
	void Translate(float x, float y, float z);

	///<summary>
	///Adjust the current position relative to the entity's local axes.
	///</summary>
	void TranslateLocal(DirectX::XMFLOAT3 translation);
	void TranslateLocal(float x, float y, float z);

	///<summary>
	///Adjust the current position along the entity's forward vector.
	///</summary>
	void TranslateForward(float magnitude);

	///<summary>
	///Adjust the current rotation by some specified quantity.
	///</summary>
	void Rotate(DirectX::XMFLOAT3 eulers);
	void Rotate(float xAngle, float yAngle, float zAngle);
	void Rotate(DirectX::XMFLOAT3 axis, float angle); //rotate about a specified axis

	//Accessors and Mutators

	DirectX::XMFLOAT4X4 GetWorldMatrix();
	DirectX::XMFLOAT4X4 GetInverseTranspose();

	DirectX::XMFLOAT3 GetPosition();
	void SetPosition(DirectX::XMFLOAT3 pos);
	void SetPosition(float x, float y, float z);

	DirectX::XMFLOAT3 GetRotation();
	void SetRotation(DirectX::XMFLOAT3 rot);
	void SetRotation(float xAngle, float yAngle, float zAngle);

	DirectX::XMFLOAT3 GetScale();
	void SetScale(DirectX::XMFLOAT3 scl);
	void SetScale(float x, float y, float z);

	//local direction vector accessors
	DirectX::XMFLOAT3 GetForwardVector();
	DirectX::XMFLOAT3 GetRightVector();
	DirectX::XMFLOAT3 GetUpVector();

	//Individual component setting functions

	void SetPositionX(float x);
	void SetPositionY(float y);
	void SetPositionZ(float z);

	void SetRotationX(float xAngle);
	void SetRotationY(float yAngle);
	void SetRotationZ(float zAngle);

	void SetScaleX(float x);
	void SetScaleY(float y);
	void SetScaleZ(float z);

	bool matrixOutdated; //indicates when the world matrix needs to be recalculated

	///<summary>
	///Combines position, rotation, and scale data into a single matrix.
	///This combined transformation, when applied, will put the GameEntity into world coordinates
	///</summary>
	void CalculateWorldMatrix();

private:

	///<summary>
	///Computes and stores the entity's local forward, right, and up vectors
	///</summary>
	void CalculateLocalDirections();

	//transformation data
	DirectX::XMFLOAT3 position; //this game entity's position in cartesian coordinates
	DirectX::XMFLOAT3 rotation; //the euler angle representation of this entity's rotation
	DirectX::XMFLOAT3 scale;	//vector multiplier of the entity's size
	DirectX::XMFLOAT4X4 worldMatrix; //the combination of the above vectors. Transforms the entity to world coordinates
	DirectX::XMFLOAT4X4 inverseTransposeWorld; //matrix maintaining the rotations, but inverting the scale. Useful for transforming normals.

	//local direction vectors
	DirectX::XMFLOAT3 forward;
	DirectX::XMFLOAT3 right;
	DirectX::XMFLOAT3 up;
};

