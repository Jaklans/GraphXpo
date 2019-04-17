
// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

// Textures and such
float Threshold			: register(b0);
Texture2D Pixels		: register(t0);
SamplerState Sampler	: register(s0);


// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	float4 extractedColor = Pixels.Sample(Sampler, input.uv);
	
	
	if (extractedColor.r> Threshold)
		return extractedColor;

	if (extractedColor.g > Threshold)
		return extractedColor;

	if (extractedColor.b > Threshold)
		return extractedColor;
	
	return float4(0, 0, 0, 0);
}


