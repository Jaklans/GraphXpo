#include "FPSController.h"


///<summary>
///Default construtor, sets camera.
///</summary>
FPSController::FPSController(Camera* fpsCam)
{
	cam = fpsCam;
	transform = new Transform(); //create the entity's transform object
}

///<summary>
///Accept the mesh data and set the default transformation and camera.
///</summary>
FPSController::FPSController(std::shared_ptr<Mesh> const& meshObj, std::shared_ptr<Material> const& materialObj, Camera* fpsCam):GameEntity(meshObj, materialObj)
{
	cam = fpsCam;
}

///<summary>
///Upadtes transforms based on player input.
///</summary>
void FPSController::Update(float deltaTime)
{
	//Rotate player based on camera
	transform->SetRotationY(cam->transform.GetRotation().y);


	//Player input
	HandleInput(deltaTime);
	if (transform->matrixOutdated)
		transform->CalculateWorldMatrix();

	//Set Camera position to player position
	cam->transform.SetPosition(transform->GetPosition());

	//Update Camera transform/matrices
	cam->Update(deltaTime);

}

void FPSController::HandleInput(float deltaTime)
{
	//move the player
	if (GetAsyncKeyState('W') & 0x8000)  //forward
	{
		transform->TranslateForward(deltaTime * 4.0f);
	}
	else if (GetAsyncKeyState('S') & 0x8000) //backwards
	{
		transform->TranslateForward(deltaTime * -4.0f);
	}
	if (GetAsyncKeyState('D') & 0x8000) //right
	{
		transform->TranslateLocal(deltaTime * 4.0f, 0, 0);
	}
	else if (GetAsyncKeyState('A') & 0x8000) //left
	{
		transform->TranslateLocal(deltaTime * -4.0f, 0, 0);
	}
	if (GetAsyncKeyState('X') & 0x8000) //up
	{
		transform->TranslateLocal(0.0f, deltaTime * 4.0f, 0);
	}
	if (GetAsyncKeyState('C') & 0x8000) //up
	{
		transform->TranslateLocal(0.0f, deltaTime * -4.0f, 0);
	}
}
