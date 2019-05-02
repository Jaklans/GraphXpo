#include "Game.h"
#include "Vertex.h"
#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"

using namespace DirectX;

Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,		// The application's handle
		"DirectX Game",	// Text for the window's title bar
		1280,			// Width of the window's client area
		720,			// Height of the window's client area
		true)			// Show extra stats (fps) in title bar?
{
	// Initialize fields
	vertexShader = 0;
	pixelShader = 0;

	meshes[0] = nullptr;
	meshes[1] = nullptr;
	meshes[2] = nullptr;
	meshes[3] = nullptr;
	meshes[4] = nullptr;
	meshes[5] = nullptr;
	meshes[6] = nullptr;
	meshes[7] = nullptr;


	postProcessing = true; //Toggle Post-processing effects

#if defined(DEBUG) || defined(_DEBUG)
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif
	
}

Game::~Game()
{
	// Release any (and all!) DirectX objects
	// we've made in the Game class

	//I've opted to wrap meshes, shaders, and materials in shared_ptrs, so they don't require manual deallocation

	//delete all of the game objects
	for (size_t i = 0; i < 44; i++)
	{
		delete gameEntities[i];
	}


	delete flatWater;

	sampler->Release();

	//Release Post-Process resources
	bloom1RTV->Release();
	bloom1SRV->Release();

	bloom2RTV->Release();
	bloom2SRV->Release();

	motionBlurRTV->Release();
	motionBlurSRV->Release();


	delete player;
	delete camera;

	// Release particle resources
	particleTexture->Release();
	particleBlendState->Release();
	particleDepthStencilState->Release();
	delete thrusterEmitter;
	delete thrusterEmitter2;
	delete thrusterEmitter3;
	delete campfireEmitter;

	// Release sky resources
	skyDepthStencilState->Release();
	skyRasterizerState->Release();
}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	camera = new Camera();
	player = new FPSController(camera);

	//Initialize cursor settings
	ShowCursor(false);
	prevMousePos.x = width / 2;
	prevMousePos.y = height / 2;

	rotating = false;
	printf("WASD to move. Space/X for vertical movement. Click and drag to rotate.");

	LoadAssets();
	CreateMatrices();
	CreateBasicGeometry();

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//set up the directional lights
	Light dl1;
	dl1.Type = LIGHT_TYPE_DIR;
	dl1.Color = XMFLOAT3(0.5f, 0.5f, 0.5f);
	dl1.Direction = XMFLOAT3(0.25f, -1.55f, 0.5f);
	dl1.Intensity = 0.5f;

	Light dl2;
	dl2.Type = LIGHT_TYPE_POINT;
	dl2.Color = XMFLOAT3(0.4f, 0.2f, 0.4f);
	dl2.Position = XMFLOAT3(0.0f, 1.0f, -2.5f);
	dl2.Range = 5.0f;
	dl2.Intensity = 3.0f;

	Light dl3;
	dl3.Type = LIGHT_TYPE_POINT;
	dl3.Color = XMFLOAT3(0.8f, 0.8f, 0.8f);
	dl3.Position = XMFLOAT3(-3.3f, 0.5f, -2.8f);
	dl3.Range = 3.0f;
	dl3.Intensity = 2.0f;

	Light dl4;
	dl4.Type = LIGHT_TYPE_POINT;
	dl4.Color = XMFLOAT3(1.0f, 0.2f, 0.2f);
	dl4.Position = XMFLOAT3(1.6f, 1.25f, 9.7f);
	dl4.Range = 2.5f;
	dl4.Intensity = 2.5f;

	Light dl5;
	dl5.Type = LIGHT_TYPE_POINT;
	dl5.Color = XMFLOAT3(0.2f, 0.2f, 1.0f);
	dl5.Position = XMFLOAT3(-1.6f, 1.25f, 9.7f);
	dl5.Range = 2.5f;
	dl5.Intensity = 2.5f;

	Light dl6;
	dl6.Type = LIGHT_TYPE_POINT;
	dl6.Color = XMFLOAT3(0.62f, 0.62f, 0.62f);
	dl6.Position = XMFLOAT3(0.0f, 1.85f, 9.7f);
	dl6.Range = 2.0f;
	dl6.Intensity = 1.5f;

	Light dl7;
	dl7.Type = LIGHT_TYPE_POINT;
	dl7.Color = XMFLOAT3(0.972f, 0.823f, 0.686f);
	dl7.Position = XMFLOAT3(1.2f, -0.46f, 17.0f);
	dl7.Range = 4.0f;
	dl7.Intensity = 1.5f;

	lights.emplace_back(dl1);
	lights.emplace_back(dl2);
	lights.emplace_back(dl3);
	lights.emplace_back(dl4);
	lights.emplace_back(dl5);
	lights.emplace_back(dl6);
	lights.emplace_back(dl7);

	// Set up the Emitters
	thrusterEmitter = new Emitter(
		80,									// Max particles
		50,										// Particles per second
		1.6f,									// Particle lifetime
		0.33f,									// Start size
		0.0f,									// End size
		XMFLOAT4(0.788f, 0.094f, 0.094f, 0.0f),	// Start color
		XMFLOAT4(0.972f, 0.823f, 0.686f, 1.0f),	// End color
		XMFLOAT3(-0.84f, 0.115f, 9.82f),		// Emitter position
		XMFLOAT3(-0.7f, -0.25f, 0.7f),			// Constant acceleration
		XMFLOAT3(-0.48f, -0.18f, 0.48f),		// Start velocity
		XMFLOAT3(0.1f, 0.05f, 0.0f),			// Velocity randomness range
		XMFLOAT3(0, 0, 0),						// Position randomness range
		XMFLOAT4(0, 0, -2, 2),					// Random rotation ranges (startMin, startMax, endMin, endMax)
		device,
		particleTexture,
		particleVertexShader,
		particlePixelShader
	);

	thrusterEmitter2 = new Emitter(
		40,										// Max particles
		23,										// Particles per second
		1.3f,									// Particle lifetime
		0.24f,									// Start size
		0.0f,									// End size
		XMFLOAT4(0.788f, 0.094f, 0.094f, 0.0f),	// Start color
		XMFLOAT4(0.972f, 0.823f, 0.686f, 1.0f),	// End color
		XMFLOAT3(-0.52f, 0.0f, 10.02f),			// Emitter position
		XMFLOAT3(-0.7f, -0.25f, 0.7f),			// Constant acceleration
		XMFLOAT3(-0.48f, -0.18f, 0.48f),		// Start velocity
		XMFLOAT3(0.1f, 0.05f, 0.0f),			// Velocity randomness range
		XMFLOAT3(0, 0, 0),						// Position randomness range
		XMFLOAT4(0, 0, -2, 2),					// Random rotation ranges (startMin, startMax, endMin, endMax)
		device,
		particleTexture,
		particleVertexShader,
		particlePixelShader
	);

	thrusterEmitter3 = new Emitter(
		40,										// Max particles
		23,										// Particles per second
		1.3f,									// Particle lifetime
		0.24f,									// Start size
		0.0f,									// End size
		XMFLOAT4(0.788f, 0.094f, 0.094f, 0.0f),	// Start color
		XMFLOAT4(0.972f, 0.823f, 0.686f, 1.0f),	// End color
		XMFLOAT3(-1.08f, 0.0f, 9.52f),			// Emitter position
		XMFLOAT3(-0.7f, -0.25f, 0.7f),			// Constant acceleration
		XMFLOAT3(-0.48f, -0.18f, 0.48f),		// Start velocity
		XMFLOAT3(0.1f, 0.05f, 0.0f),			// Velocity randomness range
		XMFLOAT3(0, 0, 0),						// Position randomness range
		XMFLOAT4(0, 0, -2, 2),					// Random rotation ranges (startMin, startMax, endMin, endMax)
		device,
		particleTexture,
		particleVertexShader,
		particlePixelShader
	);

	campfireEmitter = new Emitter(
		120,									// Max particles
		40,										// Particles per second
		2.8f,									// Particle lifetime
		0.20f,									// Start size
		0.02f,									// End size
		XMFLOAT4(0.972f, 0.823f, 0.686f, 1.0f),	// Start color
		XMFLOAT4(0.877f, 0.877f, 0.877f, 0.0f),	// End color
		XMFLOAT3(1.2f, -0.66f, 17.0f),			// Emitter position
		XMFLOAT3(0.0f, -0.18f, 0.0f),			// Constant acceleration
		XMFLOAT3(0.0f, 0.60f, 0.0f),			// Start velocity
		XMFLOAT3(0.0f, 0.15f, 0.0f),			// Velocity randomness range
		XMFLOAT3(0.12f, 0.1f, 0.12f),			// Position randomness range
		XMFLOAT4(0, 0, -1, 1),					// Random rotation ranges (startMin, startMax, endMin, endMax)
		device,
		particleTexture,
		particleVertexShader,
		particlePixelShader
	);
}

