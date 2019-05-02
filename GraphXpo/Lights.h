#pragma once

#include <DirectXMath.h>

struct PointLight
{
	float Range;
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Color;
	float Intensity;
	DirectX::XMFLOAT4X4 viewProjection[6];
};

struct DirectionalLight {
	DirectX::XMFLOAT4 Direction;
	DirectX::XMFLOAT4 Color;
	float Intensity;
};