#define LIGHT_TYPE_DIR		0
#define LIGHT_TYPE_POINT	1
#define LIGHT_TYPE_SPOT		2

#define MIN_ROUGHNESS		0.01
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
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: SV_POSITION;
	float3 normal		: NORMAL;
	float3 worldPos		: POSITION; // world-space position of the vertex
	float3 tangent		: TANGENT;
	float2 UV			: TEXCOORD;
};

//Trowbridge-Reitz BRDF, as supplied by class materials
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

float GeometricShadowing(float3 normal, float3 viewVec, float3 halfVec, float roughness)
{
	float k = pow(roughness + 1, 2) / 8.0f;
	float NdotV = saturate(dot(normal, viewVec));

	return NdotV / (NdotV * (1 - k) + k);
}

float3 MicrofacetBRDF(float3 normal, float3 halfVec, float3 viewVec, float3 toLight, float3 specColor, float roughness)
{
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

float3 DirLight(Light light, VertexToPixel input)
{
	//Lambert Lighting + Ambient ////////////////////////////////////////////

	//calculate the normalized direction to the light
	float3 toLight = normalize(-light.Direction);

	//calculate light amount on this pixel
	float nDotL = saturate(dot(input.normal, toLight));

	//scale the diffuse by the amount of light hitting and add the ambient
	float3 diffuse = (light.Color * nDotL) + (light.Color*0.1);

	//specular lighting ///////////////////////////////////////////////////////

	float3 viewVec = cameraPos - input.worldPos;
	float3 halfVec = (toLight + viewVec) / 2;
	float3 specColor = metallicTexture.Sample(basicSampler, input.UV);
	float roughness = roughnessTexture.Sample(basicSampler, input.UV).r;

	float3 specular = MicrofacetBRDF(input.normal, halfVec, viewVec, toLight, specColor, roughness);


	float3 output = DiffuseEnergyConserve(diffuse, specular, specColor) + specular;

	return output * light.Intensity;
}

float3 PointLight(Light light, VertexToPixel input)
{
	//Lambert Lighting + Ambient ////////////////////////////////////////////

	//calculate the normalized direction to the light
	float3 toLight = normalize(light.Position - input.worldPos);

	//calculate light amount on this pixel
	float nDotL = saturate(dot(input.normal, toLight));

	//scale the diffuse by the amount of light hitting and add the ambient
	float3 diffuse = (light.Color * nDotL) + (light.Color*0.1);

	//specular lighting ///////////////////////////////////////////////////////

	float3 viewVec = cameraPos - input.worldPos;
	float3 halfVec = (toLight + viewVec) / 2;
	float3 specColor = metallicTexture.Sample(basicSampler, input.UV);
	float3 roughness = roughnessTexture.Sample(basicSampler, input.UV);

	float3 specular = MicrofacetBRDF(input.normal, halfVec, viewVec, toLight, specColor, roughness);

	float3 output = DiffuseEnergyConserve(diffuse, specular, specColor) + specular;


	float dist = length(light.Position - input.worldPos);

	float attenuation = saturate(1.0-dist*dist/(light.Range * light.Range));
	attenuation *= attenuation;

	return output * attenuation * light.Intensity;
}

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	//normalize the normal from VS, as interpolation can result in non-unit vectors
	input.normal = normalize(input.normal);

	//ensure that tangent is still orthogonal (again, interpolation can cause issues)
	//implements the gram-schmidt process
	input.tangent = normalize(input.tangent - dot(input.tangent, input.normal) * input.normal);

	//pull the normal from the normal map ////////////////////////////////////////////
	float3 unpackedNorm = (float3)normalTexture.Sample(basicSampler, input.UV) * 2.0f - 1.0f;

	float3 biTangent = cross(input.normal, input.tangent);

	float3x3 TBN = float3x3(input.tangent, biTangent, input.normal);
	
	//Transform the normal from normal map to world space
	input.normal = mul(unpackedNorm, TBN);

	
	//calculate color according to diffuse and lighting ///////////////////////////////
	float4 surfaceColor = diffuseTexture.Sample(basicSampler, input.UV);

	surfaceColor = pow(surfaceColor, 2.2); //un-gamma correct diffuse surface color
	
	//process all lights this frame
	float3 lightColor = float3(0, 0, 0);
	for (int i = 0; i < lightCount; i++)
	{
		switch (lights[i].Type)
		{
		case LIGHT_TYPE_DIR: lightColor += DirLight(lights[i], input);			break;
		case LIGHT_TYPE_POINT: lightColor += PointLight(lights[i], input);		break;
		}
	}

	float4 finalColor = surfaceColor * float4(lightColor,1); //apply lighting to the sampled surface color

	finalColor = pow(finalColor, 1 / 2.2); //gamma correct the final color

	return finalColor;
}

