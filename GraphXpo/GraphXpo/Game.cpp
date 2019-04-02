#include "Game.h"
#include "Vertex.h"

#include "WICTextureLoader.h"

using namespace DirectX;

Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,		// The application's handle
		"DirectX Game",	   	// Text for the window's title bar
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

#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif
	
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
	//I've opted to wrap meshes, shaders, and materials in shared_ptrs, so they don't require manual deallocation

	//delete all of the game objects
	for (size_t i = 0; i < 10; i++)
	{
		delete gameEntities[i];
	}
}

void Game::Init()
{
	camera = Camera();
	rotating = false;
	printf("WASD to move. Space/X for vertical movement. Click and drag to rotate.");

	LoadShaders();
	CreateMatrices();
	CreateBasicGeometry();

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//set up the directional lights
	dl1.AmbientColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	dl1.DiffuseColor = XMFLOAT4(0.5f, 0.5f, 0.5f, 1);
	dl1.Direction = XMFLOAT3(1, -1, 0);

	dl2.AmbientColor = XMFLOAT4(0.05f, 0.0f, 0.0f, 1.0f);
	dl2.DiffuseColor = XMFLOAT4(0.6f, 0.4f, 0.6f, 1.0f);
	dl2.Direction = XMFLOAT3(-0.5f, -0.75f, 0.0f);
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files using
// my SimpleShader wrapper for DirectX shader manipulation.
// - SimpleShader provides helpful methods for sending
//   data to individual variables on the GPU
// --------------------------------------------------------
void Game::LoadShaders()
{
	vertexShader = std::make_shared<SimpleVertexShader>(device, context);
	vertexShader->LoadShaderFile(L"FoldingVert.cso");

	pixelShader = std::make_shared<SimplePixelShader>(device, context);
	pixelShader->LoadShaderFile(L"PixelShader.cso");

	ID3D11ShaderResourceView* barkSRV;
	CreateWICTextureFromFile(device, context, L"..\\Assets\\Textures\\bark.jpg", 0, &barkSRV);
	ID3D11ShaderResourceView* bark_s_SRV;
	CreateWICTextureFromFile(device, context, L"..\\Assets\\Textures\\bark_s.jpg", 0, &bark_s_SRV);
	ID3D11ShaderResourceView* bark_n_SRV;
	CreateWICTextureFromFile(device, context, L"..\\Assets\\Textures\\bark_n.jpg", 0, &bark_n_SRV);

	ID3D11ShaderResourceView* carpetSRV;
	CreateWICTextureFromFile(device, context, L"..\\Assets\\Textures\\carpet.jpg", 0, &carpetSRV);
	ID3D11ShaderResourceView* carpet_s_SRV;
	CreateWICTextureFromFile(device, context, L"..\\Assets\\Textures\\carpet_s.jpg", 0, &carpet_s_SRV);
	ID3D11ShaderResourceView* carpet_n_SRV;
	CreateWICTextureFromFile(device, context, L"..\\Assets\\Textures\\carpet_n.jpg", 0, &carpet_n_SRV);

	ID3D11SamplerState* sampler;
	D3D11_SAMPLER_DESC samplerDesc = {};

	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, &sampler);

	barkMaterial = std::make_shared<Material>(vertexShader, pixelShader, barkSRV, bark_s_SRV, bark_n_SRV, sampler);
	carpetMaterial = std::make_shared<Material>(vertexShader, pixelShader, carpetSRV, carpet_s_SRV, carpet_n_SRV, sampler);
}



// --------------------------------------------------------
// Initializes the matrices necessary to represent our geometry's 
// transformations and our 3D camera
// --------------------------------------------------------
void Game::CreateMatrices()
{
	// Set up world matrix
	// - In an actual game, each object will need one of these and they should
	//    update when/if the object moves (every frame)
	// - You'll notice a "transpose" happening below, which is redundant for
	//    an identity matrix.  This is just to show that HLSL expects a different
	//    matrix (column major vs row major) than the DirectX Math library
	XMMATRIX W = XMMatrixIdentity();
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(W)); // Transpose for HLSL!

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
	XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(V)); // Transpose for HLSL!

	// Create the Projection matrix
	// - This should match the window's aspect ratio, and also update anytime
	//    the window resizes (which is already happening in OnResize() below)
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,		// Field of View Angle
		(float)width / height,		// Aspect ratio
		0.1f,						// Near clip plane distance
		100.0f);					// Far clip plane distance
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P)); // Transpose for HLSL!

	camera.UpdateProjectionMatrix(width, height);
}


// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateBasicGeometry()
{
	meshes[0] = std::make_shared<Mesh>((char *)"..\\Assets\\Models\\helix.obj", device);
	meshes[1] = std::make_shared<Mesh>((char *)"..\\Assets\\Models\\cone.obj", device);
	meshes[2] = std::make_shared<Mesh>((char *)"..\\Assets\\Models\\torus.obj", device);

	//now create the new game objects and assign the meshes
	for (int i = 0; i < 10; i++)
	{
		if(i % 2 == 0)
			gameEntities[i] = new GameEntity(meshes[i%3], barkMaterial);
		else
			gameEntities[i] = new GameEntity(meshes[i % 3], carpetMaterial);

		gameEntities[i]->transform->SetPosition((-7.0f + (float)i), 0, 0);
		gameEntities[i]->transform->SetScale(0.5f, 0.5f, 0.5f);
	}
}


// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	// Update our projection matrix since the window size changed
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,	// Field of View Angle
		(float)width / height,	// Aspect ratio
		0.001f,				  	// Near clip plane distance
		100.0f);			  	// Far clip plane distance
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P)); // Transpose for HLSL!

	camera.UpdateProjectionMatrix(width, height);
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();

	camera.Update(deltaTime);

	for (size_t i = 0; i < 10; i++) //update each of the game objects
	{
		XMFLOAT3 rotAxis(0, 0, 1);

		//gameEntities[i]->transform->Translate(cos(4*totalTime) * 0.05f * deltaTime, sin(4*totalTime) * 0.05f * deltaTime,0);
		//gameEntities[i]->transform->SetScale(gameEntities[i]->transform->GetScale().x + cos(4 * totalTime) * 0.05f * deltaTime, gameEntities[i]->transform->GetScale().y + cos(4 * totalTime) * 0.05f * deltaTime, gameEntities[i]->transform->GetScale().z + cos(4 * totalTime) * 0.05f * deltaTime);
		//gameEntities[i]->transform->Rotate(rotAxis, cos(4 * totalTime) * 0.5f * deltaTime);
	}
}

float TEMP_EVOLUTION_TIMER = -1.57f;

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Background color (Cornflower Blue in this case) for clearing
	const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };

	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearDepthStencilView(
		depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	

	for (size_t i = 0; i < 10; i++)//draw each of the 10 gameEntities meshes
	{
		//if the world matrix is outdated, recalculate it
		if (gameEntities[i]->transform->matrixOutdated)
			gameEntities[i]->transform->CalculateWorldMatrix();

		//pass the directional lights to the material's pixel shader
		gameEntities[i]->material->GetPixelShader()->SetData("dl1", &dl1, sizeof(DirectionalLight));
		gameEntities[i]->material->GetPixelShader()->SetData("dl2", &dl2, sizeof(DirectionalLight));

		gameEntities[i]->material->GetPixelShader()->SetData("cameraPos", &camera.transform.GetPosition(), sizeof(DirectX::XMFLOAT3));

		gameEntities[i]->PrepareMaterial(camera.GetViewMatrix(), camera.GetProjectionMatrix());

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

		gameEntities[i]->material->GetVertexShader()->SetFloat("evolution", (sin(TEMP_EVOLUTION_TIMER += .01f * deltaTime) + 1.0f) / 2.0f);

		// Finally do the actual drawing
		//  - Do this ONCE PER OBJECT you intend to draw
		//  - This will use all of the currently set DirectX "stuff" (shaders, buffers, etc)
		//  - DrawIndexed() uses the currently set INDEX BUFFER to look up corresponding
		//     vertices in the currently set VERTEX BUFFER
		context->DrawIndexed(
			gameEntities[i]->mesh->GetIndexCount(),     // The number of indices to use (we could draw a subset if we wanted)
			0,     // Offset to the first index we want to use
			0);
	}

	// Present the back buffer to the user
	//  - Puts the final frame we're drawing into the window so the user can see it
	//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
	swapChain->Present(0, 0);
}


#pragma region Mouse Input

// --------------------------------------------------------
// Helper method for mouse clicking.  We get this information
// from the OS-level messages anyway, so these helpers have
// been created to provide basic mouse input if you want it.
// --------------------------------------------------------
void Game::OnMouseDown(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	if (buttonState & 0x0001) //if left click
	{
		rotating = true;
	}

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;

	// Caputure the mouse so we keep getting mouse move
	// events even if the mouse leaves the window.  we'll be
	// releasing the capture once a mouse button is released
	SetCapture(hWnd);
}

// --------------------------------------------------------
// Helper method for mouse release
// --------------------------------------------------------
void Game::OnMouseUp(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	if (!buttonState & 0x0001) //if left click released
	{
		rotating = false;
	}

	// We don't care about the tracking the cursor outside
	// the window anymore (we're not dragging if the mouse is up)
	ReleaseCapture();
}

// --------------------------------------------------------
// Helper method for mouse movement.  We only get this message
// if the mouse is currently over the window, or if we're 
// currently capturing the mouse.
// --------------------------------------------------------
void Game::OnMouseMove(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	if (rotating) //in this case, if the left mouse button is being held down
	{
		float pixelToRads = 0.003f;

		//rotate the camera according to the change in the mouse position
		//movement on the x-axis should rotate about the y-axis, and vice versa
		float xAngle = ((float)y - (float)prevMousePos.y) * pixelToRads;
		float yAngle = ((float)x - (float)prevMousePos.x) * pixelToRads;

		//now rotate 
		camera.RotateCamera(xAngle, yAngle);
	}


	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;
}

// --------------------------------------------------------
// Helper method for mouse wheel scrolling.  
// WheelDelta may be positive or negative, depending 
// on the direction of the scroll
// --------------------------------------------------------
void Game::OnMouseWheel(float wheelDelta, int x, int y)
{
	// Add any custom code here...
}
#pragma endregion