//--------------------------------------------------------------------------------------
// File: SelectStage.cpp
//
// セレクトシーンクラス
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "SelectScene.h"

#include "TitleScene.h"
#include "TutorialScene.h"
#include "StageScene.h"
#include "LoadScene.h"
#include "ResultScene.h"

// 定数の定義
const float SelectScene::DEFAULT_POSITION_X = 0.0f;					///< デフォルトの位置X
const float SelectScene::DEFAULT_POSITION_Y = 0.0f;					///< デフォルトの位置Y
const float SelectScene::DEFAULT_SCALE_X = 0.7f;					///< デフォルトの大きさX
const float SelectScene::DEFAULT_SCALE_Y = 0.7f;					///< デフォルトの大きさY
const float SelectScene::SELECTED_UI_OFFSET_POSITION_X = 900.0f;	///< 選択中のUI項目を移動させるX軸のオフセット量
const int SelectScene::MAX_MENUINDEX = 4;							///< メニューインデックスの最大値

// 静的変数の実体定義
static std::string s_currentStageFilePath = "Resources/Stages/stage01.json";

/*
* @brief コンストラクタ
*
* @param[in]  なし
* 
* @return なし
*/
SelectScene::SelectScene()
	: m_lastTime_Tutorial(0),
	 m_lastTime_FirstStage(0),
	 m_lastTime_SecondStage(0),
	 m_lastTime_ThirdStage(0),
	 m_bestTime_Tutorial(0),
	 m_bestTime_FirstStage(0),
	 m_bestTime_SecondStage(0),
	 m_bestTime_ThirdStage(0),
	 m_lastTimeNumber(nullptr),
	 m_bestTimeNumber(nullptr)
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
SelectScene::~SelectScene()
{
}

/*
* @brief ステージシーンからパスを取得
*
* @param[in]  なし
*
* @return なし
*/
std::string SelectScene::GetCurrentStageFilePath()
{
	return s_currentStageFilePath;
}

/*
* @brief ステージシーンからパスを取得
*
* @param[in]  nextStage 次のステージ
*
* @return なし
*/
void SelectScene::SetCurrentStageFilePathByIndex(int stageIndex)
{
	// 条件に一致していたら何もしない
	if (stageIndex < 0 || stageIndex >= 4) return;

	// 直接JSONファイルの構造に合わせてパスを決定
	if (stageIndex == 0) s_currentStageFilePath = "Resources/Stages/tutorial.json";
	else if (stageIndex == 1) s_currentStageFilePath = "Resources/Stages/stage01.json";
	else if (stageIndex == 2) s_currentStageFilePath = "Resources/Stages/stage02.json";
	else if (stageIndex == 3) s_currentStageFilePath = "Resources/Stages/stage03.json";
}

