//--------------------------------------------------------------------------------------
// File: Shadow.cpp
//
// 影を描画するクラス
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "Shadow.h"


/*
* @brief コンストラクタ
*
* @param[in]  なし
*
* @return なし
*/
Shadow::Shadow()
	:
	m_isOnBlock(false)
{
}

/*
* @brief デストラクタ
*
* @param[in]  なし
*
* @return なし
*/
Shadow::~Shadow()
{
}

/*
* @brief 初期化処理
*
* @param[in]  startPos 開始座標
*
* @return なし
*/
void Shadow::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
	// ベーシックエフェクトの作成
	m_basicEffect = std::make_unique<DirectX::BasicEffect>(device);
	m_basicEffect->SetVertexColorEnabled(false);
	m_basicEffect->SetTextureEnabled(true);

	// プリミティブバッチの作成
	m_primitiveBatch = std::make_unique<DirectX::PrimitiveBatch<DirectX::VertexPositionTexture>>(context);

	// 入力レイアウトの作成
	DX::ThrowIfFailed(
		DirectX::CreateInputLayoutFromEffect<DirectX::VertexPositionTexture>(
			device,
			m_basicEffect.get(),
			m_inputLayout.ReleaseAndGetAddressOf())
	);
	DX::ThrowIfFailed
	(
		DirectX::CreateDDSTextureFromFile(
			device, L"Resources/Textures/shadow.dds",
			nullptr,
			m_shadowTexture.ReleaseAndGetAddressOf())
	);
}

/*
* @brief 描画処理
*
* @param[in]　context デバイスコンテキスト
* @param[in]　states コモンステート
* @param[in]　position 影の位置
* @param[in]　radius 影の半径
*
* @return なし
*/
void Shadow::Draw(ID3D11DeviceContext* context, DirectX::CommonStates* states, DirectX::SimpleMath::Vector3 position, float radius)
{
	// エフェクトの設定と適用
	m_basicEffect->SetWorld(DirectX::SimpleMath::Matrix::Identity);
	m_basicEffect->SetView(m_view);
	m_basicEffect->SetProjection(m_proj);
	m_basicEffect->SetTexture(m_shadowTexture.Get());
	m_basicEffect->Apply(context);

	// インプットレイアウトの設定
	context->IASetInputLayout(m_inputLayout.Get());

	// テクスチャサンプラーの設定
	ID3D11SamplerState* sampler[] = { states->LinearClamp() };
	context->PSSetSamplers(0, 1, sampler);

	// αブレンドの設定
	context->OMSetBlendState(states->AlphaBlend(),nullptr, 0xFFFFFFFF);

	// 深度バッファの設定
	context->OMSetDepthStencilState(m_depthStencilState.Get(), 1);

	// 頂点情報
	DirectX::VertexPositionTexture vartexes[] =
	{
		DirectX::VertexPositionTexture(DirectX::SimpleMath::Vector3::Zero,DirectX::SimpleMath::Vector2(0.0f,0.0f)),
		DirectX::VertexPositionTexture(DirectX::SimpleMath::Vector3::Zero,DirectX::SimpleMath::Vector2(1.0f,0.0f)),
		DirectX::VertexPositionTexture(DirectX::SimpleMath::Vector3::Zero,DirectX::SimpleMath::Vector2(0.0f,1.0f)),
		DirectX::VertexPositionTexture(DirectX::SimpleMath::Vector3::Zero,DirectX::SimpleMath::Vector2(1.0f,1.0f)),
	};

	// 頂点の位置情報
	vartexes[0].position = DirectX::SimpleMath::Vector3(-radius, 0.01f, -radius) + position;
	vartexes[1].position = DirectX::SimpleMath::Vector3(radius, 0.01f, -radius) + position;
	vartexes[2].position = DirectX::SimpleMath::Vector3(-radius, 0.01f, radius) + position;
	vartexes[3].position = DirectX::SimpleMath::Vector3(radius, 0.01f, radius) + position;

	// インデックス番号
	uint16_t indexes[] = { 2,3,1,2,1,0 };

	m_primitiveBatch->Begin();
	m_primitiveBatch->DrawIndexed(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		indexes, _countof(indexes),
		vartexes, _countof(vartexes)
	);
	m_primitiveBatch->End();
}