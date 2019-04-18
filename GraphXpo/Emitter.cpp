#include "Emitter.h"


Emitter::Emitter(int maxParticles, int particlesPerSecond, float lifetime, float startSize, float endSize, DirectX::XMFLOAT4 startColor, DirectX::XMFLOAT4 endColor, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 acceleration, DirectX::XMFLOAT3 startVelocity, DirectX::XMFLOAT3 velocityRandomRange, DirectX::XMFLOAT3 positionRandomRange, DirectX::XMFLOAT4 rotationRandomRanges, ID3D11Device * device, ID3D11ShaderResourceView * texture, std::shared_ptr<SimpleVertexShader> vertexShader, std::shared_ptr<SimplePixelShader> pixelShader)
{
	// Store variables
	this->maxParticles = maxParticles;
	this->particlesPerSecond = particlesPerSecond;
	this->secondsPerParticle = 1.0f / particlesPerSecond;
	this->lifetime = lifetime;
	this->startSize = startSize;
	this->endSize = endSize;
	this->startColor = startColor;
	this->endColor = endColor;
	this->position = position;
	this->acceleration = acceleration;
	this->startVelocity = startVelocity;
	this->velocityRandomRange = velocityRandomRange;
	this->positionRandomRange = positionRandomRange;
	this->rotationRandomRanges = rotationRandomRanges;
	this->texture = texture;
	this->vertexShader = vertexShader;
	this->pixelShader = pixelShader;

	timeSinceLastEmit = 0;
	liveParticleCount = 0;
	firstLiveIndex = 0;
	firstDeadIndex = 0;

	particles = new Particle[maxParticles];
	ZeroMemory(particles, sizeof(Particle) * maxParticles);

	// Create buffers
	// Index buffer data
	unsigned int* indices = new unsigned int[maxParticles * 6];
	int indexCount = 0;
	for (int i = 0; i < maxParticles * 4; i += 4)
	{
		indices[indexCount++] = i;
		indices[indexCount++] = i + 1;
		indices[indexCount++] = i + 2;
		indices[indexCount++] = i;
		indices[indexCount++] = i + 2;
		indices[indexCount++] = i + 3;
	}
	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = indices;

	// Index buffer
	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.ByteWidth = sizeof(unsigned int) * maxParticles * 6;
	device->CreateBuffer(&ibDesc, &indexData, &indexBuffer);

	D3D11_BUFFER_DESC allParticleBufferDesc = {};
	allParticleBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	allParticleBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	allParticleBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	allParticleBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	allParticleBufferDesc.StructureByteStride = sizeof(Particle);
	allParticleBufferDesc.ByteWidth = sizeof(Particle) * maxParticles;
	device->CreateBuffer(&allParticleBufferDesc, 0, &particleDataBuffer);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = maxParticles;
	device->CreateShaderResourceView(particleDataBuffer, &srvDesc, &particleDataSRV);

	delete[] indices;
}

Emitter::~Emitter()
{
	// Clean up Emitter
	delete[] particles;
	indexBuffer->Release();
	particleDataBuffer->Release();
	particleDataSRV->Release();
}

void Emitter::Update(float deltaTime, float currentTime)
{
	// Update living particles if there are any
	if (liveParticleCount > 0)
	{
		if (firstLiveIndex < firstDeadIndex)
		{
			// Update from firstLiveIndex to firstDeadIndex
			for (int i = firstLiveIndex; i < firstDeadIndex; i++)
				UpdateParticle(currentTime, i);
		}
		else if (firstDeadIndex < firstLiveIndex)
		{
			// Living particles wrap around because the first live particle is after the first dead particle
			// Updates are done in two halves: firstLiveIndex to maxParticles and 0 to firstDeadIndex
			for (int i = firstLiveIndex; i < maxParticles; i++)
				UpdateParticle(currentTime, i);
			for (int i = 0; i < firstDeadIndex; i++)
				UpdateParticle(currentTime, i);
		}
		else
		{
			// Update all particles, since all particles are alive
			for (int i = 0; i < maxParticles; i++)
				UpdateParticle(currentTime, i);
		}
	}

	timeSinceLastEmit += deltaTime;

	// Spawn new Particles if enough time has passed
	while (timeSinceLastEmit > secondsPerParticle)
	{
		SpawnParticle(currentTime);
		timeSinceLastEmit -= secondsPerParticle;
	}
}