void Game::LoadAssets()
{
	//Sampler Creation
	D3D11_SAMPLER_DESC samplerDesc = {};

	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, &sampler);

	D3D11_SAMPLER_DESC clampedDesc = {};

	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 1;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, &clampedSampler);

#pragma region shader loading
	vertexShader = std::make_shared<SimpleVertexShader>(device, context);
	vertexShader->LoadShaderFile(L"VertexShader.cso");

	pixelShader = std::make_shared<SimplePixelShader>(device, context);
	pixelShader->LoadShaderFile(L"PixelShader.cso");

	pbrPixelShader = std::make_shared<SimplePixelShader>(device, context);
	pbrPixelShader->LoadShaderFile(L"PBRPixelShader.cso");

	// Load Skybox shaders
	skyVertexShader = std::make_shared<SimpleVertexShader>(device, context);
	skyVertexShader->LoadShaderFile(L"SkyVertexShader.cso");

	skyPixelShader = std::make_shared<SimplePixelShader>(device, context);
	skyPixelShader->LoadShaderFile(L"SkyPixelShader.cso");

	//Load Post-Processing shaders
	postProcessVS = std::make_shared<SimpleVertexShader>(device, context);
	postProcessVS->LoadShaderFile(L"PostProcessVS.cso");

	brightExtractPS = std::make_shared<SimplePixelShader>(device, context);
	brightExtractPS->LoadShaderFile(L"BrightnessExtractPS.cso");

	bloomBlurHPS = std::make_shared<SimplePixelShader>(device, context);
	bloomBlurHPS->LoadShaderFile(L"BloomBlurHorizontalPS.cso");

	bloomBlurVPS = std::make_shared<SimplePixelShader>(device, context);
	bloomBlurVPS->LoadShaderFile(L"BloomBlurVerticalPS.cso");

	motionBlurPS = std::make_shared<SimplePixelShader>(device, context);
	motionBlurPS->LoadShaderFile(L"MotionBlurPS.cso");

	// Load Particle shaders
	particleVertexShader = std::make_shared<SimpleVertexShader>(device, context);
	particleVertexShader->LoadShaderFile(L"ParticleVertexShader.cso");

	particlePixelShader = std::make_shared<SimplePixelShader>(device, context);
	particlePixelShader->LoadShaderFile(L"ParticlePixelShader.cso");

	//Load Water Shaders
	waterPixelShader = std::make_shared<SimplePixelShader>(device, context);
	waterPixelShader->LoadShaderFile(L"WaterPixelShader.cso");

	refractiveMaskPS = std::make_shared<SimplePixelShader>(device, context);
	refractiveMaskPS->LoadShaderFile(L"RefractiveMaskPS.cso");

	combineRefractionPS = std::make_shared<SimplePixelShader>(device, context);
	combineRefractionPS->LoadShaderFile(L"CombineRefractionPS.cso");
	

#pragma endregion

