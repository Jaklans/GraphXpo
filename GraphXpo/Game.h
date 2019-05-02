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
	void DrawShadowMaps();
	void DrawSky();
	void DrawWater(float totalTime);

	// Overridden mouse input helper methods
	void OnMouseDown(WPARAM buttonState, int x, int y);
	void OnMouseUp(WPARAM buttonState, int x, int y);
	void OnMouseMove(WPARAM buttonState, int x, int y);
	void OnMouseWheel(float wheelDelta, int x, int y);
private:

	// Initialization helper methods - feel free to customize, combine, etc.
	void LoadAssets();
	void CreateMatrices();
	void CreateBasicGeometry();
	void PostProcessing();

	Camera* camera;
	FPSController* player;
	bool rotating;

	//Lights
	DirectionalLight* directionalLight;
	std::vector<PointLight> lights;

	//Each mesh contains geometry data for drawing
	std::shared_ptr<Mesh> meshes[8];


	// GameEntity objects
	GameEntity* gameEntities[44];

	GameEntity* flatWater;

	std::shared_ptr<Material> barkMaterial;
	std::shared_ptr<Material> carpetMaterial;
	std::shared_ptr<Material> ceilingMaterial;
	std::shared_ptr<Material> marbleMaterial;
	std::shared_ptr<Material> marbleWallMaterial;
	std::shared_ptr<Material> skyMaterial;
	std::shared_ptr<Material> spaceshipMaterial;
	std::shared_ptr<Material> rockMaterial;
	std::shared_ptr<Material> logMaterial;
	std::shared_ptr<Material> dirtMaterial;
	std::shared_ptr<Material> caveMaterial;
	std::shared_ptr<Material> waterMaterial;


	// Wrappers for DirectX shaders to provide simplified functionality
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimplePixelShader> pbrPixelShader;
	std::shared_ptr<SimplePixelShader> waterPixelShader;
	std::shared_ptr<SimplePixelShader> refractiveMaskPS;
	std::shared_ptr<SimplePixelShader> combineRefractionPS;

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
	Emitter* campfireEmitter;

	// Shadows
	int shadowMapSize = 1024;
	ID3D11DepthStencilView* shadowDSV;
	ID3D11ShaderResourceView* shadowSRV;
	ID3D11SamplerState* shadowSampler;
	ID3D11RasterizerState* shadowRasterizer;
	std::shared_ptr<SimpleVertexShader> shadowVS;

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
	std::shared_ptr<SimplePixelShader> bloomBlurHPS;
	std::shared_ptr<SimplePixelShader> bloomBlurVPS;
	std::shared_ptr<SimplePixelShader> motionBlurPS;

	//Render target and tetxure for bloom blur
	ID3D11RenderTargetView* bloom1RTV;
	ID3D11ShaderResourceView* bloom1SRV;

	//Render target and tetxure for bloom blur 2
	ID3D11RenderTargetView* bloom2RTV;
	ID3D11ShaderResourceView* bloom2SRV;

	//Render target and tetxure for motion blur
	ID3D11RenderTargetView* motionBlurRTV;
	ID3D11ShaderResourceView* motionBlurSRV;


#pragma endregion


	//Refraction assets
	ID3D11RenderTargetView* nonRefractiveRTV;
	ID3D11ShaderResourceView* nonRefractiveSRV;

	ID3D11RenderTargetView* refractiveRTV;
	ID3D11ShaderResourceView* refractiveSRV;

	ID3D11RenderTargetView* refractiveMaskRTV;
	ID3D11ShaderResourceView* refractiveMaskSRV;

	ID3D11SamplerState* clampedSampler;


};

