
// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{

	float4 position		: SV_POSITION;
	float3 normal		: NORMAL;
	float3 worldPos		: POSITION; // world-space position of the vertex
	float3 tangent		: TANGENT;
	float2 UV			: TEXCOORD;
};

// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	return float4(1,1,1,1);
}