#pragma region general texture loading
	ID3D11ShaderResourceView* barkSRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\rock.jpg", 0, &barkSRV);
	ID3D11ShaderResourceView* bark_s_SRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\rock_s.jpg", 0, &bark_s_SRV);
	ID3D11ShaderResourceView* bark_n_SRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\rock_n.jpg", 0, &bark_n_SRV);

	ID3D11ShaderResourceView* brickSRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\brick.jpg", 0, &brickSRV);
	ID3D11ShaderResourceView* brick_s_SRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\brick_s.jpg", 0, &brick_s_SRV);
	ID3D11ShaderResourceView* brick_n_SRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\brick_n.jpg", 0, &brick_n_SRV);

	ID3D11ShaderResourceView* ceilingSRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\ceiling.tif", 0, &ceilingSRV);
	ID3D11ShaderResourceView* ceiling_s_SRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\ceiling_roughness.tif", 0, &ceiling_s_SRV);
	ID3D11ShaderResourceView* ceiling_n_SRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\ceiling_n.tif", 0, &ceiling_n_SRV);

	ID3D11ShaderResourceView* marbleSRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\marble.tif", 0, &marbleSRV);
	ID3D11ShaderResourceView* marble_s_SRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\marble_roughness.tif", 0, &marble_s_SRV);
	ID3D11ShaderResourceView* marble_n_SRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\marble_n.tif", 0, &marble_n_SRV);

	ID3D11ShaderResourceView* marbleWallSRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\marble_wall.tif", 0, &marbleWallSRV);
	ID3D11ShaderResourceView* marbleWall_s_SRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\marble_wall_roughness.tif", 0, &marbleWall_s_SRV);
	ID3D11ShaderResourceView* marbleWall_n_SRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\marble_wall_n.tif", 0, &marbleWall_n_SRV);

	ID3D11ShaderResourceView* spaceshipSRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\StarSparrow_Purple.png", 0, &spaceshipSRV);
	ID3D11ShaderResourceView* spaceship_n_SRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\StarSparrow_Normal.png", 0, &spaceship_n_SRV);
	ID3D11ShaderResourceView* spaceship_m_SRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\StarSparrow_MetallicSmoothness.png", 0, &spaceship_m_SRV);

	ID3D11ShaderResourceView* metalSRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\grimy-metal_d.png", 0, &metalSRV);
	ID3D11ShaderResourceView* metal_n_SRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\grimy-metal_n.png", 0, &metal_n_SRV);
	ID3D11ShaderResourceView* metal_m_SRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\grimy-metal_m.png", 0, &metal_m_SRV);
	ID3D11ShaderResourceView* metal_r_SRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\grimy-metal_r.png", 0, &metal_r_SRV);

	ID3D11ShaderResourceView* rockSRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\sharprock_d.png", 0, &rockSRV);
	ID3D11ShaderResourceView* rock_n_SRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\sharprock_n.png", 0, &rock_n_SRV);
	ID3D11ShaderResourceView* rock_m_SRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\sharprock_m.png", 0, &rock_m_SRV);
	ID3D11ShaderResourceView* rock_r_SRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\sharprock_r.png", 0, &rock_r_SRV);

	ID3D11ShaderResourceView* logSRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\log.png", 0, &logSRV);

	ID3D11ShaderResourceView* dirtSRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\dirt-d.png", 0, &dirtSRV);
	ID3D11ShaderResourceView* dirt_n_SRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\dirt-n.png", 0, &dirt_n_SRV);
	ID3D11ShaderResourceView* dirt_m_SRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\dirt-m.png", 0, &dirt_m_SRV);
	ID3D11ShaderResourceView* dirt_r_SRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\dirt-r.png", 0, &dirt_r_SRV);

	ID3D11ShaderResourceView* caveSRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\cave_d.png", 0, &caveSRV);
	ID3D11ShaderResourceView* cave_n_SRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\cave_n.png", 0, &cave_n_SRV);
	ID3D11ShaderResourceView* cave_m_SRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\cave_m.png", 0, &cave_m_SRV);
	ID3D11ShaderResourceView* cave_r_SRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\cave_r.png", 0, &cave_r_SRV);

	ID3D11ShaderResourceView* water_norm;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\water_n.png", 0, &water_norm);


	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\fireParticle.jpg", 0, &particleTexture);

#pragma endregion

#pragma region skybox loading
	ID3D11ShaderResourceView* skySRV;
	CreateDDSTextureFromFile(device, L"..\\..\\Assets\\Textures\\NightSkyCubemap.dds", 0, &skySRV);

	barkMaterial = std::make_shared<Material>(vertexShader, pixelShader, barkSRV, bark_s_SRV, bark_n_SRV, sampler);
	carpetMaterial = std::make_shared<Material>(vertexShader, pbrPixelShader, metalSRV, metal_m_SRV, metal_r_SRV, metal_n_SRV, sampler);
	ceilingMaterial = std::make_shared<Material>(vertexShader, pixelShader, ceilingSRV, ceiling_s_SRV, ceiling_n_SRV, sampler);
	marbleMaterial = std::make_shared<Material>(vertexShader, pixelShader, marbleSRV, marble_s_SRV, marble_n_SRV, sampler);
	marbleWallMaterial = std::make_shared<Material>(vertexShader, pixelShader, marbleWallSRV, marbleWall_s_SRV, marbleWall_n_SRV, sampler);
	spaceshipMaterial = std::make_shared<Material>(vertexShader, pbrPixelShader, spaceshipSRV, spaceship_m_SRV, spaceship_m_SRV, spaceship_n_SRV, sampler);
	skyMaterial = std::make_shared<Material>(skyVertexShader, skyPixelShader, skySRV, nullptr, nullptr, sampler);
	rockMaterial = std::make_shared<Material>(vertexShader, pbrPixelShader, rockSRV, rock_m_SRV, rock_r_SRV, rock_n_SRV, sampler);
	logMaterial = std::make_shared<Material>(vertexShader, pixelShader, logSRV, nullptr, nullptr, sampler);
	dirtMaterial = std::make_shared<Material>(vertexShader, pbrPixelShader, dirtSRV, dirt_m_SRV, dirt_r_SRV, dirt_n_SRV, sampler);
	caveMaterial = std::make_shared<Material>(vertexShader, pbrPixelShader, caveSRV, cave_m_SRV, cave_r_SRV, cave_n_SRV, sampler);
	waterMaterial = std::make_shared<Material>(vertexShader, waterPixelShader, nullptr, nullptr, water_norm, sampler);


	// Rasterizer and DepthStencil states for the skybox
	D3D11_RASTERIZER_DESC skyRD = {};
	skyRD.CullMode = D3D11_CULL_FRONT;
	skyRD.FillMode = D3D11_FILL_SOLID;
	skyRD.DepthClipEnable = true;
	device->CreateRasterizerState(&skyRD, &skyRasterizerState);

	D3D11_DEPTH_STENCIL_DESC skyDS = {};
	skyDS.DepthEnable = true;
	skyDS.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	skyDS.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&skyDS, &skyDepthStencilState);

#pragma endregion

