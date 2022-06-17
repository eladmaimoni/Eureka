
////////////////////////////////////////////////////////////////////////////
// 
//                         Common Descriptor Sets layout 
// 
// https://www.lei.chat/posts/hlsl-for-vulkan-resources/
////////////////////////////////////////////////////////////////////////////


//
// Set 0 - per frame, per view data
//

struct ViewProjectionTransformData
{
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
};

//cbuffer set0_id000_binding0 : register(b0, space0)
//{
//	ViewProjectionTransformData vpTransformData;
//};

[[vk::binding(0, 0)]] cbuffer set0_id000_binding0 
{
	ViewProjectionTransformData vpTransformData;
};

// this is equivalent to before but we won't get the same name in reflection data
//[[vk::binding(0, 0)]] ConstantBuffer<ViewProjectionTransformData> vpTransformData;


