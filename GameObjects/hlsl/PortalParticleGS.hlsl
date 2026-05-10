#include "Particle.hlsli"

static const int vnum = 4;

static const float4 offset_array[vnum] =
{
    float4(-0.5f, 0.5f, 0.0f, 0.0f), // 左上
    float4(0.5f, 0.5f, 0.0f, 0.0f), // 右上
    float4(-0.5f, -0.5f, 0.0f, 0.0f), // 左下
    float4(0.5f, -0.5f, 0.0f, 0.0f), // 右下
};

[maxvertexcount(vnum)]
void main(
    point PS_INPUT input[1],
    inout TriangleStream<PS_INPUT> output
)
{
    // ビュー行列から右方向と上方向を取得
    float3 right = float3(matView._11, matView._21, matView._31);
    float3 up = float3(matView._12, matView._22, matView._32);

    for (int i = 0; i < vnum; i++)
    {
        PS_INPUT element;

        // オフセットを計算
        float3 offset = right * offset_array[i].x * input[0].Tex.x +
                        up * offset_array[i].y * input[0].Tex.x;

        // パーティクルの位置を計算
        element.Pos = input[0].Pos + float4(offset, 0.0f);

        // ワールド、ビュー、プロジェクション行列を適用
        element.Pos = mul(element.Pos, matWorld);
        element.Pos = mul(element.Pos, matView);
        element.Pos = mul(element.Pos, matProj);

        // 色とUV座標を設定
        element.Color = input[0].Color;
        element.Tex.x = offset_array[i].x + 0.5f;
        element.Tex.y = -offset_array[i].y + 0.5f;

        output.Append(element);
    }

    output.RestartStrip();
}