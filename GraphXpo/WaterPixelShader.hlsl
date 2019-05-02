#define TILE_INTERVAL		2

struct PointLight
{
	float Range;
	float3 Position;
	float3 Color;
	float Intensity;
	matrix viewProjection[6];
};
struct DirectionalLight
{
	float4 Direction;
	float4 Color;
	float Intensity;
};

#define MAX_LIGHTS 6
cbuffer externalData : register(b0)
{
	PointLight lights[MAX_LIGHTS];
	DirectionalLight dirLight;

	int lightCount;

	float3 cameraPos;

	float3 scale;

	float totalTime;

	matrix view;
	matrix projection;

	int width;
	int height;
};

Texture2D normalTexture   : register(t0);
Texture2D sceneSansWater  : register(t1);
Texture2D mask  : register(t2);


SamplerState basicSampler : register(s0);
SamplerState clampedSampler : register(s1);

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
	float3 tangent		: TANGENT;
	float2 UV			: TEXCOORD;
};

float3 DirLight(DirectionalLight light, VertexToPixel input)
{
	//Lambert Lighting + Ambient ////////////////////////////////////////////

	//calculate the normalized direction to the light
	float3 toLight = normalize(-light.Direction);

	//calculate light amount on this pixel
	float nDotL = saturate(dot(input.normal, toLight));

	//scale the diffuse by the amount of light hitting and add the ambient
	float3 output = (light.Color * nDotL) + (light.Color*0.1);

	//specular lighting ///////////////////////////////////////////////////////

	float3 dirToCam = cameraPos - input.worldPos;
	float3 h = normalize(toLight + dirToCam);
	float NdotH = saturate(dot(input.normal, h));

	float specAmt = pow(NdotH, 30);

	output += float3(specAmt.rrr);

	return output * light.Intensity;
}

float3 PointLights(PointLight light, VertexToPixel input)
{
	//Lambert Lighting + Ambient ////////////////////////////////////////////

	//calculate the normalized direction to the light
	float3 toLight = normalize(light.Position - input.worldPos);

	//calculate light amount on this pixel
	float nDotL = saturate(dot(input.normal, toLight));

	//scale the diffuse by the amount of light hitting and add the ambient
	float3 output = (light.Color * nDotL) + (light.Color*0.1);

	//specular lighting ///////////////////////////////////////////////////////

	float3 dirToCam = cameraPos - input.worldPos;
	float3 h = normalize(toLight + dirToCam);
	float NdotH = saturate(dot(input.normal, h));

	float specAmt = pow(NdotH, 30);

	output += float3(specAmt.rrr);

	float dist = length(light.Position - input.worldPos);

	float attenuation = saturate(1.0-dist*dist/(light.Range * light.Range));
	attenuation *= attenuation;

	return output * attenuation * light.Intensity;
}

