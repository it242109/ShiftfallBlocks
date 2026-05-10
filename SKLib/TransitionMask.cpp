//--------------------------------------------------------------------------------------
// File: TransitionMask.cpp
//
// 画面切り替え用のマスク表示クラス
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "TransitionMask.h"
#include "ReadData.h"

// コンストラクタ
TransitionMask::TransitionMask(
	ID3D11Device* device,
	ID3D11DeviceContext* context,
	float interval
)
	: m_interval(interval),
	m_rate(0.0f),
	m_open(true),
	m_request()
{
	// スプライトバッチの作成
	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(context);

	// ピクセルシェーダーの作成
	std::vector<uint8_t> ps_fade = DX::ReadData(L"Resources/Shaders/PS_Fade.cso");
	DX::ThrowIfFailed(
		device->CreatePixelShader(ps_fade.data(), ps_fade.size(), nullptr, m_PS_Fade.ReleaseAndGetAddressOf())
	);

	// 定数バッファの作成
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = static_cast<UINT>(sizeof(ConstantBuffer));	// 確保するサイズ（16の倍数で設定する）
	// GPU (読み取り専用) と CPU (書き込み専用) の両方からアクセスできるリソース
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;	// 定数バッファとして扱う
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	// CPUが内容を変更できるようにする
	DX::ThrowIfFailed(device->CreateBuffer(&bufferDesc, nullptr, m_constantBuffer.ReleaseAndGetAddressOf()));

	// マスク用テクスチャの読み込み
	DX::ThrowIfFailed(
		DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/FadeMask.dds", nullptr, m_maskTexture.ReleaseAndGetAddressOf())
	);
}

// 更新処理
void TransitionMask::Update(float elapsedTime)
{
	if (m_open)
	{
		// オープン
		// (減算量をインターバルで割る＝インターバル分の時間にする)
		// ０以下になったら０にする
		m_rate -= elapsedTime / m_interval;
		m_rate = (m_rate < 0.0f) ? 0 : m_rate;

	}
	else
	{
		// クローズ
		// (加算量をインターバルで割る＝インターバル分の時間にする)
		// 1以上になったら１にする
		m_rate += elapsedTime / m_interval;
		m_rate = (m_rate > 1.0f) ? 1 : m_rate;

	}
}

// 描画処理
void TransitionMask::Draw(
	ID3D11DeviceContext* context,
	DirectX::CommonStates* states,
	ID3D11ShaderResourceView* texture,
	const RECT& rect
)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// GPUが定数バッファに対してアクセスを行わないようにする
	context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	// 定数バッファを更新
	static_cast<ConstantBuffer*>(mappedResource.pData)->rate = m_rate;

	// GPUが定数バッファに対してのアクセスを許可する
	context->Unmap(m_constantBuffer.Get(), 0);

	m_spriteBatch->Begin(
		DirectX::SpriteSortMode_Immediate,
		states->NonPremultiplied(), 
		nullptr, states->DepthNone(), nullptr, [&]()
		{
			// 定数バッファの設定
			ID3D11Buffer* cbuf[] = { m_constantBuffer.Get() };
			context->PSSetConstantBuffers(1, 1, cbuf);

			// マスク用テクスチャを設定
			auto srv = m_maskTexture.Get();
			context->PSSetShaderResources(1, 1, &srv);

			// シェーダーの設定
			context->PSSetShader(m_PS_Fade.Get(), nullptr, 0);
		}
	);

	m_spriteBatch->Draw(texture, rect);
	m_spriteBatch->End();
}

// マスクをオープンする関数
void TransitionMask::Open()
{
	m_open = true;
	m_rate = 1.0f;
}

// マスクをクローズする関数
void TransitionMask::Close()
{
	m_open = false;
	m_rate = 0.0f;
}
