#include "UI.hlsli"

Texture2D tex : register(t0);
Texture2D tex2 : register(t1);
SamplerState samLinear : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
	//	‰æ‘œ•\Ž¦
	float4 output = tex.Sample(samLinear, input.tex);

	//	‰æ‘œ•\Ž¦‚»‚Ì2
    float4 output2 = tex2.Sample(samLinear, input.tex);
	
	float4 ret;
    ret = lerp(float4(0, 0, 0, 0), output, 1.0f);
	
	return ret;
}