#pragma region post process resource loading
	// Create post process resources
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;

	// Create the Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	// Create the Shader Resource View
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	//General Post Process render texture
	ID3D11Texture2D* ppTexture;
	device->CreateTexture2D(&textureDesc, 0, &ppTexture);
	device->CreateRenderTargetView(ppTexture, &rtvDesc, &postProcessRTV);
	device->CreateShaderResourceView(ppTexture, &srvDesc, &postProcessSRV);

	//Bloom resources
	ID3D11Texture2D* bloomTexture1;
	device->CreateTexture2D(&textureDesc, 0, &bloomTexture1);
	device->CreateRenderTargetView(bloomTexture1, &rtvDesc, &bloom1RTV);
	device->CreateShaderResourceView(bloomTexture1, &srvDesc, &bloom1SRV);

	ID3D11Texture2D* bloomTexture2;
	device->CreateTexture2D(&textureDesc, 0, &bloomTexture2);
	device->CreateRenderTargetView(bloomTexture2, &rtvDesc, &bloom2RTV);
	device->CreateShaderResourceView(bloomTexture2, &srvDesc, &bloom2SRV);

	ID3D11Texture2D* motionBlurTexture;
	device->CreateTexture2D(&textureDesc, 0, &motionBlurTexture);
	device->CreateRenderTargetView(motionBlurTexture, &rtvDesc, &motionBlurRTV);
	device->CreateShaderResourceView(motionBlurTexture, &srvDesc, &motionBlurSRV);

	// We don't need the texture references 
	ppTexture->Release();
	bloomTexture1->Release();
	bloomTexture2->Release();
	motionBlurTexture->Release();
#pragma endregion

#pragma region particles loading
	// A depth state for the particles
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // Turns off depth writing
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	device->CreateDepthStencilState(&dsDesc, &particleDepthStencilState);


	// Blend for particles (additive)
	D3D11_BLEND_DESC blend = {};
	blend.AlphaToCoverageEnable = false;
	blend.IndependentBlendEnable = false;
	blend.RenderTarget[0].BlendEnable = true;
	blend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA; // Still respect pixel shader output alpha
	blend.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	device->CreateBlendState(&blend, &particleBlendState);


#pragma endregion

#pragma region Water loading


	//General Post Process render texture
	ID3D11Texture2D* nonRefractiveTexture;
	device->CreateTexture2D(&textureDesc, 0, &nonRefractiveTexture);
	device->CreateRenderTargetView(nonRefractiveTexture, &rtvDesc, &nonRefractiveRTV);
	device->CreateShaderResourceView(nonRefractiveTexture, &srvDesc, &nonRefractiveSRV);

	ID3D11Texture2D* refractiveTexture;
	device->CreateTexture2D(&textureDesc, 0, &refractiveTexture);
	device->CreateRenderTargetView(refractiveTexture, &rtvDesc, &refractiveRTV);
	device->CreateShaderResourceView(refractiveTexture, &srvDesc, &refractiveSRV);

	ID3D11Texture2D* refractiveMaskTexture;
	device->CreateTexture2D(&textureDesc, 0, &refractiveMaskTexture);
	device->CreateRenderTargetView(refractiveMaskTexture, &rtvDesc, &refractiveMaskRTV);
	device->CreateShaderResourceView(refractiveMaskTexture, &srvDesc, &refractiveMaskSRV);

	// We don't need the texture references 
	nonRefractiveTexture->Release();
	refractiveTexture->Release();
	refractiveMaskTexture->Release();

#pragma endregion
}


void Game::CreateMatrices()
{
	// Set up world matrix
	XMMATRIX W = XMMatrixIdentity();
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(W));

	// Create the View matrix
	// - In an actual game, recreate this matrix every time the camera 
	//    moves (potentially every frame)
	// - We're using the LOOK TO function, which takes the position of the
	//    camera and the direction vector along which to look (as well as "up")
	// - Another option is the LOOK AT function, to look towards a specific
	//    point in 3D space
	XMVECTOR pos = XMVectorSet(0, 0, -5, 0);
	XMVECTOR dir = XMVectorSet(0, 0, 1, 0);
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMMATRIX V = XMMatrixLookToLH(
		pos,     // The position of the "camera"
		dir,     // Direction the camera is looking
		up);     // "Up" direction in 3D space (prevents roll)
	XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(V));

	// Create the Projection matrix
	// - This should match the window's aspect ratio, and also update anytime
	//    the window resizes (which is already happening in OnResize() below)
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,		// Field of View Angle
		(float)width / height,		// Aspect ratio
		0.1f,						// Near clip plane distance
		100.0f);					// Far clip plane distance
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P)); // Transpose for HLSL!

	camera->UpdateProjectionMatrix(width, height);
}


