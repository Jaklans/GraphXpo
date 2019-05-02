
cbuffer Data : register(b0)
{
	float pixelHeight;
}


// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

// Textures and such
Texture2D Pixels		: register(t0);
Texture2D ExtractedPixels		: register(s0);
SamplerState Sampler	: register(s0);


// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	//Fixed 5-point guassian blur - vertical

	float4 totalColor = float4(0,0,0,0);
	float2 uv;

	totalColor += ExtractedPixels.Sample(Sampler, input.uv) * 0.38774;

	uv = input.uv + float2(0, 1 * pixelHeight);
	totalColor += ExtractedPixels.Sample(Sampler, uv) * 0.24477;

	uv = input.uv + float2(0, -1 * pixelHeight);
	totalColor += ExtractedPixels.Sample(Sampler, uv) * 0.24477;

	uv = input.uv + float2(0, 2 * pixelHeight);
	totalColor += ExtractedPixels.Sample(Sampler, uv) * 0.06136;

	uv = input.uv + float2(0, -2 * pixelHeight);
	totalColor += ExtractedPixels.Sample(Sampler, uv) * 0.06136;

	return totalColor + Pixels.Sample(Sampler, input.uv);
	

}