#include "Game.h"
#include "Vertex.h"
#include "WICTextureLoader.h"

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
	for (size_t i = 0; i < 20; i++)
	{
		delete gameEntities[i];
	}

	delete player;
	delete camera;
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

	LoadShaders();
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
	dl2.Color = XMFLOAT3(0.8f, 0.1f, 0.3f);
	dl2.Position = XMFLOAT3(-1.0f, 1.0f, -2.0f);
	dl2.Range = 5.0f;
	dl2.Intensity = 3.0f;

	lights.emplace_back(dl1);
	lights.emplace_back(dl2);
}

void Game::LoadShaders()
{
	vertexShader = std::make_shared<SimpleVertexShader>(device, context);
	vertexShader->LoadShaderFile(L"VertexShader.cso");

	pixelShader = std::make_shared<SimplePixelShader>(device, context);
	pixelShader->LoadShaderFile(L"PixelShader.cso");

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

	ID3D11SamplerState* sampler;
	D3D11_SAMPLER_DESC samplerDesc = {};

	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, &sampler);

	barkMaterial = std::make_shared<Material>(vertexShader, pixelShader, rockSRV, rock_s_SRV, rock_n_SRV, sampler);
	carpetMaterial = std::make_shared<Material>(vertexShader, pixelShader, brickSRV, brick_s_SRV, brick_n_SRV, sampler);
	ceilingMaterial = std::make_shared<Material>(vertexShader, pixelShader, ceilingSRV, ceiling_s_SRV, ceiling_n_SRV, sampler);
	marbleMaterial = std::make_shared<Material>(vertexShader, pixelShader, marbleSRV, marble_s_SRV, marble_n_SRV, sampler);
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

	// Create basic test geometry
	for (int i = 0; i < 10; i++)
	{
		if(i % 2 == 0)
			gameEntities[i] = new GameEntity(meshes[i%3], barkMaterial);
		else
			gameEntities[i] = new GameEntity(meshes[i % 3], carpetMaterial);

		gameEntities[i]->transform->SetPosition((-4.0f + (float)i), 0, 1.2f);
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

	//camera->Update(deltaTime);
	player->Update(deltaTime);

	// Update each of the game objects
	for (size_t i = 0; i < 10; i++) 
	{
		XMFLOAT3 zAxis(0, 0, 1);
		XMFLOAT3 yAxis(0, 1, 0);

		gameEntities[i]->transform->Translate(cos(4*totalTime) * 0.05f * deltaTime, sin(4*totalTime) * 0.05f * deltaTime,0);
		gameEntities[i]->transform->SetScale( //We should remove functionality of non uniform scaling
			gameEntities[i]->transform->GetScale().x + cos(4 * totalTime) * 0.05f * deltaTime, 
			gameEntities[i]->transform->GetScale().y + cos(4 * totalTime) * 0.05f * deltaTime, 
			gameEntities[i]->transform->GetScale().z + cos(4 * totalTime) * 0.05f * deltaTime);
		gameEntities[i]->transform->Rotate(zAxis, cos(2 * totalTime) * 0.5f * deltaTime);
		gameEntities[i]->transform->Rotate(yAxis, 0.25f * deltaTime);
	}
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

	
	// Game Entitie Meshes
	for (size_t i = 0; i < 20; i++)
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
		gameEntities[i]->material->GetPixelShader()->SetShaderResourceView("specularTexture", gameEntities[i]->material->GetSpecular());
		gameEntities[i]->material->GetPixelShader()->SetShaderResourceView("normalTexture", gameEntities[i]->material->GetNormal());

		context->DrawIndexed(gameEntities[i]->mesh->GetIndexCount(), 0, 0);
	}

	swapChain->Present(0, 0);
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