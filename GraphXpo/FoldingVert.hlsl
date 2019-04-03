/*---------------------------------------------------------
A normal Vertex Shader with an additional animatable effect

To use, increment the evolution value between 0.0 and 1.0

This shader should only be used when the effect is actively
running, and should be swapped for the normal shader as 
soon as possible

Now with less documentation!
---------------------------------------------------------*/

cbuffer externalData : register(b0)
{
	matrix world;
	matrix invTransWorld;
	matrix view;
	matrix projection;
	//Progress of the effect
	float evolution;
};
struct VertexShaderInput
{
	float3 position		: POSITION;
	float3 normal		: NORMAL;
	float3 worldPos		: POSITION;
	float2 UV			: TEXCOORD;
};

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 normal		: NORMAL;
	float3 worldPos		: POSITION; 
	float2 UV			: TEXCOORD;
};

VertexToPixel main(VertexShaderInput input)
{
	VertexToPixel output;

	matrix worldViewProj = mul(mul(world, view), projection);

	output.position = mul(float4(input.position, 1.0f), worldViewProj);
	output.normal   = mul(input.normal, (float3x3)invTransWorld);
	output.worldPos = mul(float4(input.position, 1.0f), world).xyz;
	output.UV       = input.UV;

	//output.position = output.position + float4(0.0f, 0.0f, -output.position.z, 0.0f);

	float4 target = float4(
		round(output.position.xy * 4.0f / output.position.w) / 4.0f,
		0.0f,
		1.0f
		);
	float3 normalTarget = float3(0, 0, 1);
	
	//LERP output and target based on evolution
	output.position = output.position * (1.0f - evolution) + target * evolution;
	//output.normal = output.normal * (1.0f - evolution) + normalTarget * evolution;

	return output;
}
	