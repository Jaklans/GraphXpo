#define LIGHT_TYPE_DIR		0
#define LIGHT_TYPE_POINT	1
#define LIGHT_TYPE_SPOT		2

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
	int isRefractive;
	float3 cameraPos;

	matrix view;
};

Texture2D diffuseTexture : register(t0);
Texture2D specularTexture : register(t1);
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

float3 DirLight(Light light, VertexToPixel input)
{
	//Lambert Lighting + Ambient ////////////////////////////////////////////

	//calculate the normalized direction to the light
	float3 toLight = normalize(-light.Direction);

	//calculate light amount on this pixel
	float nDotL = saturate(dot(input.normal, toLight));

	//scale the diffuse by the amount of light hitting and add the ambient
	float3 output = (light.Color * nDotL) + (light.Color*0.1);

	//specular lighting ///////////////////////////////////////////////////////

	float3 dirToCam = cameraPos - input.worldPos;
	float3 h = normalize(toLight + dirToCam);
	float NdotH = saturate(dot(input.normal, h));

	float specAmt = pow(NdotH, 30);

	output += float3(specAmt.rrr * specularTexture.Sample(basicSampler, input.UV).r);

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
	float3 output = (light.Color * nDotL) + (light.Color*0.1);

	//specular lighting ///////////////////////////////////////////////////////

	float3 dirToCam = cameraPos - input.worldPos;
	float3 h = normalize(toLight + dirToCam);
	float NdotH = saturate(dot(input.normal, h));

	float specAmt = pow(NdotH, 30);

	output += float3(specAmt.rrr * specularTexture.Sample(basicSampler, input.UV).r);

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

	finalColor.a = mul((input.worldPos - cameraPos), view).z / 99.9f;

	return finalColor;
}