void Game::CreateBasicGeometry()
{
	meshes[0] = std::make_shared<Mesh>((char *)"..\\..\\Assets\\Models\\cube.obj", device);
	meshes[1] = std::make_shared<Mesh>((char *)"..\\..\\Assets\\Models\\sphere.obj", device);
	meshes[2] = std::make_shared<Mesh>((char *)"..\\..\\Assets\\Models\\torus.obj", device);
	meshes[3] = std::make_shared<Mesh>((char *)"..\\..\\Assets\\Models\\arch.obj", device);
	meshes[4] = std::make_shared<Mesh>((char *)"..\\..\\Assets\\Models\\spaceship.obj", device);
	meshes[5] = std::make_shared<Mesh>((char *)"..\\..\\Assets\\Models\\sharprock.obj", device);
	meshes[6] = std::make_shared<Mesh>((char *)"..\\..\\Assets\\Models\\log.obj", device);
	meshes[7] = std::make_shared<Mesh>((char *)"..\\..\\Assets\\Models\\plane.obj", device);


	// Create basic test geometry
	for (int i = 0; i < 10; i++)
	{
		if(i % 2 == 0)
			gameEntities[i] = new GameEntity(meshes[i%3], barkMaterial);
		else
			gameEntities[i] = new GameEntity(meshes[i % 3], carpetMaterial);

		gameEntities[i]->transform->SetPosition((-4.0f + (float)i), -0.5f, 1.2f);
		gameEntities[i]->transform->SetScale(0.5f, 0.5f, 0.5f);
	}

	// Create arches
	for (int i = 0; i < 8; i++) {
		gameEntities[i + 10] = new GameEntity(meshes[3], marbleMaterial);
		gameEntities[i + 10]->transform->SetPosition(-5.7f + 3.8f * (i % 4), -0.82f, ((i/4) * 12.4f) - 6.2f);
		gameEntities[i + 10]->transform->SetScale(0.47f, 0.47f, 0.47f);
	}

	// Create the ceiling
	gameEntities[18] = new GameEntity(meshes[0], ceilingMaterial);
	gameEntities[18]->transform->SetPosition(0.0f, 2.5f, 0.0f);
	gameEntities[18]->transform->SetScale(15.3f, 0.02f, 13.0f);
	gameEntities[18]->SetUVScale(9.0f);

	// Create the floor
	gameEntities[19] = new GameEntity(meshes[0], carpetMaterial);
	gameEntities[19]->transform->SetPosition(0.0f, -0.8f, 0.0f);
	gameEntities[19]->transform->SetScale(15.3f, 0.02f, 13.0f);
	gameEntities[19]->SetUVScale(4.2f);

	// Create two walls
	gameEntities[20] = new GameEntity(meshes[0], marbleWallMaterial);
	gameEntities[20]->transform->SetPosition(7.65f, 0.84f, 0.0f);
	gameEntities[20]->transform->SetScale(0.02f, 3.32f, 13.0f);
	gameEntities[20]->SetUVScale(4.8f);

	gameEntities[21] = new GameEntity(meshes[0], marbleWallMaterial);
	gameEntities[21]->transform->SetPosition(-7.65f, 0.84f, 0.0f);
	gameEntities[21]->transform->SetScale(0.02f, 3.32f, 13.0f);
	gameEntities[21]->SetUVScale(4.8f);

	// Create the spaceship
	gameEntities[22] = new GameEntity(meshes[4], spaceshipMaterial);
	gameEntities[22]->transform->SetPosition(0.0f, 0.44f, 9.0f);
	gameEntities[22]->transform->SetScale(0.0023f, 0.0023f, 0.0023f);
	gameEntities[22]->transform->SetRotation(0.22f, -0.8f, 0.0f);

	// Add logs
	gameEntities[23] = new GameEntity(meshes[6], logMaterial);
	gameEntities[23]->transform->SetPosition(1.2f, -0.68f, 17.0f);
	gameEntities[23]->transform->SetScale(0.32f, 0.32f, 0.32f);
	gameEntities[23]->transform->SetRotation(0.0f, 1.0f, 0.0f);

	gameEntities[24] = new GameEntity(meshes[6], logMaterial);
	gameEntities[24]->transform->SetPosition(1.2f, -0.68f, 17.0f);
	gameEntities[24]->transform->SetScale(0.32f, 0.32f, 0.32f);
	gameEntities[24]->transform->SetRotation(0.0f, 2.5708f, 0.0f);

	// Add walls for the rooms
	gameEntities[25] = new GameEntity(meshes[0], marbleWallMaterial);
	gameEntities[25]->transform->SetPosition(-5.35f, 0.84f, 4.6f);
	gameEntities[25]->transform->SetScale(0.02f, 3.32f, 6.0f);
	gameEntities[25]->transform->SetRotationY(1.0f);
	gameEntities[25]->SetUVScale(4.8f);

	gameEntities[26] = new GameEntity(meshes[0], marbleWallMaterial);
	gameEntities[26]->transform->SetPosition(5.35f, 0.84f, 4.6f);
	gameEntities[26]->transform->SetScale(0.02f, 3.32f, 6.0f);
	gameEntities[26]->transform->SetRotationY(-1.0f);
	gameEntities[26]->SetUVScale(4.8f);

	gameEntities[27] = new GameEntity(meshes[0], marbleWallMaterial);
	gameEntities[27]->transform->SetPosition(2.9f, 0.84f, 9.7f);
	gameEntities[27]->transform->SetScale(0.02f, 3.32f, 7.0f);
	gameEntities[27]->SetUVScale(4.8f);

	gameEntities[28] = new GameEntity(meshes[0], marbleWallMaterial);
	gameEntities[28]->transform->SetPosition(-2.9f, 0.84f, 9.7f);
	gameEntities[28]->transform->SetScale(0.02f, 3.32f, 7.0f);
	gameEntities[28]->SetUVScale(4.8f);

	gameEntities[29] = new GameEntity(meshes[0], marbleWallMaterial);
	gameEntities[29]->transform->SetPosition(2.0f, 0.84f, 13.2f);
	gameEntities[29]->transform->SetScale(0.05f, 3.32f, 1.8f);
	gameEntities[29]->transform->SetRotationY(1.5708);
	gameEntities[29]->SetUVScale(4.8f);

	gameEntities[30] = new GameEntity(meshes[0], marbleWallMaterial);
	gameEntities[30]->transform->SetPosition(-2.0f, 0.84f, 13.2f);
	gameEntities[30]->transform->SetScale(0.05f, 3.32f, 1.8f);
	gameEntities[30]->transform->SetRotationY(1.5708);
	gameEntities[30]->SetUVScale(4.8f);

	gameEntities[31] = new GameEntity(meshes[0], caveMaterial);
	gameEntities[31]->transform->SetPosition(2.0f, 0.84f, 13.25f);
	gameEntities[31]->transform->SetScale(0.05f, 3.32f, 1.8f);
	gameEntities[31]->transform->SetRotationY(1.5708);
	gameEntities[31]->SetUVScale(4.8f);

	gameEntities[32] = new GameEntity(meshes[0], caveMaterial);
	gameEntities[32]->transform->SetPosition(-2.0f, 0.84f, 13.25f);
	gameEntities[32]->transform->SetScale(0.05f, 3.32f, 1.8f);
	gameEntities[32]->transform->SetRotationY(1.5708);
	gameEntities[32]->SetUVScale(4.8f);

	gameEntities[33] = new GameEntity(meshes[0], caveMaterial);
	gameEntities[33]->transform->SetPosition(2.9f, 0.84f, 16.75f);
	gameEntities[33]->transform->SetScale(0.02f, 3.32f, 7.0f);
	gameEntities[33]->SetUVScale(4.8f);

	gameEntities[34] = new GameEntity(meshes[0], caveMaterial);
	gameEntities[34]->transform->SetPosition(-2.9f, 0.84f, 16.75f);
	gameEntities[34]->transform->SetScale(0.02f, 3.32f, 7.0f);
	gameEntities[34]->SetUVScale(4.8f);

	gameEntities[35] = new GameEntity(meshes[0], caveMaterial);
	gameEntities[35]->transform->SetPosition(2.0f, 0.84f, 20.25f);
	gameEntities[35]->transform->SetScale(0.05f, 3.32f, 1.8f);
	gameEntities[35]->transform->SetRotationY(1.5708);
	gameEntities[35]->SetUVScale(4.8f);

	gameEntities[36] = new GameEntity(meshes[0], caveMaterial);
	gameEntities[36]->transform->SetPosition(-2.0f, 0.84f, 20.25f);
	gameEntities[36]->transform->SetScale(0.05f, 3.32f, 1.8f);
	gameEntities[36]->transform->SetRotationY(1.5708);
	gameEntities[36]->SetUVScale(4.8f);

	// Add floors for rooms
	gameEntities[37] = new GameEntity(meshes[0], carpetMaterial);
	gameEntities[37]->transform->SetPosition(0.0f, -0.8f, 9.875f);
	gameEntities[37]->transform->SetScale(5.8f, 0.02f, 6.75f);
	gameEntities[37]->SetUVScale(4.2f);

	gameEntities[38] = new GameEntity(meshes[0], dirtMaterial);
	gameEntities[38]->transform->SetPosition(0.0f, -0.8f, 16.75f);
	gameEntities[38]->transform->SetScale(5.8f, 0.02f, 7.0f);
	gameEntities[38]->SetUVScale(4.2f);

	// Add ceilings for rooms
	gameEntities[39] = new GameEntity(meshes[0], carpetMaterial);
	gameEntities[39]->transform->SetPosition(0.0f, 2.5f, 9.875f);
	gameEntities[39]->transform->SetScale(5.8f, 0.02f, 6.75f);
	gameEntities[39]->SetUVScale(4.2f);

	gameEntities[40] = new GameEntity(meshes[0], caveMaterial);
	gameEntities[40]->transform->SetPosition(0.0f, 2.5f, 16.75f);
	gameEntities[40]->transform->SetScale(5.8f, 0.02f, 7.0f);
	gameEntities[40]->SetUVScale(4.2f);

	// Add rocks
	gameEntities[41] = new GameEntity(meshes[5], rockMaterial);
	gameEntities[41]->transform->SetScale(0.005f, 0.005f, 0.005f);
	gameEntities[41]->transform->SetPosition(-0.2f, -0.72f, 15.8f);

	gameEntities[42] = new GameEntity(meshes[5], rockMaterial);
	gameEntities[42]->transform->SetScale(0.0058f, 0.0058f, 0.0058f);
	gameEntities[42]->transform->SetPosition(-2.2f, -0.72f, 17.8f);
	gameEntities[42]->transform->SetRotationY(0.6f);

	gameEntities[43] = new GameEntity(meshes[5], rockMaterial);
	gameEntities[43]->transform->SetScale(0.0037f, 0.0037f, 0.0037f);
	gameEntities[43]->transform->SetPosition(0.823f, -0.72f, 18.0f);
	gameEntities[43]->transform->SetRotationY(1.354f);

	gameEntities[44] = new GameEntity(meshes[5], rockMaterial);
	gameEntities[44]->transform->SetScale(0.0049f, 0.0049f, 0.0049f);
	gameEntities[44]->transform->SetPosition(0.0f, -0.72f, 19.0f);
	gameEntities[44]->transform->SetRotationY(2.4f);


	//create the water
	flatWater = new GameEntity(meshes[7], waterMaterial);
	flatWater->transform->SetPosition(0.0f, -0.7f, -7.5f);
	flatWater->transform->SetScale(100.0f, 100.0f, 100.0f);
	flatWater->transform->SetRotation(0.0f, 0.0f, 0.0f);

}


