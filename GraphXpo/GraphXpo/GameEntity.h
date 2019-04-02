#pragma once
#include "Transform.h"
#include "Mesh.h"
#include "Material.h"
#include <memory>

class GameEntity
{
public:
	///<summary>
	///Default Constructor
	///</summary>
	GameEntity();

	///<summary>
	///Accept the mesh data and set the default transformation.
	///</summary>
	
	GameEntity(std::shared_ptr<Mesh> const& meshObj, std::shared_ptr<Material> const& materialObj);
	virtual ~GameEntity();
	
	///<summary>
	///Set shader data and activate the shaders.
	///</summary>
	void PrepareMaterial(DirectX::XMFLOAT4X4 view, DirectX::XMFLOAT4X4 projection);

	Transform* transform; //holds all data for moving, rotating, and scaling the game entity. Also contains the entity's world matrix

	std::shared_ptr<Mesh> mesh; //this object's mesh representation. Pointer is used so that mesh data can be shared
	std::shared_ptr<Material> material;

	float uvScale; // Float value to scale the game entities UVs by (default of 1)
	void SetUVScale(float scale);
};

