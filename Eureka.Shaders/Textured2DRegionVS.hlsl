
struct PushConstants
{
	float2 scale; // size of the region, in normalized device coordinates
	float2 translate; // top left corner of the region start, in normalized device coordinates
};

[[vk::push_constant]]
PushConstants pushConstants;



/*

VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE

             y
             ^
    (-1,1)   |    (1,1)
             |
             |
             |
             ------------->x



	(-1,-1)		  (1,-1)


*/

struct VertexData
{
	float2 position;
	float2 uv;
};

static VertexData vertices[6] =
{
	{float2(-1.0, 1.0), float2(0.0, 0.0)},
	{float2(-1.0, -1.0), float2(0.0, 1.0)},
	{float2(1.0, 1.0), float2(0.0, 1.0)},
	{float2(1.0, 1.0), float2(0.0, 1.0)},
	{float2(-1.0, -1.0), float2(0.0, 1.0)},
	{float2(1.0, -1.0), float2(1.0, 1.0)}
};

struct VSOutput
{
	float4 position : SV_POSITION; // position in normalized device coordinates
	[[vk::location(0)]] float2 uv : TEXCOORD0;
};

VSOutput main(uint vertexIndex : SV_VertexID)
{
	VSOutput output = (VSOutput)0;
	output.position = float4(vertices[vertexIndex].position * pushConstants.scale + pushConstants.translate, 0.0, 1.0);
	output.uv = vertices[vertexIndex].uv;
	return output;
}