void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	// Update our projection matrix since the window size changed
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,	// Field of View Angle
		(float)width / height,	// Aspect ratio
		0.1f,				  	// Near clip plane distance
		100.0f);			  	// Far clip plane distance

	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P));

	camera->UpdateProjectionMatrix(width, height);
}

void Game::Update(float deltaTime, float totalTime)
{
	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();

	
	thrusterEmitter->Update(deltaTime, totalTime);
	thrusterEmitter2->Update(deltaTime, totalTime);
	thrusterEmitter3->Update(deltaTime, totalTime);
	campfireEmitter->Update(deltaTime, totalTime);

	//camera->Update(deltaTime);
	player->Update(deltaTime);

	// Update each of the game objects
	for (size_t i = 0; i < 10; i++) 
	{
		XMFLOAT3 zAxis(0, 0, 1);
		XMFLOAT3 yAxis(0, 1, 0);

		gameEntities[i]->transform->Translate(cos(4*totalTime) * 0.05f * deltaTime, sin(4*totalTime) * 0.05f * deltaTime,0);
		/*gameEntities[i]->transform->SetScale( //We should remove functionality of non uniform scaling
			gameEntities[i]->transform->GetScale().x + cos(4 * totalTime) * 0.05f * deltaTime, 
			gameEntities[i]->transform->GetScale().y + cos(4 * totalTime) * 0.05f * deltaTime, 
			gameEntities[i]->transform->GetScale().z + cos(4 * totalTime) * 0.05f * deltaTime);*/
		gameEntities[i]->transform->Rotate(zAxis, cos(2 * totalTime) * 0.5f * deltaTime);
		gameEntities[i]->transform->Rotate(yAxis, 0.25f * deltaTime);
	}

	lights[1].Position = XMFLOAT3(sin(totalTime/4) * 4.0f, 0.5f, 1.0f);
}

