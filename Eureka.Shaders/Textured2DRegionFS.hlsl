// Copyright 2020 Google LLC

Texture2D colorTexture : register(t0);
SamplerState colorSampler : register(s0);

struct VSOutput
{
	[[vk::location(0)]] float2 UV : TEXCOORD0;
};

float4 main(VSOutput input) : SV_TARGET
{
	return colorTexture.Sample(colorSampler, input.UV);
}