/*
* @brief 初期化処理
*
* @param[in]  なし
* 
* @return なし
*/
void SelectScene::Initialize()
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();

	m_prevKeyboardState = {};

	// マスクの初期化
	auto transitionMask = GetUserResources()->GetTransitionMask();
	transitionMask->Open();

	// ステージのパス一覧を読み込む
	m_stageFilePaths.clear();
	std::ifstream listFile("Resources/stage_list.json");
	if (listFile.is_open())
	{
		nlohmann::json stageListData;
		listFile >> stageListData;

		// "STAGES"キーが存在するか確認し、配列であることをチェック
		if (stageListData.contains("STAGES") && stageListData["STAGES"].is_array())
		{
			// 各ステージのパスを取得してリストに追加
			for (auto& stage : stageListData["STAGES"])
			{
				if (stage.contains("data"))
				{
					m_stageFilePaths.push_back(stage["data"].get<std::string>());
				}
			}
		}
		listFile.close();
	}
	else
	{
		// ファイルが開けない場合のエラーハンドリング
		m_stageFilePaths = 
		{
			"Resources/Stages/tutorial.json",
			"Resources/Stages/stage01.json",
			"Resources/Stages/stage02.json",
			"Resources/Stages/stage03.json"
		};
	}

	// メニューの初期化
	m_selectMenu->Add(L"Resources/Textures/tutorial.png"
		, ScreenManager::Pos(DEFAULT_POSITION_X, DEFAULT_POSITION_Y)
		, ScreenManager::Scale(DEFAULT_SCALE_X, DEFAULT_SCALE_Y)
		, ANCHOR::MIDDLE_CENTER);
	m_selectMenu->Add(L"Resources/Textures/Stage1.png"
		, ScreenManager::Pos(DEFAULT_POSITION_X, DEFAULT_POSITION_Y)
		, ScreenManager::Scale(DEFAULT_SCALE_X, DEFAULT_SCALE_Y)
		, ANCHOR::MIDDLE_CENTER);
	m_selectMenu->Add(L"Resources/Textures/Stage2.png"
		, ScreenManager::Pos(DEFAULT_POSITION_X, DEFAULT_POSITION_Y)
		, ScreenManager::Scale(DEFAULT_SCALE_X, DEFAULT_SCALE_Y)
		, ANCHOR::MIDDLE_CENTER);
	m_selectMenu->Add(L"Resources/Textures/Stage3.png"
		, ScreenManager::Pos(DEFAULT_POSITION_X, DEFAULT_POSITION_Y)
		, ScreenManager::Scale(DEFAULT_SCALE_X, DEFAULT_SCALE_Y)
		, ANCHOR::MIDDLE_CENTER);
	m_selectMenu->Add(L"Resources/Textures/title.png"
		, ScreenManager::Pos(DEFAULT_POSITION_X, DEFAULT_POSITION_Y)
		, ScreenManager::Scale(DEFAULT_SCALE_X, DEFAULT_SCALE_Y)
		, ANCHOR::MIDDLE_CENTER);

	// タスクマネージャーの初期化／数字の初期化
	m_lastTimeNumber = m_taskManager.AddTask<Number>(&m_spriteBatch, m_numberSRV.GetAddressOf());
	m_lastTimeNumber->SetPosition(DirectX::SimpleMath::Vector2(550.0f, 410.0f));

	m_bestTimeNumber = m_taskManager.AddTask<Number>(&m_spriteBatch, m_numberSRV.GetAddressOf());
	m_bestTimeNumber->SetPosition(DirectX::SimpleMath::Vector2(550.0f, 470.0f));

	// 各ステージのタイムを取得
	if (m_selectMenu->m_menuIndex == 0)
	{
		m_lastTimeNumber->SetNumber(m_lastTime_Tutorial);
		m_bestTimeNumber->SetNumber(m_bestTime_Tutorial);
	}
	else if (m_selectMenu->m_menuIndex == 1)
	{
		m_lastTimeNumber->SetNumber(m_lastTime_FirstStage);
		m_bestTimeNumber->SetNumber(m_bestTime_FirstStage);
	}
	else if (m_selectMenu->m_menuIndex == 2)
	{
		m_lastTimeNumber->SetNumber(m_lastTime_SecondStage);
		m_bestTimeNumber->SetNumber(m_bestTime_SecondStage);
	}
	else if (m_selectMenu->m_menuIndex == 3)
	{
		m_lastTimeNumber->SetNumber(m_lastTime_ThirdStage);
		m_bestTimeNumber->SetNumber(m_bestTime_ThirdStage);
	}
}

