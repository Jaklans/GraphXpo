#include "GameEntity.h"


///<summary>
///Default Constructor
///</summary>
GameEntity::GameEntity()
{
}

///<summary>
///Accept the mesh data and set the default transformation.
///</summary>
GameEntity::GameEntity(std::shared_ptr<Mesh> const& meshObj, std::shared_ptr<Material> const& materialObj)
{
	mesh = meshObj;
	material = materialObj;
	transform = new Transform(); //create the entity's transform object
	uvScale = 1;
}

GameEntity::~GameEntity()
{
	delete transform;
}

///<summary>
///Set shader data and activate the shaders.
///</summary>
void GameEntity::PrepareMaterial(DirectX::XMFLOAT4X4 view, DirectX::XMFLOAT4X4 projection)
{
	// Send data to shader variables
	//  - Do this ONCE PER OBJECT you're drawing
	//  - This is actually a complex process of copying data to a local buffer
	//    and then copying that entire buffer to the GPU.  
	//  - The "SimpleShader" class handles all of that for you.
	material->GetVertexShader()->SetMatrix4x4("world", transform->GetWorldMatrix());
	material->GetVertexShader()->SetMatrix4x4("invTransWorld", transform->GetInverseTranspose());
	material->GetVertexShader()->SetMatrix4x4("view", view);
	material->GetVertexShader()->SetMatrix4x4("projection", projection);
	material->GetVertexShader()->SetFloat("uvScale", uvScale);

	// Once you've set all of the data you care to change for
	// the next draw call, you need to actually send it to the GPU
	//  - If you skip this, the "SetMatrix" calls above won't make it to the GPU!
	material->GetVertexShader()->CopyAllBufferData();

	material->GetPixelShader()->CopyAllBufferData();

	// Set the vertex and pixel shaders to use for the next Draw() command
	//  - These don't technically need to be set every frame...YET
	//  - Once you start applying different shaders to different objects,
	//    you'll need to swap the current shaders before each draw
	material->GetVertexShader()->SetShader();
	material->GetPixelShader()->SetShader();
}

void GameEntity::SetUVScale(float scale)
{
	uvScale = scale;
}
