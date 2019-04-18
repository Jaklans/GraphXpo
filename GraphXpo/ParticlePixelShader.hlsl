
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
	float4 color		: TEXCOORD1;
};

Texture2D particle		: register(t0);
SamplerState trilinear	: register(s0);


float4 main(VertexToPixel input) : SV_TARGET
{
	// Sample texture and combine with input color
	float4 color = particle.Sample(trilinear, input.uv) * input.color;

	// Return either particle color or white (for debugging)
	return color;
}