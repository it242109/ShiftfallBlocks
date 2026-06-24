//--------------------------------------------------------------------------------------
// File: BlockPutParticle.cpp
//
// ブロックを置いたときに出るパーティクルクラス
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "BlockPutParticle.h"

#include "SKLib/BinaryFile.h"
#include "SKLib/DeviceResources.h"
#include "GameObjects/Effects/ParticleUtility.h"

#include <Effects.h>
#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include <WICTextureLoader.h>
#include <CommonStates.h>
#include <vector>
#include <algorithm>
#include <random>
#include <DirectXMath.h>

// 定数の定義
const float BlockPutParticle::EMITTER_RANGE_SCALE = 0.8f;		///< エミッタ―の範囲をどれくらいにするか
const int BlockPutParticle::PARTICLE_COUNT = 6;					///< パーティクルの数
const float BlockPutParticle::MIN_SPARN_RADIUS = 0.8f;			///< パーティクルの最小半径
const float BlockPutParticle::MAX_SPARN_RADIUS = 1.0f;			///< パーティクルの最大半径
const float BlockPutParticle::VELOCITY_HORIZONTAL_WEIGHT = 0.5f;///< 横方向の速度の勢い
const float BlockPutParticle::VELOCITY_UPWARD_BASE = 1.0f;      ///< 上方向の基本速度
const float BlockPutParticle::VELOCITY_SPEED_MIN = 1.5f;		///< 最小初速倍率
const float BlockPutParticle::VELOCITY_SPEED_MAX = 2.0f;		///< 最大初速倍率
const float BlockPutParticle::PARTICLE_LIFETIME = 1.0f;			///< 生存時間（秒）
const float BlockPutParticle::PARTICLE_START_SCALE = 0.5f;      ///< 開始時のスケール