//Implements Schlick's approximation to calculate fresnel term (between water and air)
float Fresnel(float3 view, float3 normal)
{
	float r0 = pow((1 - 1.33) / (1 + 1.33), 2);

	float fres = r0 + (1 - r0) * pow((1 - dot(view, normal)), 5);

	return fres;
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
	//normalize the normal from VS, as interpolation can result in non-unit vectors
	input.normal = normalize(input.normal);

	//ensure that tangent is still orthogonal (again, interpolation can cause issues)
	//implements the gram-schmidt process
	input.tangent = normalize(input.tangent - dot(input.tangent, input.normal) * input.normal);

	//scroll the two normal map samples
	float2 dir1 = normalize(float2(1, 0.5)) * totalTime/400;
	float2 dir2 = normalize(float2(-0.4, 1.8)) * totalTime / 400;

	float2 sampleCoord1 = float2(input.UV.r + dir1.r, input.UV.g + dir1.g);
	float2 sampleCoord2 = float2(input.UV.r + dir2.r, input.UV.g + dir2.g);

	//tile the UV according to scale
	sampleCoord1 = float2(sampleCoord1.r * scale.r / TILE_INTERVAL, sampleCoord1.g * scale.b / TILE_INTERVAL);
	sampleCoord2 = float2(sampleCoord2.r * scale.r / TILE_INTERVAL*0.55, sampleCoord2.g * scale.b / TILE_INTERVAL*0.55);

	//pull the normal from the normal map ////////////////////////////////////////////
	float3 unpackedNorm1 = (float3)normalTexture.Sample(basicSampler, sampleCoord1) * 2.0f - 1.0f;
	float3 unpackedNorm2 = (float3)normalTexture.Sample(basicSampler, sampleCoord2) * 2.0f - 1.0f;
	float3 avgNorm = (unpackedNorm1 + unpackedNorm2) / 2;

	float3 biTangent = cross(input.normal, input.tangent);

	float3x3 TBN = float3x3(input.tangent, biTangent, input.normal);
	
	//Transform the normal from normal map to world space
	input.normal = mul(avgNorm, TBN);

	//water color = reflection + refraction + water tint

	//get refraction data
	float2 coords = float2(input.position.x / width, input.position.y / height);
	float2 perturbedCoords = coords + input.normal.xy * float2(0.05, 0.05);
	float4 refraction = lerp(sceneSansWater.Sample(clampedSampler, coords), sceneSansWater.Sample(clampedSampler, perturbedCoords), mask.Sample(clampedSampler, perturbedCoords).a);
	refraction.a = 1;
	
	//get reflection data
	float3 worldReflection = normalize(reflect(normalize(input.worldPos - cameraPos), input.normal));

	matrix viewProj = mul(view, projection);

	float4 reflection = float4(0,0,0,0);

	float startDepth = mul((input.worldPos - cameraPos), view).z / 99.9f;
	float4 prevRayPos = float4(input.worldPos, 1);
	float previousDepth = startDepth;

	int i;
	for (i = 0; i < 16; i++)
	{
		float4 rayPos = float4(input.worldPos + worldReflection * (0.05f + 0.05 * i * i), 1);
		float depth = mul((rayPos - cameraPos), view).z / 99.9f;

		float4 rayPosScreenSpace = mul(rayPos, viewProj);

		rayPosScreenSpace /= rayPosScreenSpace.w;

		int winX = (int)round(((rayPosScreenSpace.x + 1) / 2.0) * width);
		int winY = (int)round(((1 - rayPosScreenSpace.y) / 2.0) * height);
		float2 rayPosUV = float2(winX / (float)width, winY / (float)height);

		float4 pixel = sceneSansWater.Sample(basicSampler, rayPosUV);
		
		float4 maskPixel = mask.Sample(basicSampler, rayPosUV); //make sure that the pixel being considered for reflection is not water (or below it)

		if (depth > pixel.a && previousDepth < pixel.a && maskPixel.a == 0) 
		{
			//refine the ray hit detection with a binary search
			float4 MinRayPos = prevRayPos;
			float4 MaxRayPos = rayPos;
			float4 MidRayPos;
			for (int binStep = 0; i < 10; i++)
			{
				MidRayPos = lerp(MinRayPos, MaxRayPos, 0.5);

				depth = mul((MidRayPos - cameraPos), view).z / 99.9f;

				rayPosScreenSpace = mul(MidRayPos, viewProj);

				rayPosScreenSpace /= rayPosScreenSpace.w;

				winX = (int)round(((rayPosScreenSpace.x + 1) / 2.0) * width);
				winY = (int)round(((1 - rayPosScreenSpace.y) / 2.0) * height);

				pixel = sceneSansWater.Sample(basicSampler, float2(winX / (float)width, winY / (float)height));

				if (depth > pixel.a)
					MaxRayPos = MidRayPos;
				else
					MinRayPos = MidRayPos;
			}

			reflection = pixel;
			break;
		}

		previousDepth = depth;
		prevRayPos = rayPos;
	}

	float fresnel = Fresnel(normalize(cameraPos - input.worldPos), input.normal); //approximate, but better than basic dot product
	
	float4 waterColor = float4((210.0/255.0), (247.0/255.0), (255.0/255.0), 1); //the tint of the water itself

	float4 surfaceColor = lerp(refraction,reflection,fresnel) * waterColor;
	
	//process all lights this frame
	float3 lightColor = float3(0, 0, 0);
	for (int i = 0; i < lightCount; i++)
	{
		lightColor += PointLights(lights[i], input);
	}
	lightColor += DirLight(dirLight, input);

	float4 finalColor = surfaceColor * float4(lightColor,1); //apply lighting to the sampled surface color

	return finalColor;
}