/*
* @brief 更新処理
*
* @param[in]  elapsedTime 前フレームからの経過時間
* 
* @return なし
*/
void SelectScene::Update(float elapsedTime)
{
	UNREFERENCED_PARAMETER(elapsedTime);

	// キーのインプットマネージャーの更新
	InputManager::Get().Update();

	// キー入力の取得
	DirectX::Keyboard::State keystate = DirectX::Keyboard::Get().GetState();

	// マスクの取得
	auto transitionMask = GetUserResources()->GetTransitionMask();

	// サウンドマネージャーの更新
	auto& sound = SoundManager::GetInstance();

	// メニューの更新
	m_selectMenu->Update();

	// 下方向の制限
	if (InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Down)
		|| InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::S))
	{
		sound.Play(L"SELECT");
		// 進ませないように固定する
		if (m_selectMenu->m_menuIndex == 0) m_selectMenu->m_menuIndex = MAX_MENUINDEX;
	}
	// 上方向の制限
	if (InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Up)
		|| InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::W))
	{
        sound.Play(L"SELECT");
		// 戻らせないように固定する
		if (m_selectMenu->m_menuIndex >= MAX_MENUINDEX) m_selectMenu->m_menuIndex = 0;
	}

	// 選択中のインデックスを直接取得
	int selectedIndex = m_selectMenu->m_menuIndex;

	// ボタンのデフォルトの座標
	DirectX::SimpleMath::Vector2 positions[5] = 
	{
			{ -250.0f, 320.0f },
			{ -250.0f, 320.0f },
			{ -250.0f, 320.0f },
			{ -250.0f, 320.0f },
			{ -250.0f, 400.0f }
	};

	// 配列から要素数を自動計算
	const size_t menuCount = std::size(positions);

	// 選択中のものだけずらす
	positions[selectedIndex].x += SELECTED_UI_OFFSET_POSITION_X;

	// 計算した要素数を使ってループを回す
	for (int i = 0; i < menuCount; i++)
	{
		m_selectMenu->SetPosition(static_cast<int>(i), ScreenManager::Pos(positions[i].x, positions[i].y));
	}

	// メニューの更新
    m_selectMenu->Update();

	// 各ステージのラストタイムを取得
	m_lastTime_Tutorial = static_cast<int>(ResultScene::GetLastTimeFronJson(ResultScene::ResultStage::TUTORIAL));
	m_lastTime_FirstStage = static_cast<int>(ResultScene::GetLastTimeFronJson(ResultScene::ResultStage::FIRST));
	m_lastTime_SecondStage = static_cast<int>(ResultScene::GetLastTimeFronJson(ResultScene::ResultStage::SECOND));
	m_lastTime_ThirdStage = static_cast<int>(ResultScene::GetLastTimeFronJson(ResultScene::ResultStage::THIRD));

	// 各ステージのベストタイムを取得
	m_bestTime_Tutorial = static_cast<int>(ResultScene::GetBestTimeFromJson(ResultScene::ResultStage::TUTORIAL));
	m_bestTime_FirstStage = static_cast<int>(ResultScene::GetBestTimeFromJson(ResultScene::ResultStage::FIRST));
	m_bestTime_SecondStage = static_cast<int>(ResultScene::GetBestTimeFromJson(ResultScene::ResultStage::SECOND));
	m_bestTime_ThirdStage = static_cast<int>(ResultScene::GetBestTimeFromJson(ResultScene::ResultStage::THIRD));

	// ラストタイム・ベストタイムの表示
	if (m_selectMenu->m_menuIndex == 0)
	{
		// 数字のセット／更新
		m_lastTimeNumber->SetTimer(static_cast<int>(m_lastTime_Tutorial));
		m_bestTimeNumber->SetTimer(static_cast<int>(m_bestTime_Tutorial));
	}
	else if (m_selectMenu->m_menuIndex == 1)
	{
		m_lastTimeNumber->SetTimer(static_cast<int>(m_lastTime_FirstStage));
		m_bestTimeNumber->SetTimer(static_cast<int>(m_bestTime_FirstStage));
	}
	else if (m_selectMenu->m_menuIndex == 2)
	{
		m_lastTimeNumber->SetTimer(static_cast<int>(m_lastTime_SecondStage));
		m_bestTimeNumber->SetTimer(static_cast<int>(m_bestTime_SecondStage));
	}
	else if (m_selectMenu->m_menuIndex == 3)
	{
		m_lastTimeNumber->SetTimer(static_cast<int>(m_lastTime_ThirdStage));
		m_bestTimeNumber->SetTimer(static_cast<int>(m_bestTime_ThirdStage));
	}

	// 数字の更新
	m_lastTimeNumber->Update(elapsedTime);
	m_bestTimeNumber->Update(elapsedTime);

	if (m_selectMenu->m_menuIndex < MAX_MENUINDEX)
	{
		// タスクマネージャーの更新処理
		m_taskManager.Update(elapsedTime);
	}

	// シーン切り替え：チュートリアルステージ
	if (m_selectMenu->m_menuIndex == 0 && InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Space))
	{
		sound.Play(L"DECISION");
		SoundManager::GetInstance().Stop(L"TITLEBGM");

		if (!m_isClosingTutorial)
		{
			ResultScene::SetGlobalStage(ResultScene::ResultStage::TUTORIAL);

			transitionMask->Close(); // フェードアウト
			m_isClosingTutorial = true;
		}
	}
	if (m_isClosingTutorial && transitionMask->IsEnd())
	{
		// フェードアウト完了後にセレクトシーンへ遷移
		m_isClosingTutorial = false;
		ChangeLoadingScene<TutorialScene, LoadScene>();
	}

	// ステージ1へ遷移
	if (m_selectMenu->m_menuIndex == 1 && InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Space))
	{
		sound.Play(L"DECISION");
		SoundManager::GetInstance().Stop(L"TITLEBGM");

		if (!m_isClosingFirstStage)
		{
			if (1 < m_stageFilePaths.size())
			{
				s_currentStageFilePath = m_stageFilePaths[1];
			}

			ResultScene::SetGlobalStage(ResultScene::ResultStage::FIRST);

			transitionMask->Close();
			m_isClosingFirstStage = true;
		}
	}
	if (m_isClosingFirstStage && transitionMask->IsEnd())
	{
		m_isClosingFirstStage = false;
		ChangeLoadingScene<StageScene, LoadScene>();
	}

	// ステージ2へ遷移
	if (m_selectMenu->m_menuIndex == 2 && InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Space))
	{
		sound.Play(L"DECISION");
		SoundManager::GetInstance().Stop(L"TITLEBGM");

		if (!m_isClosingSecondStage)
		{
			if (2 < m_stageFilePaths.size())
			{
				s_currentStageFilePath = m_stageFilePaths[2];
			}
			ResultScene::SetGlobalStage(ResultScene::ResultStage::SECOND);

			transitionMask->Close();
			m_isClosingSecondStage = true;
		}
	}
	if (m_isClosingSecondStage && transitionMask->IsEnd())
	{
		m_isClosingSecondStage = false;
		ChangeLoadingScene<StageScene, LoadScene>();
	}

	// ステージ3へ遷移
	if (m_selectMenu->m_menuIndex == 3 && InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Space))
	{
		sound.Play(L"DECISION");
		SoundManager::GetInstance().Stop(L"TITLEBGM");

		if (!m_isClosingThirdStage)
		{
			if (3 < m_stageFilePaths.size())
			{
				s_currentStageFilePath = m_stageFilePaths[3];
			}
			ResultScene::SetGlobalStage(ResultScene::ResultStage::THIRD);

			transitionMask->Close();
			m_isClosingThirdStage = true;
		}
	}
	if (m_isClosingThirdStage && transitionMask->IsEnd())
	{
		m_isClosingThirdStage = false;
		ChangeLoadingScene<StageScene, LoadScene>();
	}

	// タイトルシーンへ遷移
	if (m_selectMenu->m_menuIndex == 4 && InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Space))
	{
		sound.Play(L"DECISION");

		if (!m_isClosingTitle)
		{
			transitionMask->Close();
			m_isClosingTitle = true;
		}
	}
	if (m_isClosingTitle && transitionMask->IsEnd())
	{
		m_isClosingTitle = false;
		ChangeScene<TitleScene>();
	}
}

