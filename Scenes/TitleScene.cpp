//--------------------------------------------------------------------------------------
// File: TitleScene.cpp
//
// タイトルシーンクラス
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "TitleScene.h"
#include "SelectScene.h"
#include "LoadScene.h"

// 定数の定義
const DirectX::SimpleMath::Vector3 TitleScene::TITLE_PLAYER_POSITION = { 0.0f,1.0f,-2.0f };	///< タイトルシーンのプレイヤーの位置
const DirectX::SimpleMath::Vector3 TitleScene::TITLE_PLAYER_SCALE = { 1.0f,1.5f,1.0f };		///< タイトルシーンのプレイヤーの大きさ
const DirectX::SimpleMath::Vector3 TitleScene::GIMMICKBLOCK_POSITION = { 0.0f,0.8f,2.0f };	///< 仕掛けブロックの位置
const DirectX::SimpleMath::Vector3 TitleScene::GIMMICKBLOCK_SCALE = { 0.7f,0.7f,0.7f };		///< 仕掛けブロックの大きさ
const DirectX::SimpleMath::Vector3 TitleScene::FLOOR_POSITION = { 0.0f,0.0f,0.0f };			///< 床の位置
const DirectX::SimpleMath::Vector3 TitleScene::FLOOR_SCALE = { 20.0f,1.0f,20.0f };			///< 床の大きさ

const float TitleScene::WALL_OFFSET_DIST = 15.0f;	///<　壁オブジェクトの中心からの距離
const float TitleScene::WALL_THICKNESS = 10.0f;		///< 壁の厚み
const float TitleScene::WALL_HEIGHT = 30.0f;		///< 壁の高さ
const float TitleScene::WALL_LENGTH = 20.0f;		///< 壁の長さ

const float TitleScene::MENU_DEFAULT_POSITION_X = 1000.0f;	///< メニューのデフォルトの位置X
const float TitleScene::MENU_DEFAULT_SCALE_X = 0.8f;		///< メニューのデフォルトの大きさX
const float TitleScene::MENU_DEFAULT_SCALE_Y = 0.8f;		///< メニューのデフォルトの大きさY

const float TitleScene::FIELD_OF_VIEW_DEGREES = 45.0f;	///< 視野角
const float TitleScene::NEAR_PLANE_DISTANCE = 0.1f;		///< カメラの最前面のクリップ距離
const float TitleScene::FAR_PLANE_DISTANCE = 100.0f;	///< カメラの最遠面のクリップ距離

/*
* @brief コンストラクタ
*
* @param[in]  なし
* 
* @return なし
*/
TitleScene::TitleScene()
{
	// BGMの再生
	SoundManager::GetInstance().PlayLoop(L"TITLEBGM", 1);
}

/*
* @brief デストラクタ
*
* @param[in]  なし
* 
* @return なし
*/
TitleScene::~TitleScene()
{
}

/*
* @brief 初期化処理
*
* @param[in]  なし
* 
* @return なし
*/
void TitleScene::Initialize()
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();

	m_prevKeyboardState = {};

	// マスクの初期化
	auto transitionMask = GetUserResources()->GetTransitionMask();
	transitionMask->Open();

	// プレイヤーの初期化
	m_playerPosition = TITLE_PLAYER_POSITION;
	m_playerScale = TITLE_PLAYER_SCALE;

	// 仕掛けブロックの初期化
	m_gimmickBlockPosition = GIMMICKBLOCK_POSITION;
	m_gimmickBlockScale = GIMMICKBLOCK_SCALE;

	// 床の初期化
	m_floorPosition = FLOOR_POSITION;
	m_floorScale = FLOOR_SCALE;
	m_floorRotate = DirectX::SimpleMath::Quaternion::Identity;

	// 壁の初期化
	m_wallPositions.clear();
	m_wallScales.clear();
	m_wallPositions =
	{
		{-WALL_OFFSET_DIST,0.0f,0.0f},
		{WALL_OFFSET_DIST,0.0f,0.0f},
		{0.0f,0.0f,-WALL_OFFSET_DIST},
		{0.0f,0.0f,WALL_OFFSET_DIST}
	};
	m_wallScales =
	{
		{WALL_THICKNESS,WALL_HEIGHT,WALL_LENGTH},
		{WALL_THICKNESS,WALL_HEIGHT,WALL_LENGTH},
		{WALL_LENGTH,WALL_HEIGHT,WALL_THICKNESS},
		{WALL_LENGTH,WALL_HEIGHT,WALL_THICKNESS}
	};

	// カメラの初期化(床に合わせる)
	m_camera.SetPlayer(m_floorPosition, m_floorRotate);

	// メニューの初期化
	m_titleMenu->Add(L"Resources/Textures/start.png"
		, ScreenManager::Pos(MENU_DEFAULT_POSITION_X, 350.0f)
		, ScreenManager::Scale(MENU_DEFAULT_SCALE_X, MENU_DEFAULT_SCALE_Y)
		, ANCHOR::MIDDLE_CENTER);
	m_titleMenu->Add(L"Resources/Textures/exit.png"
		, ScreenManager::Pos(MENU_DEFAULT_POSITION_X,550.0f)
		, ScreenManager::Scale(MENU_DEFAULT_SCALE_X, MENU_DEFAULT_SCALE_Y)
		, ANCHOR::MIDDLE_CENTER);
}