void Game::Draw(float deltaTime, float totalTime)
{
	// Background color
	const float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	// Clear the render target and depth buffer
	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearDepthStencilView(
		depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);


	//render all non-refractive elements to a texture
	context->ClearRenderTargetView(nonRefractiveRTV, color);
	context->OMSetRenderTargets(1, &nonRefractiveRTV, depthStencilView);

#pragma region main draw
	// Game Entity Meshes
	for (size_t i = 0; i < 44; i++)
	{
		// If the world matrix is outdated, recalculate it
		if (gameEntities[i]->transform->matrixOutdated)
			gameEntities[i]->transform->CalculateWorldMatrix();

		// Pass the appropriate lights to the material's pixel shader
		Light goodLights[128];
		int lightCount = 0;
		for (size_t i = 0; i < lights.size(); i++)
		{
			if (lights[i].Type == LIGHT_TYPE_DIR || lights[i].Type == LIGHT_TYPE_POINT)
			{
				goodLights[lightCount] = lights[i];
				lightCount++;
			}
		}

		gameEntities[i]->material->GetPixelShader()->SetData("lights", &goodLights, sizeof(Light) * 128);
		gameEntities[i]->material->GetPixelShader()->SetData("lightCount", &lightCount, sizeof(int));

		gameEntities[i]->material->GetPixelShader()->SetData("cameraPos", &camera->transform.GetPosition(), sizeof(DirectX::XMFLOAT3));

		

		// Set buffers in the input assembler
		//  - Do this ONCE PER OBJECT you're drawing, since each object might
		//    have different geometry.
		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		ID3D11Buffer* vertexBuffer = gameEntities[i]->mesh->GetVertexBuffer();
		context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		context->IASetIndexBuffer(gameEntities[i]->mesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

		gameEntities[i]->material->GetPixelShader()->SetSamplerState("basicSampler", gameEntities[i]->material->GetSamplerState());
		gameEntities[i]->material->GetPixelShader()->SetShaderResourceView("diffuseTexture", gameEntities[i]->material->GetDiffuse());
		gameEntities[i]->material->GetPixelShader()->SetShaderResourceView("normalTexture", gameEntities[i]->material->GetNormal());

		gameEntities[i]->material->GetPixelShader()->SetMatrix4x4("view", camera->GetViewMatrix());

		if (gameEntities[i]->material->GetSpecular() != nullptr) //non-pbr
		{
			gameEntities[i]->material->GetPixelShader()->SetShaderResourceView("specularTexture", gameEntities[i]->material->GetSpecular());
		}
		else // pbr
		{
			gameEntities[i]->material->GetPixelShader()->SetShaderResourceView("metallicTexture", gameEntities[i]->material->GetMetalness());
			gameEntities[i]->material->GetPixelShader()->SetShaderResourceView("roughnessTexture", gameEntities[i]->material->GetRoughness());
		}

		gameEntities[i]->PrepareMaterial(camera->GetViewMatrix(), camera->GetProjectionMatrix());

		context->DrawIndexed(gameEntities[i]->mesh->GetIndexCount(), 0, 0);
	}
#pragma endregion

	DrawWater(totalTime);

	// Draw the sky after all other entities have been drawn
	DrawSky();

	// Draw the emitter
	float blend[4] = { 1,1,1,1 };
	context->OMSetBlendState(particleBlendState, blend, 0xffffffff);	// Additive blending
	context->OMSetDepthStencilState(particleDepthStencilState, 0);
	thrusterEmitter->Draw(context, camera, totalTime);
	thrusterEmitter2->Draw(context, camera, totalTime);
	thrusterEmitter3->Draw(context, camera, totalTime);
	campfireEmitter->Draw(context, camera, totalTime);

	// Reset states for drawing the sky
	context->OMSetBlendState(0, blend, 0xffffffff);
	context->OMSetDepthStencilState(0, 0);
	context->RSSetState(0);

	if (postProcessing)
		PostProcessing();

	swapChain->Present(0, 0);

	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);
}

// Method for drawing the sky
// This assumes that the cube mesh is meshes[0], might need to be changed at a later time
void Game::DrawSky()
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	// Set the vertex and index buffers
	ID3D11Buffer* vb = meshes[0]->GetVertexBuffer();
	ID3D11Buffer* ib = meshes[0]->GetIndexBuffer();
	context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

	// Set up the shaders
	skyMaterial->GetVertexShader()->SetMatrix4x4("view", camera->GetViewMatrix());
	skyMaterial->GetVertexShader()->SetMatrix4x4("projection", camera->GetProjectionMatrix());
	skyMaterial->GetVertexShader()->CopyAllBufferData();
	skyMaterial->GetVertexShader()->SetShader();

	skyMaterial->GetPixelShader()->SetSamplerState("basicSampler", skyMaterial->GetSamplerState());
	skyMaterial->GetPixelShader()->SetShaderResourceView("sky", skyMaterial->GetDiffuse());
	skyMaterial->GetPixelShader()->SetShader();

	// Set up the render states
	context->RSSetState(skyRasterizerState);
	context->OMSetDepthStencilState(skyDepthStencilState, 0);

	// Draw the sky
	context->DrawIndexed(meshes[0]->GetIndexCount(), 0, 0);

	// Reset the render states
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);
}

