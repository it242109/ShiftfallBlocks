//--------------------------------------------------------------------------------------
// File: Shadow.h
//
// 影を描画するクラス
//--------------------------------------------------------------------------------------
#pragma once

#include "SKLib/UserResources.h"

class Shadow
{
public:
	// コンストラクタ
	Shadow();

	// デストラクタ
	~Shadow();

	// 初期化
	void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);

	// 描画
	void Draw(ID3D11DeviceContext* context,
		DirectX::CommonStates* states,
		DirectX::SimpleMath::Vector3 position,
		float radius = 1.5f
	);

private:

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shadowTexture;

	// ベーシックエフェクト
	std::unique_ptr<DirectX::BasicEffect> m_basicEffect;

	// プリミティブバッチ
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionTexture>> m_primitiveBatch;

	// 入力レイアウトへのポインタ
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

	// ビュー行列
	DirectX::SimpleMath::Matrix m_view;

	// 射影行列
	DirectX::SimpleMath::Matrix m_proj;

	// 深度ステンシルステート影
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;

	// 地面についたかどうか
	bool m_isOnBlock;
};