/*
* @brief 更新処理
*
* @param[in]  elapsedTime 前フレームからの経過時間
* 
* @return なし
*/
void TitleScene::Update(float elapsedTime)
{
	UNREFERENCED_PARAMETER(elapsedTime);

	// キー入力の取得
	DirectX::Keyboard::State keystate = DirectX::Keyboard::Get().GetState();
	InputManager::Get().Update();

	// メニューの更新
	m_titleMenu->Update();

	auto& sound = SoundManager::GetInstance();

	if (InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Up)
		|| InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::W))
	{
		sound.Play(L"SELECT");
	}
	else if (InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Down)
			|| InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::S))
	{
		sound.Play(L"SELECT");
	}

	// カメラの更新
	m_camera.Update(elapsedTime);

	// マスクの取得
	auto transitionMask = GetUserResources()->GetTransitionMask();

	// フェードアウト開始
	if (InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Space))
	{
		sound.Play(L"DECISION");

		if (!m_isClosing) 
		{
			transitionMask->Close();
			m_isClosing = true;
		}
	}
	// フェードアウト完了後にセレクトシーンへ遷移
	if (m_isClosing && transitionMask->IsEnd())
	{
		ChangeScene<SelectScene>();
		return;
	}
	// 終了するキー
	if (m_titleMenu->m_menuIndex == 1 && InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Space)) 
	{
		PostQuitMessage(0);
	}
}