/*
* @brief　描画処理
*
* @param[in]  なし
* 
* @return なし
*/
void SelectScene::Render()
{
	auto* debugFont = GetUserResources()->GetDebugFont();

	// 背景の描画
	m_background->Render();

	m_spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, GetUserResources()->GetCommonStates()->NonPremultiplied());

	m_spriteBatch->Draw(m_selectSRV.Get(), ScreenManager::Pos(230.0f, 180.0f), nullptr,
		DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
		ScreenManager::Scale(1.0f, 0.8f));

	// タスクマネージャーの描画処理
	if (m_selectMenu->m_menuIndex < MAX_MENUINDEX)
	{
		m_taskManager.Render();
	}
	m_spriteBatch->Draw(m_selectFontSRV.Get(), ScreenManager::Pos(370.0f, 35.0f), nullptr,
		DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
		ScreenManager::Scale(1.5f, 1.5f));

	// ラストタイム・ベストタイムの描画
	if (m_selectMenu->m_menuIndex < MAX_MENUINDEX)
	{
		m_spriteBatch->Draw(m_lastTimeSRV.Get(), ScreenManager::Pos(370.0f, 400.0f), nullptr,
			DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
			ScreenManager::Scale(0.8f, 0.8f));
		m_spriteBatch->Draw(m_bestTimeSRV.Get(), ScreenManager::Pos(370.0f, 450.0f), nullptr,
			DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
			ScreenManager::Scale(0.8f, 0.8f));
		m_spriteBatch->Draw(m_downSRV.Get(), DirectX::SimpleMath::Vector2(ScreenManager::Pos(620.0f, 580.0f)));

	}
	if (m_selectMenu->m_menuIndex > 0)
		m_spriteBatch->Draw(m_upSRV.Get(), DirectX::SimpleMath::Vector2(ScreenManager::Pos(620.0f, 180.0f)));

	// 矢印の描画
	m_spriteBatch->Draw(m_selectKeySRV.Get(), ScreenManager::Pos(40.0f, 640.0f), nullptr,
		DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
		ScreenManager::Scale(1.0f, 1.0f));

	m_spriteBatch->End();

	// メニューの描画
	m_selectMenu->Render();

	debugFont->Render(GetUserResources()->GetCommonStates());
}

