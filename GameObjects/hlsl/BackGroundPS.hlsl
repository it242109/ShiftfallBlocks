#include "Particle.hlsli"

Texture2D tex : register(t0);
Texture2D tex2 : register(t1);
SamplerState samLinear : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
    // スクロール速度
    float scrollSpeed = 0.3f;
    
    // 元のUV座標をコピー
    float2 scrollUV = input.Tex;
    
    // X座標をずらす
    scrollUV.x += time.x * scrollSpeed;
    
	//	画像表示
    float4 output = tex.Sample(samLinear, scrollUV);

    
    return output;
}