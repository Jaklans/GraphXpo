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
	for (size_t i = 0; i < 23; i++)
	{
		delete gameEntities[i];
	}

	sampler->Release();

	//Release Post-Process resources
	bloomRTV->Release();
	bloomSRV->Release();


	delete player;
	delete camera;

	// Release particle resources
	particleTexture->Release();
	particleBlendState->Release();
	particleDepthStencilState->Release();
	delete thrusterEmitter;
	delete thrusterEmitter2;
	delete thrusterEmitter3;

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
	dl1.Direction = XMFLOAT3(0.25f, -0.15f, 0.5f);
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

	lights.emplace_back(dl1);
	lights.emplace_back(dl2);
	lights.emplace_back(dl3);

	// Set up the Emitters
	thrusterEmitter = new Emitter(
		80,									// Max particles
		50,										// Particles per second
		1.6f,									// Particle lifetime
		0.33f,									// Start size
		0.0f,									// End size
		XMFLOAT4(0.788f, 0.094f, 0.094f, 0.0f),	// Start color
		XMFLOAT4(0.972f, 0.823f, 0.686f, 1.0f),	// End color
		XMFLOAT3(-4.44f, 0.115f, -1.68f),		// Emitter position
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
		XMFLOAT3(-4.12f, 0.0f, -1.48f),			// Emitter position
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
		XMFLOAT3(-4.68f, 0.0f, -1.98f),			// Emitter position
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

	bloomBlurPS = std::make_shared<SimplePixelShader>(device, context);
	bloomBlurPS->LoadShaderFile(L"BloomBlurPS.cso");

	// Load Particle shaders
	particleVertexShader = std::make_shared<SimpleVertexShader>(device, context);
	particleVertexShader->LoadShaderFile(L"ParticleVertexShader.cso");

	particlePixelShader = std::make_shared<SimplePixelShader>(device, context);
	particlePixelShader->LoadShaderFile(L"ParticlePixelShader.cso");

	shadowVS = std::make_shared<SimpleVertexShader>(device, context);
	shadowVS->LoadShaderFile(L"ShadowVS");
#pragma endregion

#pragma region general texture loading
	ID3D11ShaderResourceView* rockSRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\rock.jpg", 0, &rockSRV);
	ID3D11ShaderResourceView* rock_s_SRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\rock_s.jpg", 0, &rock_s_SRV);
	ID3D11ShaderResourceView* rock_n_SRV;
	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\rock_n.jpg", 0, &rock_n_SRV);

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


	CreateWICTextureFromFile(device, context, L"..\\..\\Assets\\Textures\\fireParticle.jpg", 0, &particleTexture);

#pragma endregion

#pragma region skybox loading
	ID3D11ShaderResourceView* skySRV;
	CreateDDSTextureFromFile(device, L"..\\..\\Assets\\Textures\\NightSkyCubemap.dds", 0, &skySRV);

	barkMaterial = std::make_shared<Material>(vertexShader, pixelShader, rockSRV, rock_s_SRV, rock_n_SRV, sampler);
	carpetMaterial = std::make_shared<Material>(vertexShader, pbrPixelShader, metalSRV, metal_m_SRV, metal_r_SRV, metal_n_SRV, sampler);
	ceilingMaterial = std::make_shared<Material>(vertexShader, pixelShader, ceilingSRV, ceiling_s_SRV, ceiling_n_SRV, sampler);
	marbleMaterial = std::make_shared<Material>(vertexShader, pixelShader, marbleSRV, marble_s_SRV, marble_n_SRV, sampler);
	marbleWallMaterial = std::make_shared<Material>(vertexShader, pixelShader, marbleWallSRV, marbleWall_s_SRV, marbleWall_n_SRV, sampler);
	spaceshipMaterial = std::make_shared<Material>(vertexShader, pbrPixelShader, spaceshipSRV, spaceship_m_SRV, spaceship_m_SRV, spaceship_n_SRV, sampler);
	skyMaterial = std::make_shared<Material>(skyVertexShader, skyPixelShader, skySRV, nullptr, nullptr, sampler);

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
	ID3D11Texture2D* bloomTexture;
	device->CreateTexture2D(&textureDesc, 0, &bloomTexture);
	device->CreateRenderTargetView(bloomTexture, &rtvDesc, &bloomRTV);
	device->CreateShaderResourceView(bloomTexture, &srvDesc, &bloomSRV);



	// We don't need the texture references 
	ppTexture->Release();
	bloomTexture->Release();


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

#pragma region shadow loading
	// Create Shadow Map Textures
	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = shadowMapSize;
	shadowDesc.Height = shadowMapSize;
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;

	ID3D11Texture2D* shadowTexture;
	device->CreateTexture2D(&shadowDesc, 0, &shadowTexture);

	// Create the depth/stencil
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDepthStencilDesc = {};
	shadowDepthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDepthStencilDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDepthStencilDesc.Texture2D.MipSlice = 0;

	device->CreateDepthStencilView(shadowTexture, &shadowDepthStencilDesc, &shadowDSV);

	// Create the SRV for the shadow map
	D3D11_SHADER_RESOURCE_VIEW_DESC shadowsrvDesc = {};
	shadowsrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shadowsrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shadowsrvDesc.Texture2D.MipLevels = 1;
	shadowsrvDesc.Texture2D.MostDetailedMip = 0;
	//srvDesc.Texture2DArray.ArraySize = x;
	device->CreateShaderResourceView(shadowTexture, &shadowsrvDesc, &shadowSRV);

	// Release the texture reference since we don't need it
	shadowTexture->Release();

	// Comparison Sampler for shadows
	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR; // Could be anisotropic
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f;
	shadowSampDesc.BorderColor[1] = 1.0f;
	shadowSampDesc.BorderColor[2] = 1.0f;
	shadowSampDesc.BorderColor[3] = 1.0f;
	device->CreateSamplerState(&shadowSampDesc, &shadowSampler);

	// Create a rasterizer state
	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000; // Multiplied by (smallest possible value > 0 in depth buffer)
	shadowRastDesc.DepthBiasClamp = 0.0f;
	shadowRastDesc.SlopeScaledDepthBias = 1.0f;
	device->CreateRasterizerState(&shadowRastDesc, &shadowRasterizer);
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

	// Create basic test geometry
	for (int i = 0; i < 10; i++)
	{
		if (i % 2 == 0)
			gameEntities[i] = new GameEntity(meshes[i % 3], barkMaterial);
		else
			gameEntities[i] = new GameEntity(meshes[i % 3], carpetMaterial);

		gameEntities[i]->transform->SetPosition((-4.0f + (float)i), 0, 1.2f);
		gameEntities[i]->transform->SetScale(0.5f, 0.5f, 0.5f);
	}

	// Create arches
	for (int i = 0; i < 8; i++) {
		gameEntities[i + 10] = new GameEntity(meshes[3], marbleMaterial);
		gameEntities[i + 10]->transform->SetPosition(-5.7f + 3.8f * (i % 4), -0.82f, ((i / 4) * 12.4f) - 6.2f);
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
	gameEntities[22]->transform->SetPosition(-3.6f, 0.44f, -2.5f);
	gameEntities[22]->transform->SetScale(0.0023f, 0.0023f, 0.0023f);
	gameEntities[22]->transform->SetRotation(0.22f, -0.8f, 0.0f);
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

	//camera->Update(deltaTime);
	player->Update(deltaTime);

	// Update each of the game objects
	for (size_t i = 0; i < 10; i++) 
	{
		XMFLOAT3 zAxis(0, 0, 1);
		XMFLOAT3 yAxis(0, 1, 0);

		gameEntities[i]->transform->Translate(cos(4 * totalTime) * 0.05f * deltaTime, sin(4 * totalTime) * 0.05f * deltaTime, 0);
		gameEntities[i]->transform->SetScale( //We should remove functionality of non uniform scaling
			gameEntities[i]->transform->GetScale().x + cos(4 * totalTime) * 0.05f * deltaTime,
			gameEntities[i]->transform->GetScale().y + cos(4 * totalTime) * 0.05f * deltaTime,
			gameEntities[i]->transform->GetScale().z + cos(4 * totalTime) * 0.05f * deltaTime);
		gameEntities[i]->transform->Rotate(zAxis, cos(2 * totalTime) * 0.5f * deltaTime);
		gameEntities[i]->transform->Rotate(yAxis, 0.25f * deltaTime);
	}

	lights[1].Position = XMFLOAT3(sin(totalTime/4) * 4.0f, 0.5f, 1.0f);
}

void Game::Draw(float deltaTime, float totalTime)
{
	// Background color
	const float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	// Clear the render target and depth buffer
	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearDepthStencilView(
		depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	DrawShadowMaps();

	//POST PROCESS PRE-RENDER 
	if (postProcessing)
	{
		//Clear the post process texture
		context->ClearRenderTargetView(postProcessRTV, color);
		// Set the post process RTV as the current render target
		context->OMSetRenderTargets(1, &postProcessRTV, depthStencilView);
	}
	
#pragma region main draw
	// Game Entity Meshes
	for (size_t i = 0; i < 23; i++)
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
					   
		gameEntities[i]->PrepareMaterial(camera->GetViewMatrix(), camera->GetProjectionMatrix());

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

		if (gameEntities[i]->material->GetSpecular() == nullptr) //non-pbr
		{
			gameEntities[i]->material->GetPixelShader()->SetShaderResourceView("specularTexture", gameEntities[i]->material->GetSpecular());
		}
		else // pbr
		{
			gameEntities[i]->material->GetPixelShader()->SetShaderResourceView("metallicTexture", gameEntities[i]->material->GetMetalness());
			gameEntities[i]->material->GetPixelShader()->SetShaderResourceView("roughnessTexture", gameEntities[i]->material->GetRoughness());
		}

		context->DrawIndexed(gameEntities[i]->mesh->GetIndexCount(), 0, 0);
	}
#pragma endregion

	// Draw the sky after all other entities have been drawn
	DrawSky();

	// Draw the emitter
	float blend[4] = { 1,1,1,1 };
	context->OMSetBlendState(particleBlendState, blend, 0xffffffff);	// Additive blending
	context->OMSetDepthStencilState(particleDepthStencilState, 0);
	thrusterEmitter->Draw(context, camera, totalTime);
	thrusterEmitter2->Draw(context, camera, totalTime);
	thrusterEmitter3->Draw(context, camera, totalTime);

	// Reset states for drawing the sky
	context->OMSetBlendState(0, blend, 0xffffffff);
	context->OMSetDepthStencilState(0, 0);
	context->RSSetState(0);

	if (postProcessing)
		PostProcessing();

	swapChain->Present(0, 0);

	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);
}

