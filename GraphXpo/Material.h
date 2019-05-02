#pragma once
#include "SimpleShader.h"
#include <memory>

class Material
{
public:
	Material(std::shared_ptr<SimpleVertexShader> const& vertex, std::shared_ptr<SimplePixelShader> const& pixel,
		ID3D11ShaderResourceView* diff, ID3D11SamplerState* sampler
	);

	Material(std::shared_ptr<SimpleVertexShader> const& vertex, std::shared_ptr<SimplePixelShader> const& pixel,
		ID3D11ShaderResourceView* diff, ID3D11ShaderResourceView* spec, ID3D11SamplerState* sampler
	);

	Material(std::shared_ptr<SimpleVertexShader> const& vertex, std::shared_ptr<SimplePixelShader> const& pixel,
		ID3D11ShaderResourceView* diff, ID3D11ShaderResourceView* spec, ID3D11ShaderResourceView* norm, ID3D11SamplerState* sampler
	);

	//pbr material (metalness-roughness workflow)
	Material(std::shared_ptr<SimpleVertexShader> const& vertex, std::shared_ptr<SimplePixelShader> const& pixel,
		ID3D11ShaderResourceView* diff, ID3D11ShaderResourceView* metal, ID3D11ShaderResourceView* rough, ID3D11ShaderResourceView* norm, ID3D11SamplerState* sampler
	);
	~Material();

	//accessors
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	std::shared_ptr<SimplePixelShader> GetPixelShader();


	ID3D11ShaderResourceView* GetDiffuse();
	ID3D11ShaderResourceView* GetSpecular();
	ID3D11ShaderResourceView* GetMetalness();
	ID3D11ShaderResourceView* GetRoughness();
	ID3D11ShaderResourceView* GetNormal();
	ID3D11SamplerState* GetSamplerState();
private:

	//shaders use shared_ptrs so that materials can share shaders and so that
	//the shaders will be cleaned up only when they are no longer referenced
	std::shared_ptr<SimpleVertexShader> vs;
	std::shared_ptr<SimplePixelShader> ps;

	ID3D11ShaderResourceView* diffuse   = 0;
	ID3D11ShaderResourceView* specular  = 0;
	ID3D11ShaderResourceView* metalness = 0;
	ID3D11ShaderResourceView* roughness = 0;
	ID3D11ShaderResourceView* normal    = 0;
	ID3D11SamplerState* textureSampler  = 0;
};