/*
* @brief　終了処理
*
* @param[in]  なし
* 
* @return なし
*/
void SelectScene::Finalize()
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
void SelectScene::CreateDeviceDependentResources()
{
	auto deviceResources = GetUserResources()->GetDeviceResources();
	auto device = deviceResources->GetD3DDevice();
	auto context = deviceResources->GetD3DDeviceContext();

	// スプライトバッチの作成
	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(context);

	// 背景の作成
	m_background = std::make_unique<BackGround>();
	m_background->Create(deviceResources);

	// メニューのインスタンス作成
	m_selectMenu = std::make_unique<Menu>();

	// メニューの初期化処理
	D3D11_VIEWPORT windowInfo = deviceResources->GetScreenViewport();
	m_selectMenu->Initialize(
		deviceResources,
		(int)windowInfo.Width,
		(int)windowInfo.Height);

	// テクスチャの読み込み
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/down.dds", nullptr, m_downSRV.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/up.dds", nullptr, m_upSRV.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/selectPlate.dds", nullptr, m_selectSRV.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/select.dds", nullptr, m_selectFontSRV.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/clearTimeFont.dds", nullptr, m_lastTimeSRV.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/bestTimeFont.dds", nullptr, m_bestTimeSRV.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/number.dds", nullptr, m_numberSRV.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/selectKey.dds", nullptr, m_selectKeySRV.ReleaseAndGetAddressOf()));
}

/*
* @brief　ウインドウサイズに依存するリソースを作成する関数
*
* @param[in]  なし
* 
* @return なし
*/
void SelectScene::CreateWindowSizeDependentResources()
{
	auto deviceResources = GetUserResources()->GetDeviceResources();
	D3D11_VIEWPORT windowInfo = deviceResources->GetScreenViewport();

	ScreenManager::SetDeviceResources(deviceResources);

	if (m_selectMenu)
	{
		m_selectMenu->Initialize(
			deviceResources,
			(int)windowInfo.Width,
			(int)windowInfo.Height);
	}
}

/*
* @brief　デバイスロストした時に呼び出される関数
*
* @param[in]  なし
* 
* @return なし
*/
void SelectScene::OnDeviceLost()
{
	m_spriteBatch.reset();
}