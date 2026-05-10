//--------------------------------------------------------------------------------------
// File: LoadScene.cpp
//
// ロードシーンクラス
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "LoadScene.h"

#include <WICTextureLoader.h>
#include <SimpleMath.h>


/*
* @brief　コンストラクタ
*
* @param[in]  なし
*
* @return     なし
*/
LoadScene::LoadScene()
{
}

/*
* @brief　デストラクタ
*
* @param[in]  なし
*
* @return     なし
*/
LoadScene::~LoadScene()
{
}

/*
* @brief　初期化処理
*
* @param[in]  なし
*
* @return     なし
*/
void LoadScene::Initialize()
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();

	// マスクの初期化
	auto transitionMask = GetUserResources()->GetTransitionMask();
	transitionMask->Open();

	// 仕掛けブロックの初期化
	m_gimmickBlockPosition = { 0.0f,0.8f,2.0f };
	m_gimmickBlockScale = { 0.7f,0.7f,0.7f };
	m_gimmickBlockRotate = DirectX::SimpleMath::Quaternion::Identity;

	// カメラの初期化(床に合わせる)
	m_camera.SetPlayer(m_gimmickBlockPosition, m_gimmickBlockRotate);
}

/*
* @brief　更新処理
*
* @param[in]  elapsedTime 前フレームからの経過時間
*
* @return     なし
*/
void LoadScene::Update(float elapsedTime)
{

	// カメラの更新
	m_camera.Update(elapsedTime *= 2);
}

/*
* @brief　描画処理
*
* @param[in]  なし
*
* @return     なし
*/
void LoadScene::Render()
{
	auto context = GetUserResources()->GetDeviceResources()->GetD3DDeviceContext();
	auto states = GetUserResources()->GetCommonStates();
	// デフォルトのマトリックス
	DirectX::SimpleMath::Matrix rot = DirectX::SimpleMath::Matrix::CreateRotationY(0.0f);

	// カメラの位置とターゲットを取得
	auto eye = m_camera.GetEyePosition();
	const auto& target = m_camera.GetTargetPosition();

	if (DirectX::SimpleMath::Vector3::DistanceSquared(eye, target) < 0.0001f)
	{
		eye.z -= 0.1f;
	}

	// ビューを回るカメラに固定
	m_view = DirectX::SimpleMath::Matrix::CreateLookAt(
		eye,
		target,
		DirectX::SimpleMath::Vector3::UnitY
	);

	m_spriteBatch->Begin();

	//　背景の描画
	m_spriteBatch->Draw(m_loadingSRV.Get(), ScreenManager::Pos(0.0f, 0.0f), nullptr,
		DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
		ScreenManager::Scale(1.0f, 1.0f));

	m_spriteBatch->End();

	// ブロックの描画
	DirectX::SimpleMath::Matrix gimmick_s = DirectX::SimpleMath::Matrix::CreateScale(m_gimmickBlockScale);
	DirectX::SimpleMath::Matrix gimmick_t = DirectX::SimpleMath::Matrix::CreateTranslation(m_gimmickBlockPosition);
	DirectX::SimpleMath::Matrix gimmickWorld = gimmick_s * rot * gimmick_t;
	m_platformBlockModel->Draw(context, *states, gimmickWorld, m_view, m_proj);

}

/*
* @brief　終了処理
*
* @param[in]  なし
*
* @return     なし
*/
void LoadScene::Finalize()
{
}

/*
* @brief　デバイスに依存するリソースを作成する関数
*
* @param[in]  なし
*
* @return     なし
*/
void LoadScene::CreateDeviceDependentResources()
{
	auto deviceResources = GetUserResources()->GetDeviceResources();
	auto device = deviceResources->GetD3DDevice();
	auto context = deviceResources->GetD3DDeviceContext();

	std::unique_ptr<DirectX::EffectFactory> fx;
	fx = std::make_unique<DirectX::EffectFactory>(device);
	fx->SetDirectory(L"Resources/Models");

	// スプライトバッチの作成
	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(context);

	// モデルの読み込み
	m_platformBlockModel = DirectX::Model::CreateFromSDKMESH(device, L"Resources/Models/platform_block.sdkmesh", *fx);

	// テクスチャの読み込み
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/loadingBackground.dds", nullptr, m_loadingSRV.ReleaseAndGetAddressOf()));

}

/*
* @brief　ウインドウサイズに依存するリソースを作成する関数
*
* @param[in]  なし
*
* @return     なし
*/
void LoadScene::CreateWindowSizeDependentResources()
{
	auto deviceResources = GetUserResources()->GetDeviceResources();
	auto outputSize = deviceResources->GetOutputSize();
	float aspectRatio = static_cast<float>(outputSize.right) / static_cast<float>(outputSize.bottom);

	m_proj = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(
		DirectX::XMConvertToRadians(45.0f),
		aspectRatio,
		0.1f,
		100.0f
	);
}

/*
* @brief　デバイスロストした時に呼び出される関数
*
* @param[in]  なし
*
* @return     なし
*/
void LoadScene::OnDeviceLost()
{
	Finalize();
}
