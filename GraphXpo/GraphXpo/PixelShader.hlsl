struct DirectionalLight
{
	float4 AmbientColor;
	float4 DiffuseColor;
	float3 Direction;
};


cbuffer externalData : register(b0)
{
	DirectionalLight dl1;
	DirectionalLight dl2;

	float3 cameraPos;
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
	float2 UV			: TEXCOORD;
};


float4 ApplyLight(DirectionalLight light, VertexToPixel input)
{
	//Lambert Lighting + Ambient ////////////////////////////////////////////
	
	//calculate the normalized direction to the light
	float3 toLight = normalize(-light.Direction);

	//calculate light amount on this pixel
	float nDotL = saturate(dot(input.normal, toLight));

	//scale the diffuse by the amount of light hitting and add the ambient
	float4 output = (light.DiffuseColor * nDotL) + light.AmbientColor;

	//specular lighting ///////////////////////////////////////////////////////

	float3 dirToCam = cameraPos - input.worldPos;
	float3 h = normalize(toLight + dirToCam);
	float NdotH = saturate(dot(input.normal, h));

	float specAmt = pow(NdotH, 30);

	output += float4(specAmt.rrr * specularTexture.Sample(basicSampler, input.UV).r, 1);
	output.a = 1;

	return output;
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
	//normalize the normal, as interpolation can result in non-unit vectors
	input.normal = normalize(input.normal);

	float4 surfaceColor = diffuseTexture.Sample(basicSampler, input.UV);

	float4 finalColor = ApplyLight(dl1, input) + ApplyLight(dl2, input);

	return surfaceColor * finalColor;
}

