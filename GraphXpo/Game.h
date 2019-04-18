#pragma once

#include "DXCore.h"
#include "SimpleShader.h"
#include <DirectXMath.h>
#include <memory>
#include <vector>

#include "Mesh.h"
#include "GameEntity.h"
#include "Material.h"
#include "Camera.h"
#include "Lights.h"
#include "FPSController.h"
#include "Emitter.h"

class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	void DrawSky();

	// Overridden mouse input helper methods
	void OnMouseDown (WPARAM buttonState, int x, int y);
	void OnMouseUp	 (WPARAM buttonState, int x, int y);
	void OnMouseMove (WPARAM buttonState, int x, int y);
	void OnMouseWheel(float wheelDelta,   int x, int y);
private:

	// Initialization helper methods - feel free to customize, combine, etc.
	void LoadAssets(); 
	void CreateMatrices();
	void CreateBasicGeometry();
	void PostProcessing();

	Camera* camera;
	FPSController* player;
	bool rotating;

	//lights
	std::vector<Light> lights;

	//Each mesh contains geometry data for drawing
	std::shared_ptr<Mesh> meshes[5];

	// GameEntity objects
	GameEntity* gameEntities[23];

	std::shared_ptr<Material> barkMaterial;
	std::shared_ptr<Material> carpetMaterial;
	std::shared_ptr<Material> ceilingMaterial;
	std::shared_ptr<Material> marbleMaterial;
	std::shared_ptr<Material> marbleWallMaterial;
	std::shared_ptr<Material> skyMaterial;
	std::shared_ptr<Material> spaceshipMaterial;

	// Wrappers for DirectX shaders to provide simplified functionality
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimplePixelShader> pbrPixelShader;

	// The matrices to go from model space to screen space
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;


	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;

	// Skybox
	std::shared_ptr<SimpleVertexShader> skyVertexShader;
	std::shared_ptr<SimplePixelShader> skyPixelShader;
	ID3D11RasterizerState* skyRasterizerState;
	ID3D11DepthStencilState* skyDepthStencilState;

	// Particles
	ID3D11ShaderResourceView* particleTexture;
	std::shared_ptr<SimpleVertexShader> particleVertexShader;
	std::shared_ptr<SimplePixelShader> particlePixelShader;
	ID3D11DepthStencilState* particleDepthStencilState;
	ID3D11BlendState* particleBlendState;
	Emitter* thrusterEmitter;
	Emitter* thrusterEmitter2;
	Emitter* thrusterEmitter3;

	//POST-PROCESSING RESOURCES

	bool postProcessing;

	//General PP Assets
	std::shared_ptr<SimpleVertexShader> postProcessVS;
	ID3D11SamplerState* sampler;
	ID3D11RenderTargetView* postProcessRTV;		// Allows us to render to a texture
	ID3D11ShaderResourceView* postProcessSRV;	// Allows us to sample from the same texture

#pragma region Bloom
	//Bloom Pixel Shaders
	std::shared_ptr<SimplePixelShader> brightExtractPS;
	std::shared_ptr<SimplePixelShader> bloomBlurPS;


	//Render target and tetxure for bloom blur
	ID3D11RenderTargetView* bloomRTV;
	ID3D11ShaderResourceView* bloomSRV;


#pragma endregion

};