//Draws a shadow map for every light
void Game::DrawShadowMaps() {

	//Set Viewport for shadows
	D3D11_VIEWPORT vp = {};
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = (float)shadowMapSize;
	vp.Height = (float)shadowMapSize;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	context->RSSetViewports(1, &vp);

	//Set and clear the depth stencil view for shadows
	context->OMSetRenderTargets(0, 0, shadowDSV);
	context->ClearDepthStencilView(shadowDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
	context->RSSetState(shadowRasterizer);

	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	XMFLOAT4X4 view;
	XMFLOAT4X4 projection;

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	shadowVS->SetShader();

	//for each (Light l in lights) {
	Light l = lights[0];
	{
		XMStoreFloat4x4(&view, XMMatrixTranspose(
			XMMatrixLookToLH(
				XMLoadFloat3(&l.Position),
				XMLoadFloat3(&l.Direction),
				XMVectorSet(0, 1, 0, 0)
			)
		));
		XMStoreFloat4x4(&projection, XMMatrixTranspose(
			XMMatrixOrthographicLH(
				10,
				10,
				.1f,
				50)
		));

		for each (GameEntity* g in gameEntities) {
			vertexBuffer = g->mesh->GetVertexBuffer();
			indexBuffer = g->mesh->GetIndexBuffer();

			context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
			context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

			shadowVS->SetMatrix4x4("world", g->transform->GetWorldMatrix());
			shadowVS->SetMatrix4x4("view", view);
			shadowVS->SetMatrix4x4("projection", projection);
			shadowVS->CopyAllBufferData();

			context->DrawIndexed(g->mesh->GetIndexCount(), 0, 0);
		}
	}

	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);
	vp.Width = (float)width;
	vp.Height = (float)height;
	context->RSSetViewports(1, &vp);
	context->RSSetState(0);
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
//General Post Processing method
//Currently only implements bloom
//TODO: Motion Blur, 
void Game::PostProcessing()
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	const float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	// Deactivate vertex and index buffers
	ID3D11Buffer* nothing = 0;
	context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
	context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	// Render a full-screen triangle using the post process shaders
	postProcessVS->SetShader();

#pragma region Bloom
	//Set render target to first bloom texture -> Render texture with just the bright bits
	context->OMSetRenderTargets(1, &bloomRTV, depthStencilView);

	

	//Pixel shader that renders just the bright parts
	brightExtractPS->SetFloat("Threshold", 0.65f);
	brightExtractPS->SetShaderResourceView("Pixels", postProcessSRV);
	brightExtractPS->SetSamplerState("Sampler", sampler);
	brightExtractPS->CopyAllBufferData();
	brightExtractPS->SetShader();

	context->Draw(3, 0);

	
	//Set render target to back buffer-> Render texture with blur combined with original frame
	context->OMSetRenderTargets(1, &backBufferRTV, 0);
	
	bloomBlurPS->SetShaderResourceView("Pixels", postProcessSRV);   //Unchanged frame texture
	bloomBlurPS->SetShaderResourceView("ExtractedPixels", bloomSRV);   //Brightness-extracted frame texture
	bloomBlurPS->SetSamplerState("Sampler", sampler);
	bloomBlurPS->SetFloat("pixelWidth", 1.f / width);
	bloomBlurPS->SetFloat("pixelHeight", 1.f / height);
	bloomBlurPS->SetInt("blurAmount", 4);
	bloomBlurPS->CopyAllBufferData();
	bloomBlurPS->SetShader();

	context->Draw(3, 0);
#pragma endregion

	// Unbind all pixel shader SRVs
	ID3D11ShaderResourceView* nullSRVs[16] = {};
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