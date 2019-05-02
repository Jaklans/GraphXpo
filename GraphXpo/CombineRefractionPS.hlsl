
// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};


Texture2D nonRefractive		: register(t0);
Texture2D refractive		: register(t1);
Texture2D mask		: register(t3);
SamplerState Sampler	: register(s0);

// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	float4 unperturbed = nonRefractive.Sample(Sampler, input.uv);
	float4 perturbed = refractive.Sample(Sampler, input.uv);

	unperturbed.a = 1;
	perturbed.a = 1;

	return lerp(unperturbed, perturbed, mask.Sample(Sampler, input.uv).a);
}


