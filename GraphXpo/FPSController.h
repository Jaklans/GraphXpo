#pragma once
#include "GameEntity.h"
#include "Camera.h"
class FPSController :
	public GameEntity
{
public:
	///<summary>
	///Default construtor, sets camera.
	///</summary>
	FPSController(Camera* fpsCam);

	///<summary>
	///Accept the mesh data and set the default transformation and camera.
	///</summary>
	FPSController(std::shared_ptr<Mesh> const& meshObj, std::shared_ptr<Material> const& materialObj, Camera* fpsCam);

	///<summary>
	///Upadtes transforms based on player input.
	///</summary>
	void Update(float deltaTime);
	void HandleInput(float deltaTime);
private:
	Camera* cam;

};

