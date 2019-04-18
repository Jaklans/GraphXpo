#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>
#include "Camera.h"
#include "SimpleShader.h"

struct Particle
{
	float SpawnTime;
	DirectX::XMFLOAT3 StartPosition;

	DirectX::XMFLOAT3 StartVelocity;
	float RotationStart;

	float RotationEnd;
	DirectX::XMFLOAT3 padding;
};

class Emitter
{
public:
	Emitter(
		int maxParticles,
		int particlesPerSecond,
		float lifetime,
		float startSize,
		float endSize,
		DirectX::XMFLOAT4 startColor,
		DirectX::XMFLOAT4 endColor,
		DirectX::XMFLOAT3 position,
		DirectX::XMFLOAT3 acceleration,
		DirectX::XMFLOAT3 startVelocity,
		DirectX::XMFLOAT3 velocityRandomRange,
		DirectX::XMFLOAT3 positionRandomRange,
		DirectX::XMFLOAT4 rotationRandomRanges,
		ID3D11Device* device,
		ID3D11ShaderResourceView* texture,
		std::shared_ptr<SimpleVertexShader> vertexShader,
		std::shared_ptr<SimplePixelShader> pixelShader
	);
	~Emitter();

	///<summary>
	///Update the Emitter.
	///</summary>
	void Update(float deltaTime, float currentTime);

	///<summary>
	///Update the Particle at the given index.
	///</summary>
	void UpdateParticle(float currentTime, int index);

	///<summary>
	///Spawn a Particle.
	///</summary>
	void SpawnParticle(float currentTime);

	///<summary>
	///Draw the Emitter.
	///</summary>
	void Draw(ID3D11DeviceContext* context, Camera* camera, float currentTime);

private:
	int particlesPerSecond;
	float secondsPerParticle;
	float timeSinceLastEmit;

	int liveParticleCount;
	float lifetime;

	DirectX::XMFLOAT3 acceleration;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 startVelocity;

	// Ranges for Position, Velocity, and Rotation
	DirectX::XMFLOAT3 positionRandomRange;
	DirectX::XMFLOAT3 velocityRandomRange;
	DirectX::XMFLOAT4 rotationRandomRanges;

	// Particle Color
	DirectX::XMFLOAT4 startColor;
	DirectX::XMFLOAT4 endColor;

	// Particle Size
	float startSize;
	float endSize;

	Particle* particles;	// Array of Particles
	int maxParticles;		// Maximum number of Particles from this Emitter
	int firstDeadIndex;		// Index of first dead Particle
	int firstLiveIndex;		// Index of first alive Particle

	ID3D11Buffer* indexBuffer;
	ID3D11Buffer* particleDataBuffer;
	ID3D11ShaderResourceView* particleDataSRV;

	ID3D11ShaderResourceView* texture;
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;
};