void Game::DrawWater(float totalTime)
{
	const float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	// If the world matrix is outdated, recalculate it
	if (flatWater->transform->matrixOutdated)
		flatWater->transform->CalculateWorldMatrix();


	//first, draw the water's shape into the alpha channel of the refractive mask
	context->ClearRenderTargetView(refractiveMaskRTV, color);
	context->OMSetRenderTargets(1, &refractiveMaskRTV, depthStencilView);

	// Set buffers in the input assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	ID3D11Buffer* vertexBuffer = flatWater->mesh->GetVertexBuffer();
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(flatWater->mesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

	flatWater->PrepareMaterial(camera->GetViewMatrix(), camera->GetProjectionMatrix());
	refractiveMaskPS->CopyAllBufferData();
	refractiveMaskPS->SetShader();

	context->DrawIndexed(flatWater->mesh->GetIndexCount(), 0, 0);

	//next, draw water into a refractive texture

	context->ClearRenderTargetView(refractiveRTV, color);
	context->OMSetRenderTargets(1, &refractiveRTV, NULL);

	// Pass the appropriate lights to the material's pixel shader
	Light goodLights[128];
	int lightCount = 0;
	for (size_t i = 0; i < lights.size(); i++)
	{
		if (lights[i].Type == LIGHT_TYPE_DIR || lights[i].Type == LIGHT_TYPE_POINT)
		{
			goodLights[lightCount] = lights[i];
			lightCount++;
		}
	}

	flatWater->material->GetPixelShader()->SetData("lights", &goodLights, sizeof(Light) * 128);
	flatWater->material->GetPixelShader()->SetData("lightCount", &lightCount, sizeof(int));
	flatWater->material->GetPixelShader()->SetData("scale", &flatWater->transform->GetScale(), sizeof(XMFLOAT3));
	flatWater->material->GetPixelShader()->SetData("totalTime", &totalTime, sizeof(float));
	flatWater->material->GetPixelShader()->SetData("cameraPos", &camera->transform.GetPosition(), sizeof(DirectX::XMFLOAT3));
	flatWater->material->GetPixelShader()->SetData("width", &width, sizeof(int));
	flatWater->material->GetPixelShader()->SetData("height", &height, sizeof(int));

	flatWater->material->GetPixelShader()->SetMatrix4x4("view", camera->GetViewMatrix());
	flatWater->material->GetPixelShader()->SetMatrix4x4("projection", camera->GetProjectionMatrix());

	flatWater->material->GetPixelShader()->SetSamplerState("basicSampler", flatWater->material->GetSamplerState());
	flatWater->material->GetPixelShader()->SetSamplerState("clampedSampler", clampedSampler);
	flatWater->material->GetPixelShader()->SetShaderResourceView("normalTexture", flatWater->material->GetNormal());
	flatWater->material->GetPixelShader()->SetShaderResourceView("sceneSansWater", nonRefractiveSRV);
	flatWater->material->GetPixelShader()->SetShaderResourceView("mask", refractiveMaskSRV);

	flatWater->PrepareMaterial(camera->GetViewMatrix(), camera->GetProjectionMatrix());

	context->DrawIndexed(flatWater->mesh->GetIndexCount(), 0, 0);

	//combine the refractive and non-refractive textures

	if (postProcessing)
	{
		//Clear the post process texture
		context->ClearRenderTargetView(postProcessRTV, color);
		context->ClearRenderTargetView(bloom1RTV, color);
		context->ClearRenderTargetView(bloom2RTV, color);
		context->ClearRenderTargetView(motionBlurRTV, color);

		// Set the post process RTV as the current render target
		context->OMSetRenderTargets(1, &postProcessRTV, depthStencilView);
	}
	else
	{
		context->OMSetRenderTargets(1, &backBufferRTV, 0);
	}

	// Deactivate vertex and index buffers
	ID3D11Buffer* nothing = 0;
	context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
	context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	// Render a full-screen triangle using the post process shaders
	postProcessVS->SetShader();

	//Pixel shader that combines non-refractive and refractive elements
	combineRefractionPS->SetShaderResourceView("nonRefractive", nonRefractiveSRV);
	combineRefractionPS->SetShaderResourceView("refractive", refractiveSRV);
	combineRefractionPS->SetShaderResourceView("mask", refractiveMaskSRV);
	combineRefractionPS->SetSamplerState("Sampler", sampler);
	combineRefractionPS->CopyAllBufferData();
	combineRefractionPS->SetShader();

	float blend[4] = { 1,1,1,1 };
	context->OMSetBlendState(particleBlendState, blend, 0xffffffff);	// Additive blending
	context->OMSetDepthStencilState(particleDepthStencilState, 0);
	context->Draw(3, 0);

	// Reset states for drawing the sky
	context->OMSetBlendState(0, blend, 0xffffffff);
	context->OMSetDepthStencilState(0, 0);
	context->RSSetState(0);
}


//General Post Processing method
//Currently only implements bloom
//TODO: Motion Blur, 
void Game::PostProcessing()
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	const float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	ID3D11ShaderResourceView* nullSRVs[16] = {};

	// Deactivate vertex and index buffers
	ID3D11Buffer* nothing = 0;
	context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
	context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	// Render a full-screen triangle using the post process shaders
	postProcessVS->SetShader();
	
#pragma region Bloom

	//Set render target to first bloom texture -> Render texture with just the bright bits
	context->OMSetRenderTargets(1, &bloom1RTV, depthStencilView);
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//Pixel shader that renders just the bright parts
	brightExtractPS->SetFloat("Threshold", 0.8f);
	brightExtractPS->SetShaderResourceView("Pixels", postProcessSRV);
	brightExtractPS->SetSamplerState("Sampler", sampler);
	brightExtractPS->CopyAllBufferData();
	brightExtractPS->SetShader();

	context->Draw(3, 0);

	// Set render target to second bloom texture->Render texture with horizontal blur
	context->OMSetRenderTargets(1, &bloom2RTV, depthStencilView);
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//Bloom first pass - Horizontal guass blur
	bloomBlurHPS->SetShaderResourceView("BrightPixels", bloom1SRV);   //Brightness-extracted frame texture
	bloomBlurHPS->SetSamplerState("Sampler", sampler);
	bloomBlurHPS->SetFloat("pixelWidth", 1.f / width);
	bloomBlurHPS->CopyAllBufferData();
	bloomBlurHPS->SetShader();

	context->Draw(3, 0);

	// Set render target to motion blur texture->Render texture with full bloom blur
	context->OMSetRenderTargets(1, &motionBlurRTV, depthStencilView);
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//Bloom second pass - vertical guass blur
	bloomBlurVPS->SetShaderResourceView("Pixels", postProcessSRV);	     //Original rendered frame
	bloomBlurVPS->SetShaderResourceView("ExtractedPixels", bloom2SRV);   //Horizontal blurred bloom texture
	bloomBlurVPS->SetSamplerState("Sampler", sampler);
	bloomBlurHPS->SetFloat("pixelHeight", 1.f / height);
	bloomBlurVPS->CopyAllBufferData();
	bloomBlurVPS->SetShader();

	context->Draw(3, 0);


#pragma endregion

#pragma region Motion Blur
	//Set render target to back buffer-> Render texture with blur combined with original frame
	context->OMSetRenderTargets(1, &backBufferRTV, 0);
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	motionBlurPS->SetShaderResourceView("Pixels", motionBlurSRV);	//Bloom blurred texture
	motionBlurPS->SetSamplerState("Sampler", sampler);
	motionBlurPS->SetFloat("pixelWidth", 1.f / width);
	motionBlurPS->SetFloat("pixelHeight", 1.f / height);
	motionBlurPS->CopyAllBufferData();
	motionBlurPS->SetShader();


	context->Draw(3, 0);

	motionBlurPS->SetFloat("blurV", 0);
	motionBlurPS->SetFloat("blurH", 0);
#pragma endregion

	// Unbind all pixel shader SRVs
	context->PSSetShaderResources(0, 16, nullSRVs);
	
}


#pragma region Mouse Input

void Game::OnMouseDown(WPARAM buttonState, int x, int y)
{
	// If Left Down
	if (buttonState & 0x0001)
	{
		rotating = true;
	}

	// Save the previous mouse position
	prevMousePos.x = x;
	prevMousePos.y = y;

	// Capture the Mouse
	SetCapture(hWnd);
}

void Game::OnMouseUp(WPARAM buttonState, int x, int y)
{
	// If Left Up
	if (!buttonState & 0x0001)
	{
		rotating = false;
	}

	ReleaseCapture();
}

void Game::OnMouseMove(WPARAM buttonState, int x, int y)
{
	if (rotating)
	{

		float pixelToRads = 0.003f;

		float xAngle = ((float)y - (float)prevMousePos.y) * pixelToRads;
		float yAngle = ((float)x - (float)prevMousePos.x) * pixelToRads;


		motionBlurPS->SetFloat("blurV", xAngle);
		motionBlurPS->SetFloat("blurH", yAngle);


		camera->RotateCamera(xAngle, yAngle);
	}


	// Save Mouse Position
	prevMousePos.x = x;
	prevMousePos.y = y;
}

void Game::OnMouseWheel(float wheelDelta, int x, int y)
{
	// Add any custom code here...
}
#pragma endregion