/*
* @brief　インプットレイアウト
*
* @param[in]  なし
* 
* @return     なし
*/
const std::vector<D3D11_INPUT_ELEMENT_DESC> BlockPutParticle::INPUT_LAYOUT =
{
	{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0,							 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	sizeof(DirectX::SimpleMath::Vector3), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, sizeof(DirectX::SimpleMath::Vector3) + sizeof(DirectX::SimpleMath::Vector4), D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

/*
* @brief　コンストラクタ
*
* @param[in]  なし
* 
* @return     なし
*/
BlockPutParticle::BlockPutParticle()
	:
	m_pDR(nullptr),
	m_timer(0.0f)
{
}

/*
* @brief　デストラクタ
*
* @param[in]  なし
* 
* @return     なし
*/
BlockPutParticle::~BlockPutParticle()
{
}

/*
* @brief　テクスチャリソース読み込み関数
*
* @param[in]  path 相対パス
* 
* @return     なし
*/
void BlockPutParticle::LoadTexture(const wchar_t* path)
{
	// テクスチャリソースの読み込み
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;
	DirectX::CreateWICTextureFromFile(m_pDR->GetD3DDevice(), path, nullptr, texture.ReleaseAndGetAddressOf());

	m_texture.push_back(texture);
}

/*
* @brief　生成関数
*
* @param[in]  pDR　ユーザーリソース等から持ってくる
* 
* @return     なし
*/
void BlockPutParticle::Create(DX::DeviceResources* pDR)
{
	m_pDR = pDR;
	ID3D11Device1* device = pDR->GetD3DDevice();

	//	シェーダーの作成
	CreateShader();

	// カスタム深度ステンシル状態の作成
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS; 

	depthStencilDesc.StencilEnable = FALSE;  // ステンシルテストは無効化

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	device->CreateDepthStencilState(&depthStencilDesc, depthStencilState.GetAddressOf());
	m_customDepthStencilState = depthStencilState;

	//	画像の読み込み
	LoadTexture(L"Resources/Textures/blockPut.png");

	//	プリミティブバッチの作成
	m_batch = std::make_unique<DirectX::PrimitiveBatch<DirectX::VertexPositionColorTexture>>(pDR->GetD3DDeviceContext());

	m_states = std::make_unique<DirectX::CommonStates>(device);
}

/*
* @brief　ランダムな浮動小数点数を生成する関数
*
* @param[in]  min 最小値
* @param[in]  max 最大値
*
* @return   minからmaxの範囲のランダムな浮動小数点数
*/
float BlockPutParticle::RandomFloat(float min, float max)
{
	return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}


/*
* @brief　更新処理
*
* @param[in] elapsedTime 前フレームからの経過時間
*
* @return     なし
*/
void BlockPutParticle::Update(float elapsedTime)
{
	// 複数のパーティクルを生成
	m_timer += elapsedTime;
	// ランダムエンジンの初期化
	std::random_device seed;
	std::default_random_engine engine(seed());
	std::uniform_real_distribution<> dist(0, DirectX::XM_2PI);

	for (const auto& emitter : m_emitters)
	{
		// 色を取得
		DirectX::SimpleMath::Color baseColor(1.0f, 1.0f, 1.0f, 1.0f);

		// 発生範囲の設定
		float rangeX = emitter.scale.x * EMITTER_RANGE_SCALE;
		float rangeZ = emitter.scale.z * EMITTER_RANGE_SCALE;

		const int particleCount = PARTICLE_COUNT;

		for (int i = 0; i < particleCount; ++i)
		{
			// ランダムな位置計算
			float angle = static_cast<float>(dist(engine));
			float radius = RandomFloat(MIN_SPARN_RADIUS, MAX_SPARN_RADIUS);

			float offsetX = radius * rangeX * cosf(angle);
			float offsetZ = radius * rangeZ * sinf(angle);

			float spawnY = emitter.position.y + (emitter.scale.y * VELOCITY_HORIZONTAL_WEIGHT);

			DirectX::SimpleMath::Vector3 position(
				emitter.position.x + offsetX,
				spawnY,
				emitter.position.z + offsetZ
			);

			// 上方向へ昇っていく動き
			DirectX::SimpleMath::Vector3 velocity(offsetX * VELOCITY_HORIZONTAL_WEIGHT, 
				VELOCITY_UPWARD_BASE, offsetZ * VELOCITY_HORIZONTAL_WEIGHT);
			velocity.Normalize();
			velocity *= RandomFloat(VELOCITY_SPEED_MIN, VELOCITY_SPEED_MAX);

			// パーティクル生成
			ParticleUtility pU(
				PARTICLE_LIFETIME,											// 生存時間
				position,													// 位置
				velocity,													// 速度
				DirectX::SimpleMath::Vector3::Zero,							// 加速度
				DirectX::SimpleMath::Vector3::One * PARTICLE_START_SCALE,   // 初期スケール
				DirectX::SimpleMath::Vector3::Zero,							// 最終スケール
				baseColor,													// 開始色
				DirectX::SimpleMath::Color(baseColor.x, baseColor.y, baseColor.z, 0.0f) // 終了色（透明へ）
			);
			m_particleUtility.push_back(pU);
		}
	}

	m_timer = 0.0f;

	// エミッターリストをクリア
	m_emitters.clear();

	// 既存パーティクルの更新と寿命による削除
	m_particleUtility.remove_if([elapsedTime](ParticleUtility& p)
		{
			return !p.Update(elapsedTime);
		});
}
/*
* @brief　シェーダー作成部分だけ分離した関数
*
* @param[in]  なし
* 
* @return     なし
*/
void BlockPutParticle::CreateShader()
{
	ID3D11Device1* device = m_pDR->GetD3DDevice();

	//	コンパイルされたシェーダファイルを読み込み
	std::unique_ptr<BinaryFile> VSData = BinaryFile::LoadFile(L"Resources/Shaders/ObjectParticleVS.cso");
	std::unique_ptr<BinaryFile> PSData = BinaryFile::LoadFile(L"Resources/Shaders/ObjectParticlePS.cso");
	std::unique_ptr<BinaryFile> GSData = BinaryFile::LoadFile(L"Resources/Shaders/ObjectParticleGS.cso");

	//	インプットレイアウトの作成
	device->CreateInputLayout(&INPUT_LAYOUT[0],
		static_cast<UINT>(INPUT_LAYOUT.size()),
		VSData->GetData(), VSData->GetSize(),
		m_inputLayout.GetAddressOf());

	//	頂点シェーダ作成
	if (FAILED(device->CreateVertexShader(VSData->GetData(), VSData->GetSize(), NULL, m_vertexShader.ReleaseAndGetAddressOf())))
	{// エラー
		MessageBox(0, L"CreateVertexShader Failed.", NULL, MB_OK);
		return;
	}

	//	ピクセルシェーダ作成
	if (FAILED(device->CreatePixelShader(PSData->GetData(), PSData->GetSize(), NULL, m_pixelShader.ReleaseAndGetAddressOf())))
	{// エラー
		MessageBox(0, L"CreatePixelShader Failed.", NULL, MB_OK);
		return;
	}

	//	ジオメトリシェーダ作成
	if (FAILED(device->CreateGeometryShader(GSData->GetData(), GSData->GetSize(), NULL, m_geometryShader.ReleaseAndGetAddressOf())))
	{// エラー
		MessageBox(0, L"CreateGeometryShader Failed.", NULL, MB_OK);
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
void BlockPutParticle::Render(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj)
{
	ID3D11DeviceContext1* context = m_pDR->GetD3DDeviceContext();

	m_world = DirectX::SimpleMath::Matrix::Identity;

	//	ビルボード設定時にもらったカメラ情報から、視線ベクトルを計算する
	DirectX::SimpleMath::Vector3 cameraDir = m_cameraTarget - m_cameraPosition;
	//	視線ベクトルは正規化しておく
	cameraDir.Normalize();

	//	パーティクル情報を、カメラからの距離順でソートする
	m_particleUtility.sort(
		//	ソート処理の基準を示す関数については、ラムダ式で指定する
		[&](ParticleUtility lhs, ParticleUtility  rhs)
		{
			//	カメラ正面の距離でソート
			return cameraDir.Dot(lhs.GetPosition() - m_cameraPosition) > cameraDir.Dot(rhs.GetPosition() - m_cameraPosition);
		});

	//	表示に使う頂点リストに登録されているデータを全削除
	m_vertices.clear();
	//	パーティクル情報から、表示に使う頂点リストを生成する
	for (ParticleUtility& li : m_particleUtility)
	{
		if (cameraDir.Dot(li.GetPosition() - m_cameraPosition) < 0.0f) 
		{
			continue;
		}

		DirectX::VertexPositionColorTexture vPCT{};

		//	表示するパーティクルの中心座標のみを入れる。
		vPCT.position = DirectX::XMFLOAT3(li.GetPosition());
		//	テクスチャの色
		vPCT.color = DirectX::XMFLOAT4(li.GetNowColor());
		//	現在のテクスチャのスケールを「XMFLOAT2」のXに入れる。
		vPCT.textureCoordinate = DirectX::XMFLOAT2(li.GetNowScale().x, 0.0f);

		//	頂点情報を1つだけ追加
		m_vertices.push_back(vPCT);
	}

	//	表示する点がない場合は描画を終わる
	if (m_vertices.empty())
	{
		return;
	}

	//	シェーダーに渡す追加のバッファを作成する。(ConstBuffer）
	ConstBuffer cbuff;
	//	ビュー設定
	cbuff.matView = view.Transpose();
	//	プロジェクション設定
	cbuff.matProj = proj.Transpose();
	//	ワールド設定
	cbuff.matWorld = m_world.Transpose();
	cbuff.Diffuse = DirectX::SimpleMath::Vector4(1, 1, 1, 1);

	//	受け渡し用バッファの内容更新(ConstBufferからID3D11Bufferへの変換）
	context->UpdateSubresource(m_CBuffer.Get(), 0, NULL, &cbuff, 0, 0);

	//	シェーダーにバッファを渡す
	ID3D11Buffer* cb[1] = { m_CBuffer.Get() };
	//	頂点シェーダもピクセルシェーダも、同じ値を渡す
	context->VSSetConstantBuffers(0, 1, cb);
	context->PSSetConstantBuffers(0, 1, cb);
	context->GSSetConstantBuffers(0, 1, cb);

	//	画像用サンプラーの登録
	ID3D11SamplerState* sampler[1] = { m_states->LinearWrap() };
	context->PSSetSamplers(0, 1, sampler);

	//	半透明描画指定
	ID3D11BlendState* blendstate = m_states->NonPremultiplied();

	//	透明判定処理
	context->OMSetBlendState(blendstate, nullptr, 0xFFFFFFFF);

	//	深度バッファに書き込み参照する
	context->OMSetDepthStencilState(m_customDepthStencilState.Get(), 0);

	//	カリングはなし
	context->RSSetState(m_states->CullNone());

	//	シェーダをセットする
	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	context->GSSetShader(m_geometryShader.Get(), nullptr, 0);

	//	Create関数で読み込んだ画像をピクセルシェーダに登録する
	for (int i = 0; i < m_texture.size(); i++)
	{
		//	for文で一気に設定する
		context->PSSetShaderResources(i, 1, m_texture[i].GetAddressOf());
	}

	//	インプットレイアウトの登録
	context->IASetInputLayout(m_inputLayout.Get());

	//	板ポリゴンを描画
	m_batch->Begin();
	m_batch->Draw(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST, &m_vertices[0], m_vertices.size());
	m_batch->End();

	//	シェーダの登録を解除しておく
	context->VSSetShader(nullptr, nullptr, 0);
	context->PSSetShader(nullptr, nullptr, 0);
	context->GSSetShader(nullptr, nullptr, 0);
}


/*
* @brief　ビルボード作成関数
*
* @param[in]  target　カメラターゲット
* @param[in]  eye　　 カメラアイ
* @param[in]  up　　　上向きベクトル
* 
* @return     なし
*/
void BlockPutParticle::Billboard(DirectX::SimpleMath::Vector3 target, DirectX::SimpleMath::Vector3 eye, DirectX::SimpleMath::Vector3 up)
{
	m_cameraPosition = eye;
	m_cameraTarget = target;

	m_billboard =
		DirectX::SimpleMath::Matrix::CreateBillboard(DirectX::SimpleMath::Vector3::Zero, eye , up);

	//	回転情報を設定する
	DirectX::SimpleMath::Matrix rot = DirectX::SimpleMath::Matrix::Identity;
	//	180度逆を向いているので、補正するための情報を入れる
	rot._11 = -1;
	rot._33 = -1;

	// 回転を適用
	m_billboard = m_billboard * rot;
	DirectX::SimpleMath::Matrix trans = DirectX::SimpleMath::Matrix::CreateTranslation(m_position);
	m_world = m_billboard * trans;
}
