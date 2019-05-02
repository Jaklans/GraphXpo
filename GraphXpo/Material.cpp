#include "Material.h"

Material::Material(std::shared_ptr<SimpleVertexShader> const& vertex, std::shared_ptr<SimplePixelShader> const& pixel, ID3D11ShaderResourceView* diff, ID3D11SamplerState* sampler)
{
	vs = vertex;
	ps = pixel;

	diffuse = diff;
	textureSampler = sampler;
}

Material::Material(std::shared_ptr<SimpleVertexShader> const & vertex, std::shared_ptr<SimplePixelShader> const & pixel, ID3D11ShaderResourceView * diff, ID3D11ShaderResourceView * spec, ID3D11SamplerState * sampler)
{
	vs = vertex;
	ps = pixel;

	diffuse = diff;
	specular = spec;
	textureSampler = sampler;
}

Material::Material(std::shared_ptr<SimpleVertexShader> const & vertex, std::shared_ptr<SimplePixelShader> const & pixel, ID3D11ShaderResourceView * diff, ID3D11ShaderResourceView * spec, ID3D11ShaderResourceView * norm, ID3D11SamplerState * sampler)
{
	vs = vertex;
	ps = pixel;

	diffuse = diff;
	specular = spec;
	normal = norm;
	textureSampler = sampler;
}

//pbr material (metalness-roughness workflow)
Material::Material(std::shared_ptr<SimpleVertexShader> const & vertex, std::shared_ptr<SimplePixelShader> const & pixel, ID3D11ShaderResourceView * diff, ID3D11ShaderResourceView * metal, ID3D11ShaderResourceView * rough, ID3D11ShaderResourceView * norm, ID3D11SamplerState * sampler)
{
	vs = vertex;
	ps = pixel;

	diffuse = diff;
	metalness = metal;
	roughness = rough;
	specular = nullptr;
	normal = norm;
	textureSampler = sampler;
}

Material::~Material()
{
	//release textures + samplers
	if (diffuse) { diffuse->Release(); }
	if (specular) { specular->Release(); }
	if (normal) { normal->Release(); }
	//textureSampler->Release();
}

std::shared_ptr<SimpleVertexShader> Material::GetVertexShader()
{
	return vs;
}

std::shared_ptr<SimplePixelShader> Material::GetPixelShader()
{
	return ps;
}

ID3D11ShaderResourceView* Material::GetDiffuse()
{
	return diffuse;
}
ID3D11ShaderResourceView * Material::GetSpecular()
{
	return specular;
}
ID3D11ShaderResourceView * Material::GetMetalness()
{
	return metalness;
}
ID3D11ShaderResourceView * Material::GetRoughness()
{
	return roughness;
}
ID3D11ShaderResourceView * Material::GetNormal()
{
	return normal;
}
ID3D11SamplerState* Material::GetSamplerState()
{
	return textureSampler;
}

