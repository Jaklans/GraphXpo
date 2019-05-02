
cbuffer Data : register(b0)
{
	float pixelWidth;
}


// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

// Textures and such
Texture2D BrightPixels		: register(s0);
SamplerState Sampler	: register(s0);


// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	//Fixed 5-point guassian blur - horizontal
	
	float4 totalColor = float4(0,0,0,0);
	float2 uv;

	totalColor += BrightPixels.Sample(Sampler, input.uv) * 0.38774;

	uv= input.uv + float2(1 * pixelWidth, 0);
	totalColor += BrightPixels.Sample(Sampler, uv) * 0.24477;

	uv = input.uv + float2(-1 * pixelWidth, 0);
	totalColor += BrightPixels.Sample(Sampler, uv) * 0.24477;

	uv = input.uv + float2(2 * pixelWidth, 0);
	totalColor += BrightPixels.Sample(Sampler, uv) * 0.06136;

	uv = input.uv + float2(-2 * pixelWidth, 0);
	totalColor += BrightPixels.Sample(Sampler, uv) * 0.06136;

	return totalColor;
}

