#define LIGHT_TYPE_DIR		0
#define LIGHT_TYPE_POINT	1
#define LIGHT_TYPE_SPOT		2


static const float F0_NON_METAL = 0.04f;
#define MIN_ROUGHNESS		0.000001
#define PI					3.14159265

//A general purpose light declaration
struct Light
{
	int Type;
	float3 Direction;
	float Range;
	float3 Position;
	float3 Color;
	float Intensity;
};


#define MAX_LIGHTS 128
cbuffer externalData : register(b0)
{
	Light lights[MAX_LIGHTS];

	int lightCount;

	float3 cameraPos;
};

Texture2D diffuseTexture : register(t0);
Texture2D roughnessTexture : register(t1);
Texture2D metallicTexture : register(t2);
Texture2D normalTexture : register(t3);
SamplerState basicSampler : register(s0);

// Struct representing the data we expect to receive from earlier pipeline stages
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 normal		: NORMAL;
	float3 worldPos		: POSITION; 
	float3 tangent		: TANGENT;
	float2 UV			: TEXCOORD;
};

//Trowbridge-Reitz, as supplied by class materials
float SpecularDistribution(float3 normal, float3 halfVec, float roughness)
{
	float NdotH = saturate(dot(normal, halfVec));
	float NdotH2 = NdotH * NdotH;
	float a = roughness * roughness;
	float a2 = max(a*a, MIN_ROUGHNESS);

	//((n dot h)^2 * (a^2 - 1) + 1)
	float denom = NdotH2 * (a2 - 1) + 1;

	return a2 / (PI * denom * denom);
}

//Schlick approximation
float3 Fresnel(float3 viewVec, float3 halfVec, float3 f0)
{
	float VdotH = saturate(dot(viewVec, halfVec));

	return f0 + (1 - f0) * pow(1 - VdotH, 5);
}

//Schlick-GGX (based on Schlick-Beckmann)
float GeometricShadowing(float3 normal, float3 viewVec, float3 halfVec, float roughness)
{
	float k = pow(roughness + 1, 2) / 8.0f;
	float NdotV = saturate(dot(normal, viewVec));

	return NdotV / (NdotV * (1 - k) + k);
}

// f(l,v) = D(h)F(v,h)G(l,v,h) / 4(n dot l)(n dot v)
float3 MicrofacetBRDF(float3 normal, float3 viewVec, float3 toLight, float3 specColor, float roughness)
{
	float3 halfVec = normalize(toLight + viewVec);

	float specDist = SpecularDistribution(normal, halfVec, roughness);
	float3 fresnel = Fresnel(viewVec, halfVec, specColor);
	float geoShadowing = GeometricShadowing(normal, viewVec, halfVec, roughness) * GeometricShadowing(normal, toLight, halfVec, roughness);

	return (specDist * fresnel * geoShadowing) / (4 * max(dot(normal, viewVec), dot(normal, toLight)));
}

//more diffuse = less specular, more specular = less diffuse
float3 DiffuseEnergyConserve(float diffuse, float3 specular, float metalness)
{
	return diffuse * ((1 - saturate(specular)) * (1 - metalness));
}

float3 DirLight(Light light, VertexToPixel input, float4 surfaceColor, float3 specColor, float metalness, float roughness)
{
	//calculate the normalized direction to the light
	float3 toLight = normalize(-light.Direction);
	float3 viewVec = normalize(cameraPos - input.worldPos);
	
	float diffuse = saturate(dot(input.normal, toLight)); //the light amount on this pixel
	float3 specular = MicrofacetBRDF(input.normal, viewVec, toLight, specColor, roughness);

	float3 output = (DiffuseEnergyConserve(diffuse, specular, metalness) * surfaceColor.rgb) + specular;

	return output * light.Intensity * light.Color;
}

float3 PointLight(Light light, VertexToPixel input, float4 surfaceColor, float3 specColor, float metalness, float roughness)
{
	//calculate the normalized direction to the light
	float3 toLight = normalize(light.Position - input.worldPos);
	float3 viewVec = normalize(cameraPos - input.worldPos);

	float diffuse = saturate(dot(input.normal, toLight)); //the light amount on this pixel
	float3 specular = MicrofacetBRDF(input.normal, viewVec, toLight, specColor, roughness);

	float3 output = (DiffuseEnergyConserve(diffuse, specular, metalness) * surfaceColor.rgb) + specular;
	

	float dist = length(light.Position - input.worldPos);

	float attenuation = saturate(1.0-dist*dist/(light.Range * light.Range));
	attenuation *= attenuation;

	return output * attenuation * light.Intensity * light.Color;
}

float3 ApplyNormalMap(VertexToPixel input)
{
	float3 unpackedNorm = (float3)normalTexture.Sample(basicSampler, input.UV) * 2.0f - 1.0f;

	float3 biTangent = cross(input.normal, input.tangent);

	float3x3 TBN = float3x3(input.tangent, biTangent, input.normal);

	//Transform the normal from texture space to world space and return it
	return mul(unpackedNorm, TBN);
}


float4 main(VertexToPixel input) : SV_TARGET
{
	//Ensure normal and tangent are normalized and orthogonal after being interpolated
	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent - dot(input.tangent, input.normal) * input.normal);
	
	input.normal = ApplyNormalMap(input);

	//calc. surface details before lighting
	float4 surfaceColor = diffuseTexture.Sample(basicSampler, input.UV);
	surfaceColor = pow(surfaceColor, 2.2); //un-gamma correct diffuse surface color
	float metalness = metallicTexture.Sample(basicSampler, input.UV).r;
	float roughness = roughnessTexture.Sample(basicSampler, input.UV).r;
	float3 specColor = lerp(F0_NON_METAL.rrr, surfaceColor.rgb, metalness);
	

	//process all lights this frame
	float3 lightColor = float3(0, 0, 0);
	for (int i = 0; i < lightCount; i++)
	{
		switch (lights[i].Type)
		{
		case LIGHT_TYPE_DIR: lightColor += DirLight(lights[i], input, surfaceColor, specColor, metalness, roughness);			break;
		case LIGHT_TYPE_POINT: lightColor += PointLight(lights[i], input, surfaceColor, specColor, metalness, roughness);		break;
		}
	}

	float4 finalColor = float4(lightColor,1); //apply lighting to the sampled surface color

	finalColor = pow(finalColor, 1 / 2.2); //gamma correct the final color

	return finalColor;
}