void Emitter::UpdateParticle(float currentTime, int index)
{
	// Kill the particle if it has outlived the lifetime
	if (currentTime - particles[index].SpawnTime >= lifetime) {
		firstLiveIndex = (firstLiveIndex + 1) % maxParticles;
		liveParticleCount--;
		return;
	}
}

void Emitter::SpawnParticle(float currentTime)
{
	// Only spawn if we have space for more Particles
	if (liveParticleCount == maxParticles)
		return;

	// Reset the first dead particle
	particles[firstDeadIndex].SpawnTime = currentTime;

	particles[firstDeadIndex].StartPosition = position;
	particles[firstDeadIndex].StartPosition.x += (((float)rand() / RAND_MAX) * 2 - 1) * positionRandomRange.x;
	particles[firstDeadIndex].StartPosition.y += (((float)rand() / RAND_MAX) * 2 - 1) * positionRandomRange.y;
	particles[firstDeadIndex].StartPosition.z += (((float)rand() / RAND_MAX) * 2 - 1) * positionRandomRange.z;

	particles[firstDeadIndex].StartVelocity = startVelocity;
	particles[firstDeadIndex].StartVelocity.x += (((float)rand() / RAND_MAX) * 2 - 1) * velocityRandomRange.x;
	particles[firstDeadIndex].StartVelocity.y += (((float)rand() / RAND_MAX) * 2 - 1) * velocityRandomRange.y;
	particles[firstDeadIndex].StartVelocity.z += (((float)rand() / RAND_MAX) * 2 - 1) * velocityRandomRange.z;

	float rotStartMin = rotationRandomRanges.x;
	float rotStartMax = rotationRandomRanges.y;
	particles[firstDeadIndex].RotationStart = ((float)rand() / RAND_MAX) * (rotStartMax - rotStartMin) + rotStartMin;

	float rotEndMin = rotationRandomRanges.z;
	float rotEndMax = rotationRandomRanges.w;
	particles[firstDeadIndex].RotationEnd = ((float)rand() / RAND_MAX) * (rotEndMax - rotEndMin) + rotEndMin;

	// Increment firstDeadIndex and liveParticleCount
	firstDeadIndex = (firstDeadIndex + 1) % maxParticles;
	liveParticleCount++;
}

void Emitter::Draw(ID3D11DeviceContext * context, Camera * camera, float currentTime)
{
	// Set up buffers
	D3D11_MAPPED_SUBRESOURCE mapped = {};
	context->Map(particleDataBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	memcpy(mapped.pData, particles, sizeof(Particle) * maxParticles);
	context->Unmap(particleDataBuffer, 0);

	UINT stride = 0;
	UINT offset = 0;
	ID3D11Buffer* nullBuffer = 0;
	context->IASetVertexBuffers(0, 1, &nullBuffer, &stride, &offset);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set shaders
	vertexShader->SetMatrix4x4("view", camera->GetViewMatrix());
	vertexShader->SetMatrix4x4("projection", camera->GetProjectionMatrix());
	vertexShader->SetFloat3("acceleration", acceleration);
	vertexShader->SetFloat4("startColor", startColor);
	vertexShader->SetFloat4("endColor", endColor);
	vertexShader->SetFloat("startSize", startSize);
	vertexShader->SetFloat("endSize", endSize);
	vertexShader->SetFloat("lifetime", lifetime);
	vertexShader->SetFloat("currentTime", currentTime);
	vertexShader->SetShader();

	context->VSSetShaderResources(0, 1, &particleDataSRV);
	pixelShader->SetShaderResourceView("particle", texture);
	pixelShader->SetShader();

	// Draw Particles
	if (firstLiveIndex < firstDeadIndex)
	{
		// Draw from firstLiveIndex to firstDeadIndex
		vertexShader->SetInt("startIndex", firstLiveIndex);
		vertexShader->CopyAllBufferData();
		context->DrawIndexed(liveParticleCount * 6, 0, 0);
	}
	else
	{
		// Living particles wrap around because the first live particle is after the first dead particle
		// Draws are done in two halves: firstLiveIndex to maxParticles and 0 to firstDeadIndex
		vertexShader->SetInt("startIndex", firstLiveIndex);
		vertexShader->CopyAllBufferData();
		context->DrawIndexed((maxParticles - firstLiveIndex) * 6, 0, 0);

		vertexShader->SetInt("startIndex", 0);
		vertexShader->CopyAllBufferData();
		context->DrawIndexed(firstDeadIndex * 6, 0, 0);
	}
}
