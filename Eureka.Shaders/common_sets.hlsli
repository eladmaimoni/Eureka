
//
// Set 0 - per frame, per view data
//

struct PerView
{
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
};


cbuffer perView : register(b0) 
{ 
	PerView perView;
}