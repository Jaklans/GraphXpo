
// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{
	float4 position			: SV_POSITION;
	float3 cubeDirection	: DIRECTION;
};


// Texture-related variables
TextureCube sky				: register(t0);
SamplerState basicSampler	: register(s0);


// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	return sky.Sample(basicSampler, input.cubeDirection);
}