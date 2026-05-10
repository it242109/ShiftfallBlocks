
Texture2D<float4> Texture : register(t0);
sampler TextureSampler : register(s0);

Texture2D<float4> MaskTexture : register(t1);

cbuffer Parameters : register(b1)
{
    float Rate;
};

float rand(float2 co)
{
    return frac(sin(dot(co, float2(12.9898, 78.233))) * 43758.5453);
}

float4 main(float4 color : COLOR0, float2 texCoord : TEXCOORD0) : SV_Target0
{
    float4 col = Texture.Sample(TextureSampler, texCoord) * color;
    
    // ブロックサイズの指定
    float2 blockSize = float2(32.0f, 18.0f);
    
    // 座標をブロック化
    float2 blockCoord = floor(texCoord * blockSize) / blockSize;
    
    // ブロックごとのランダム値
    float r = rand(blockCoord);
    
    float spread = 0.2f;
    float score = blockCoord.x + (r * spread);
    
    float invScore = (1.0f + spread) - score;

    // α値の計算
    float alpha = step(invScore, Rate * (1.0f + spread));
    
    return float4(col.rgb, col.a * alpha);
}
