#pragma once

#include <DirectXMath.h>

#define LIGHT_TYPE_DIR		0
#define LIGHT_TYPE_POINT	1
#define LIGHT_TYPE_SPOT		2

//A general purpose light declaration
struct Light
{
	int Type;
	DirectX::XMFLOAT3 Direction; //16 bytes
	float Range;
	DirectX::XMFLOAT3 Position; //32 bytes
	DirectX::XMFLOAT3 Color;
	float Intensity; //48 bytes
};