/*
* @brief　描画処理
*
* @param[in]  なし
* 
* @return なし
*/
void TitleScene::Render()
{
	auto context = GetUserResources()->GetDeviceResources()->GetD3DDeviceContext();
	auto states = GetUserResources()->GetCommonStates();
	// デフォルトのマトリックス
	DirectX::SimpleMath::Matrix rot = DirectX::SimpleMath::Matrix::CreateRotationY(0.0f);

	// ビューを回るカメラに固定
	m_view = DirectX::SimpleMath::Matrix::CreateLookAt(
		m_camera.GetEyePosition(),
		m_camera.GetTargetPosition(),
		DirectX::SimpleMath::Vector3::UnitY
	);

	// プレイヤーの描画
	DirectX::SimpleMath::Matrix player_s = DirectX::SimpleMath::Matrix::CreateScale(m_playerScale);
	DirectX::SimpleMath::Matrix player_t = DirectX::SimpleMath::Matrix::CreateTranslation(m_playerPosition);
	DirectX::SimpleMath::Matrix playerWorld = player_s * rot * player_t;
	m_playerModel->Draw(context, *states, playerWorld, m_view, m_proj);

	// 仕掛けブロックの描画
	DirectX::SimpleMath::Matrix gimmick_s = DirectX::SimpleMath::Matrix::CreateScale(m_gimmickBlockScale);
	DirectX::SimpleMath::Matrix gimmick_t = DirectX::SimpleMath::Matrix::CreateTranslation(m_gimmickBlockPosition);
	DirectX::SimpleMath::Matrix gimmickWorld = gimmick_s * rot * gimmick_t;
	m_gimmickBlockModel->Draw(context, *states, gimmickWorld, m_view, m_proj);

	// 床の描画
	DirectX::SimpleMath::Matrix floor_s = DirectX::SimpleMath::Matrix::CreateScale(m_floorScale);
	DirectX::SimpleMath::Matrix floor_t = DirectX::SimpleMath::Matrix::CreateTranslation(m_floorPosition);
	DirectX::SimpleMath::Matrix floorWorld = floor_s * rot * floor_t;
	m_floorModel->Draw(context, *states, floorWorld, m_view, m_proj);

	// 壁の描画
	for (size_t i = 0; i < m_wallPositions.size(); ++i)
	{
		DirectX::SimpleMath::Matrix wall_s = DirectX::SimpleMath::Matrix::CreateScale(m_wallScales[i]);
		DirectX::SimpleMath::Matrix wall_t = DirectX::SimpleMath::Matrix::CreateTranslation(m_wallPositions[i]);
		DirectX::SimpleMath::Matrix wallWorld = wall_s * rot * wall_t;

		m_wallModel->Draw(context, *states, wallWorld, m_view, m_proj);
	}

	m_spriteBatch->Begin();

	//　画像の描画
	m_spriteBatch->Draw(m_titleLogo.Get(), ScreenManager::Pos(100.0f, 30.0f), nullptr,
		DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
		ScreenManager::Scale(1.0f, 1.0f));
	m_spriteBatch->Draw(m_selectKey.Get(), ScreenManager::Pos(40.0f, 640.0f), nullptr,
		DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
		ScreenManager::Scale(1.0f, 1.0f));

	m_spriteBatch->End();

	// メニューの描画
	m_titleMenu->Render();

	// フェードしたら画面を塗りつぶす
	auto transitionMask = GetUserResources()->GetTransitionMask();
	if (m_isClosing && transitionMask->IsEnd())
	{
		return;
	}
}

/*
* @brief　終了処理
*
* @param[in]  なし
* 
* @return なし
*/
void TitleScene::Finalize()
{
	OnDeviceLost();
}

/*
* @brief　デバイスに依存するリソースを作成する関数
*
* @param[in]  なし
* 
* @return なし
*/
void TitleScene::CreateDeviceDependentResources()
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
	m_floorModel = DirectX::Model::CreateFromSDKMESH(device, L"Resources/Models/floor.sdkmesh", *fx);
	m_wallModel = DirectX::Model::CreateFromSDKMESH(device, L"Resources/Models/wall.sdkmesh", *fx);
	m_playerModel = DirectX::Model::CreateFromSDKMESH(device, L"Resources/Models/player.sdkmesh", *fx);
	m_gimmickBlockModel = DirectX::Model::CreateFromSDKMESH(device, L"Resources/Models/platform_block.sdkmesh", *fx);

	// テクスチャの読み込み
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/titleLogo.dds", nullptr, m_titleLogo.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/selectKey.dds", nullptr, m_selectKey.ReleaseAndGetAddressOf()));

	// メニューのインスタンス作成
	m_titleMenu = std::make_unique<Menu>();

	// メニューの初期化処理
	D3D11_VIEWPORT windowInfo = deviceResources->GetScreenViewport();
	m_titleMenu->Initialize(
		deviceResources,
		(int)windowInfo.Width,
		(int)windowInfo.Height);
}

/*
* @brief　ウインドウサイズに依存するリソースを作成する関数
*
* @param[in]  なし
* 
* @return なし
*/
void TitleScene::CreateWindowSizeDependentResources()
{
	auto deviceResources = GetUserResources()->GetDeviceResources();
	auto outputSize = deviceResources->GetOutputSize();
	D3D11_VIEWPORT windowInfo = deviceResources->GetScreenViewport();
	float aspectRatio = static_cast<float>(outputSize.right) / static_cast<float>(outputSize.bottom);

	ScreenManager::SetDeviceResources(deviceResources);

	if (m_titleMenu)
	{
		m_titleMenu->Initialize(
			deviceResources,
			(int)windowInfo.Width,
			(int)windowInfo.Height);
	}

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
* @return なし
*/
void TitleScene::OnDeviceLost()
{
	m_spriteBatch.reset();
}