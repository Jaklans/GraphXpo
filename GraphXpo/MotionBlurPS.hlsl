
cbuffer Data : register(b0)
{
	float pixelWidth;
	float pixelHeight;
	float blurV;
	float blurH;
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
SamplerState Sampler	: register(s0);


// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	//Fixed 5-point guassian blur - vertical

	float4 totalColor = Pixels.Sample(Sampler, input.uv);
	float blurAmount = 5;
	int numSamples = 1;
	float2 uv;

	if (blurV != 0)
	{
		for (int y = -blurAmount; y <= blurAmount; y += 1)
		{
			uv = input.uv + float2(0, y * pixelHeight);
			totalColor += Pixels.Sample(Sampler, uv);

			numSamples++;
		}
	}

	if (blurH != 0)
	{
		for (int x = -blurAmount; x <= blurAmount; x += 1)
		{
			uv = input.uv + float2(x * pixelWidth, 0);
			totalColor += Pixels.Sample(Sampler, uv);

			numSamples++;
		}
	}



	return totalColor / numSamples ;
	

}

/*
	// Loop and sample adjacent  pixels
	for (int y = -blurAmount; y <= blurAmount; y += 1)
	{
		for (int x = -blurAmount; x <= blurAmount; x += 1)
		{
			float2 uv = input.uv + float2(x * pixelWidth, y * pixelHeight);
			totalColor += ExtractedPixels.Sample(Sampler, uv);

			numSamples++;
		}
	}*/