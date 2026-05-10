#include "Particle.hlsli"

// ☆埼玉県の南は鳥取県☆

Texture2D tex : register(t0);
SamplerState samLinear : register(s0);

float4 createVortex(PS_INPUT input)
{
	//	UV中心
    float2 center = float2(0.5, 0.5);
    float2 d = input.Tex - center;

	//	距離の2乗
    float dist2 = dot(d, d);

	//	√の近似的に線形化する（sqrtなし）
    float dist_approx = dist2 / (0.5f + dist2); // 近似的に0～1へ線形変換
	
	//	角度（-π～π）
    float angle = atan2(d.y, d.x);
	
    float oneValue = 0.125f * 5;
	//	渦巻きのねじれ強度。1/2πを使うことで、渦の切れ目が無くなり自然となる
    float twist = (1.0f / (2.0f * 3.1415926535f)) * (1.0f / oneValue); // 値を上げると渦が強くなる（回転が速い）

	//	回転速度
    float speed = 1.0f;
    float anim = dist_approx * 7.5f + angle * twist - time.x * speed;
	
	//	5本のリングをループ
    float rings = frac(anim * oneValue);

	//	smoothstepで線の太さを制御
    float myLine = smoothstep(0.4995f, 0.5005f, rings);

	//	白リング
    float drawValue = 1.0f - myLine;

    float3 innerColor = float3(1.0f, 0.6f, 1.0f); // ピンク寄り紫
    float3 outerColor = float3(0.2f, 0.0f, 0.4f); // 濃紫

    float3 color = lerp(outerColor, innerColor, drawValue) * drawValue;
	
	//	一定範囲のみを表示するようにしておく
    return lerp( float4(0, 0, 0, 0), 
			lerp(float4(0, 0, 0, 0), float4(color, 1 - (dist_approx * 2.5)), 
			step(1.0f, drawValue)),
			step(dist2, 0.5f * 0.5f)
	);
}

//	シャボン玉？
float4 soapBubbles(PS_INPUT input)
{
	//	UV中心
    float2 center = float2(0.5, 0.5);
    float2 d = input.Tex - center;

	//	距離の2乗
    float dist2 = dot(d, d);

	//	UV: -1..+1 に変換（中心が0）
	float2 uv = input.Tex * 2.0f - 1.0f;

	//	距離の2乗（sqrt/distanceは使わない）
	float r2 = dot(uv, uv);							// 0 at center, increases outward. max at corner = 2.0

	// パラメータ（調整可）
	float radius = 1.0f;							//	見かけ半径（0..~1）
	float radiusSq = radius * radius;				//	閾値は距離^2 の単位
	float width = 0.06f;							//	リング幅（r2 単位でのぼかし量。小さくすると細い）
	float sharpness = 2.0f;							//	リングの強さのコントロール（pow の指数）

	//	--- リングマスク（縁で 1、中は 0） ---
	//	smoothstep で r2 = radiusSq ± width のあたりだけが 0→1 になるようにする
	float ringMask = smoothstep(radiusSq + width, radiusSq - width, r2);
	//	注意: smoothstep(edge0,edge1,x) は edge0<=edge1 で正しく働くが、
	//	上の順序により外側で1になるよう調整しています（反転の扱い）。
	//	safe alternative (more explicit):
	//	float ringMask = 1.0f - smoothstep(radiusSq - width, radiusSq + width, r2);

	//	さらに細く・強くしたい場合は指数で調整
	ringMask = pow(saturate(ringMask), sharpness);

	//	--- 虹色風の干渉っぽい色を作る（縁でのみ適用） ---
	//	ここは uv の方向成分を使い位相差を出す（time.x は任意で小さなゆらぎに使える）
	float phase = 0.0f;								//	基本は時間変化不要（必要なら small nonzero）
	float freq = 8.0f;								//	虹の細かさ
	float interfer = uv.x * freq + uv.y * (freq * 0.6f) + phase;

	float3 bubbleColor = float3(
		0.5f + 0.5f * sin(interfer + 0.0f),
		0.5f + 0.5f * sin(interfer + 2.0943951f),	//	120deg
		0.5f + 0.5f * sin(interfer + 4.1887902f)	//	240deg
	);

	//	色は縁領域のみ。中心はほぼ透明にする。
	float3 color = bubbleColor * ringMask;

	//	アルファは中心に近いほど基底値（0.01）、縁で大きく
	float alpha = 0.01f + 0.9f * ringMask;			//	最大で ~0.91。必要なら 0.8 等に下げてください。

	//	画面外や完全に外側は透明に抑える
	float outerFade = saturate(r2 / 2.6f);			//	安全マスク。コーナーで消える
	alpha *= outerFade;
	color *= outerFade * 1.1f;

	return float4(color, saturate(alpha));
}

float4 main(PS_INPUT input) : SV_Target
{
	return createVortex(input);
}
