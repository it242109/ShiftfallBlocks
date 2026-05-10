//--------------------------------------------------------------------------------------
// File: SwirlParticle.cpp
//
// 渦巻パーティクルクラス
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "SwirlParticle.h"

#include "SKLib/BinaryFile.h"
#include "SKLib/DeviceResources.h"
#include <SimpleMath.h>
#include <Effects.h>
#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include <WICTextureLoader.h>
#include <CommonStates.h>
#include <vector>


/*
* @brief　インプットレイアウト
*
* @param[in]  なし
* 
* @return     なし
*/
const std::vector<D3D11_INPUT_ELEMENT_DESC> SwirlParticle::INPUT_LAYOUT =
{
	{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR",	0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(DirectX::SimpleMath::Vector3), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(DirectX::SimpleMath::Vector3) + sizeof(DirectX::SimpleMath::Vector4), D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

/*
* @brief　コンストラクタ
*
* @param[in]  なし
* 
* @return     なし
*/
SwirlParticle::SwirlParticle()
	: 
    m_pDR(nullptr),
	m_time{ 0.0f }
{
}

/*
* @brief　デストラクタ
*
* @param[in]  なし
* 
* @return     なし
*/
SwirlParticle::~SwirlParticle()
{
}

/*
* @brief　テクスチャリソース読み込み関数
*
* @param[in]  path 相対パス
* 
* @return     なし
*/
void SwirlParticle::LoadTexture(const wchar_t* path)
{
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;
	DirectX::CreateWICTextureFromFile(m_pDR->GetD3DDevice(), path, nullptr, texture.ReleaseAndGetAddressOf());

	m_texture.push_back(texture);
}

void SwirlParticle::Update(float deltaTime)
{
	m_time += deltaTime;
}

/*
* @brief　生成関数
*
* @param[in]  pDR　ユーザーリソース等から持ってくる
* 
* @return     なし
*/
void SwirlParticle::Create(DX::DeviceResources* pDR)
{
	m_pDR = pDR;
	ID3D11Device1* device = pDR->GetD3DDevice();

	//	シェーダーの作成
	CreateShader();

	//	プリミティブバッチの作成
	m_batch = std::make_unique<DirectX::PrimitiveBatch<DirectX::VertexPositionColorTexture>>(pDR->GetD3DDeviceContext());

	m_states = std::make_unique<DirectX::CommonStates>(device);
}

/*
* @brief　シェーダー作成部分だけ分離した関数
*
* @param[in]  なし
* 
* @return     なし
*/
void SwirlParticle::CreateShader()
{
	ID3D11Device1* device = m_pDR->GetD3DDevice();

	//	コンパイルされたシェーダファイルを読み込み
	std::unique_ptr<BinaryFile> VSData = BinaryFile::LoadFile(L"Resources/Shaders/SwirlParticleVS.cso");
	std::unique_ptr<BinaryFile> GSData = BinaryFile::LoadFile(L"Resources/Shaders/SwirlParticleGS.cso");
	std::unique_ptr<BinaryFile> PSData = BinaryFile::LoadFile(L"Resources/Shaders/SwirlParticlePS.cso");

	//	インプットレイアウトの作成
	device->CreateInputLayout(&INPUT_LAYOUT[0],
		static_cast<UINT>(INPUT_LAYOUT.size()),
		VSData->GetData(), VSData->GetSize(),
		m_inputLayout.GetAddressOf());

	//	頂点シェーダ作成
	if (FAILED(device->CreateVertexShader(VSData->GetData(), VSData->GetSize(), NULL, m_vertexShader.ReleaseAndGetAddressOf())))
	{//	エラー
		MessageBox(0, L"CreateVertexShader Failed.", NULL, MB_OK);
		return;
	}

	//	ジオメトリシェーダ作成
	if (FAILED(device->CreateGeometryShader(GSData->GetData(), GSData->GetSize(), NULL, m_geometryShader.ReleaseAndGetAddressOf())))
	{//	エラー
		MessageBox(0, L"CreateGeometryShader Failed.", NULL, MB_OK);
		return;
	}
	//	ピクセルシェーダ作成
	if (FAILED(device->CreatePixelShader(PSData->GetData(), PSData->GetSize(), NULL, m_pixelShader.ReleaseAndGetAddressOf())))
	{//	エラー
		MessageBox(0, L"CreatePixelShader Failed.", NULL, MB_OK);
		return;
	}

	//	シェーダーにデータを渡すためのコンスタントバッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	device->CreateBuffer(&bd, nullptr, &m_CBuffer);
}

/*
* @brief　描画処理
*
* @param[in]  view　ビュー行列
* @param[in]  proj　射影行列
* 
* @return     なし
*/
void SwirlParticle::Render()
{
    ID3D11DeviceContext1* context = m_pDR->GetD3DDeviceContext();

    //  スクリーン全体を覆う4つの頂点を作成 
    DirectX::VertexPositionColorTexture vertices[4] =
    {
        // 左
        DirectX::VertexPositionColorTexture(
            DirectX::SimpleMath::Vector3(-1.0f, 1.0f, 0.0f),
            DirectX::SimpleMath::Vector4::One,
            DirectX::SimpleMath::Vector2(0.0f, 0.0f)
        ),
        // 右上
        DirectX::VertexPositionColorTexture(
            DirectX::SimpleMath::Vector3(1.0f, 1.0f, 0.0f),
            DirectX::SimpleMath::Vector4::One,
            DirectX::SimpleMath::Vector2(1.0f, 0.0f)
        ),
        // 左下 
        DirectX::VertexPositionColorTexture(
            DirectX::SimpleMath::Vector3(-1.0f, -1.0f, 0.0f),
            DirectX::SimpleMath::Vector4::One,
            DirectX::SimpleMath::Vector2(0.0f, 1.0f)           
        ),

        // 右下 
        DirectX::VertexPositionColorTexture(
            DirectX::SimpleMath::Vector3(1.0f, -1.0f, 0.0f),
            DirectX::SimpleMath::Vector4::One,
            DirectX::SimpleMath::Vector2(1.0f, 1.0f)
        ),
    };

    // 定数バッファの更新
    ConstBuffer cbuff;
    cbuff.matView = DirectX::SimpleMath::Matrix::Identity;
    cbuff.matProj = DirectX::SimpleMath::Matrix::Identity;
    cbuff.matWorld = DirectX::SimpleMath::Matrix::Identity;
    cbuff.Diffuse = DirectX::SimpleMath::Vector4(1, 1, 1, 1);
    cbuff.time = DirectX::SimpleMath::Vector4(m_time, 1, 1, 1);

    context->UpdateSubresource(m_CBuffer.Get(), 0, NULL, &cbuff, 0, 0);

    // バッファのセット
    ID3D11Buffer* cb[1] = { m_CBuffer.Get() };
    context->VSSetConstantBuffers(0, 1, cb);
    context->PSSetConstantBuffers(0, 1, cb);

    // 深度テスト無効化 
    context->OMSetDepthStencilState(m_states->DepthNone(), 0);
    // 半透明合成有効
    context->OMSetBlendState(m_states->NonPremultiplied(), nullptr, 0xFFFFFFFF);
    // カリングなし
    context->RSSetState(m_states->CullNone());
    // サンプラー設定
    ID3D11SamplerState* sampler[1] = { m_states->LinearWrap() };
    context->PSSetSamplers(0, 1, sampler);

    // シェーダーのセット
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);

    // ジオメトリシェーダの無効化 (nullptr)
    context->GSSetShader(nullptr, nullptr, 0);

    // ピクセルシェーダ
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    // テクスチャセット
    for (int i = 0; i < m_texture.size(); i++)
    {
        context->PSSetShaderResources(i, 1, m_texture[i].GetAddressOf());
    }

    // 描画
    context->IASetInputLayout(m_inputLayout.Get());
    m_batch->Begin();
    // 4頂点のトライアングルストリップとして描画
    m_batch->Draw(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, vertices, 4);
    m_batch->End();
}