#define LIGHT_TYPE_DIR		0
#define LIGHT_TYPE_POINT	1
#define LIGHT_TYPE_SPOT		2

#define TILE_INTERVAL		2

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

	float3 scale;

	float totalTime;
};

Texture2D normalTexture : register(t0);
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

	output += float3(specAmt.rrr);

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

	output += float3(specAmt.rrr);

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

	float2 dir1 = normalize(float2(1, 0.5)) * totalTime/500;
	float2 dir2 = normalize(float2(0.4, 0.9)) * totalTime / 500;

	float2 sampleCoord1 = float2(input.UV.r + dir1.r, input.UV.g + dir1.g);
	float2 sampleCoord2 = float2(input.UV.r + dir2.r, input.UV.g + dir2.g);

	//tile the UV according to scale
	sampleCoord1 = float2(sampleCoord1.r * scale.r / TILE_INTERVAL, sampleCoord1.g * scale.b / TILE_INTERVAL);
	sampleCoord2 = float2(sampleCoord2.r * scale.r / TILE_INTERVAL*0.75, sampleCoord2.g * scale.b / TILE_INTERVAL*0.75);

	//pull the normal from the normal map ////////////////////////////////////////////
	float3 unpackedNorm1 = (float3)normalTexture.Sample(basicSampler, sampleCoord1) * 2.0f - 1.0f;
	float3 unpackedNorm2 = (float3)normalTexture.Sample(basicSampler, sampleCoord2) * 2.0f - 1.0f;
	float3 avgNorm = (unpackedNorm1 + unpackedNorm2) / 2;

	float3 biTangent = cross(input.normal, input.tangent);

	float3x3 TBN = float3x3(input.tangent, biTangent, input.normal);
	
	//Transform the normal from normal map to world space
	input.normal = mul(avgNorm, TBN);

	

	//calculate color according to diffuse and lighting ///////////////////////////////
	float4 waterColor = float4((142.0/255.0), (217.0/255.0), (229.0/255.0), 1);
	float4 surfaceColor = waterColor;
	
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

	return finalColor;
}

