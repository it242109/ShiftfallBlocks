//--------------------------------------------------------------------------------------
// File: LoadScene.cpp
//
// ロードシーンクラス
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "LoadScene.h"

#include <WICTextureLoader.h>
#include <SimpleMath.h>

// 定数の定義
const float LoadScene::GIMMICK_BLOCK_INIT_X = 0.0f;			///< 演出用ブロックの初期X座標
const float LoadScene::GIMMICK_BLOCK_INIT_Y = 0.8f;			///< 演出用ブロックの初期Y座標
const float LoadScene::GIMMICK_BLOCK_INIT_Z = 2.0f;			///< 演出用ブロックの初期Z座標
const float LoadScene::GIMMICK_BLOCK_SCALE = 0.7f;			///< 演出用ブロックの標準の大きさ
const float LoadScene::CAMERA_UPDATE_SPEED = 2.0f;			///< カメラ更新速度の倍率
const float LoadScene::DEFAULT_ROTATION_ANGLE = 0.0f;		///< デフォルトの回転角度
const float LoadScene::DISTAICE_EPSILON_SQUARED = 0.0001f;	///< カメラとターゲットの重複を判定する微小距離の平方
const float LoadScene::CAMERA_Z_PUSH_OFFSET = 0.1f;			///< 位置重複時にカメラを後ろに押し戻すオフセット量
const float LoadScene::DEFAULT_SPRITE_ROTATION = 0.0f;		///< 背景スプライトの回転角度
const float LoadScene::BASE_SPRITE_SCALE = 1.0f;			///< 背景スプライトの基本拡大率
const float LoadScene::FIELD_OF_VIEW_DEGREES = 45.0f;		///< 視野角
const float LoadScene::NEAR_PLANE_DISTANCE = 0.1f;			///< カメラの最前面のクリップ距離
const float LoadScene::FAR_PLANE_DISTANCE = 100.0f;			///< カメラの最遠面のクリップ距離

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
	m_gimmickBlockPosition = { GIMMICK_BLOCK_INIT_X,GIMMICK_BLOCK_INIT_Y,GIMMICK_BLOCK_INIT_Z };
	m_gimmickBlockScale = { GIMMICK_BLOCK_SCALE,GIMMICK_BLOCK_SCALE,GIMMICK_BLOCK_SCALE };
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
	m_camera.Update(elapsedTime *= CAMERA_UPDATE_SPEED);
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
	DirectX::SimpleMath::Matrix rot = DirectX::SimpleMath::Matrix::CreateRotationY(DEFAULT_ROTATION_ANGLE);

	// カメラの位置とターゲットを取得
	auto eye = m_camera.GetEyePosition();
	const auto& target = m_camera.GetTargetPosition();

	if (DirectX::SimpleMath::Vector3::DistanceSquared(eye, target) < DISTAICE_EPSILON_SQUARED)
	{
		eye.z -= CAMERA_Z_PUSH_OFFSET;
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
		DirectX::Colors::White, DEFAULT_SPRITE_ROTATION, DirectX::SimpleMath::Vector2::Zero,
		ScreenManager::Scale(BASE_SPRITE_SCALE, BASE_SPRITE_SCALE));

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
		DirectX::XMConvertToRadians(FIELD_OF_VIEW_DEGREES),
		aspectRatio,
		NEAR_PLANE_DISTANCE,
		FAR_PLANE_DISTANCE
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
