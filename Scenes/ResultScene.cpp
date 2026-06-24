//--------------------------------------------------------------------------------------
// File: ResultScene.cpp
//
// リザルトシーンクラス
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "ResultScene.h"
#include "LoadScene.h"

ResultScene::ResultType ResultScene::s_globalResult = ResultScene::ResultType::NONE;
ResultScene::ResultStage ResultScene::s_globalStage = ResultScene::ResultStage::NONE;

float ResultScene::s_currentClearTime = 0.0f;

// 保存ファイル名
const std::string LASTTIME_FILENAME = "Resources/last_times.json";
const std::string BESTTIME_FILENAME = "Resources/best_times.json";

// 定数の定義
const int ResultScene::SECONDS_IN_HOUR = 3600;		///< １時間あたりの秒数
const int ResultScene::SECONDS_IN_MIN = 60;			///< １分あたりの秒数
const int ResultScene::PAD_WIDTH = 2;				///< 桁そろえの文字幅
const int ResultScene::LOG_COMUMN_WIDTH = 4;		///< 出力時の桁数指定（4ケタ）
const float ResultScene::PARSE_ERROR_FALLBACK = 0.0f;///< 解析失敗時のフォールバック値

/*
* @brief 文字列に変換
*
* @param[in]  stage ステージタイプ
* 
* @return ステージ名の文字列
*/
std::string ResultScene::StageEnumToString(ResultStage stage)
{
	// ステージごとに分岐
	switch (stage)
	{
	case ResultStage::TUTORIAL:
		return "TUTORIAL";
	case ResultStage::FIRST:
		return "STAGE_1";
	case ResultStage::SECOND:
		return "STAGE_2";
	case ResultStage::THIRD:
		return "STAGE_3";
	default:
		return "UNKNOWN";
	}
}

/*
* @brief コンストラクタ
*
* @param[in]  なし
* 
* @return     なし
*/
ResultScene::ResultScene()
	: m_result(ResultType::NONE)
	, m_stage(ResultStage::NONE)
	, m_clearTimeNumber{}
	, m_bestTimeNumber{}
	, m_clearTime(0)
	, m_bestTime(0)
{
}

/*
* @brief デストラクタ
*
* @param[in]  なし
* 
* @return     なし
*/
ResultScene::~ResultScene()
{
}

/*
* @brief 初期化処理
*
* @param[in]  なし
* 
* @return     なし
*/
void ResultScene::Initialize()
{
	OutputDebugStringA("ResultScene::Initialize called\n");

	// グローバル変数から結果とステージを取得
	m_result = s_globalResult;
	m_stage = GetGlobalStage();

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();

	// メニューの初期化
	m_menu->m_menuIndex = 0;
	if (m_result == ResultType::CLEAR && m_stage != ResultStage::THIRD)
	{
		// 最終ステージ以外の画面での次のステージ、リトライ、セレクト
		m_menu->Add(L"Resources/Textures/nextStage.png", ScreenManager::Pos(600, 350), ScreenManager::Scale(0.8f, 0.8f), ANCHOR::MIDDLE_CENTER);
		m_menu->Add(L"Resources/Textures/retry.png", ScreenManager::Pos(600, 500), ScreenManager::Scale(0.8f, 0.8f), ANCHOR::MIDDLE_CENTER);
		m_menu->Add(L"Resources/Textures/select.png", ScreenManager::Pos(600, 650), ScreenManager::Scale(0.8f, 0.8f), ANCHOR::MIDDLE_CENTER);
	}
	else
	{
		// 最終ステージクリアまたはゲームオーバー画面でのリトライ、セレクトのみ
		m_menu->Add(L"Resources/Textures/retry.png", ScreenManager::Pos(600, 400), ScreenManager::Scale(0.8f, 0.8f), ANCHOR::MIDDLE_CENTER);
		m_menu->Add(L"Resources/Textures/select.png", ScreenManager::Pos(600, 600), ScreenManager::Scale(0.8f, 0.8f), ANCHOR::MIDDLE_CENTER);
	}

	m_prevKeyboardState = {};

	// タスクマネージャーの初期化／数字の初期化
	m_clearTimeNumber = m_taskManager.AddTask<Number>(&m_spriteBatch, m_numberSRV.GetAddressOf());
	m_clearTimeNumber->SetNumber(m_clearTime);
	m_clearTimeNumber->SetPosition(DirectX::SimpleMath::Vector2(400.0f, 210.0f));
	m_bestTimeNumber = m_taskManager.AddTask<Number>(&m_spriteBatch, m_numberSRV.GetAddressOf());
	m_bestTimeNumber->SetNumber(m_bestTime);
	m_bestTimeNumber->SetPosition(DirectX::SimpleMath::Vector2(950.0f, 210.0f));
}

/*
* @brief　更新処理
*
* @param[in]  elapsedTime 全フレームからの経過時間
* 
* @return     なし
*/
void ResultScene::Update(float elapsedTime)
{
	UNREFERENCED_PARAMETER(elapsedTime);

	// キー入力の取得
	InputManager::Get().Update();

	// サウンドマネージャーの更新
	auto& sound = SoundManager::GetInstance();

	// メニューの更新
	m_menu->Update();

	// クリア時のみタイム更新処理を行う
	if (m_result == ResultType::CLEAR)
	{
		// 前回のタイム
		UpdateLastTime();
		// ベストタイム
		UpdateBestTime(); 
	}

	m_clearTime = static_cast<int>(s_currentClearTime);
	m_bestTime = static_cast<int>(GetBestTimeFromJson(m_stage));

	// 数字のセット／更新
	m_clearTimeNumber->SetTimer(static_cast<int>(m_clearTime));
	m_clearTimeNumber->Update(elapsedTime);
	m_bestTimeNumber->SetTimer(static_cast<int>(m_bestTime));
	m_bestTimeNumber->Update(elapsedTime);

	// タスクマネージャーの更新処理
	m_taskManager.Update(elapsedTime);

	// メニューの最大インデックス
	// デフォルト：ボタン2つ (0, 1)
	unsigned int maxIndex = 1;
	if (m_result == ResultType::CLEAR && m_stage != ResultStage::THIRD)
	{
		maxIndex = 2;
	}

	// 現在のインデックスが最大を超えないように調整
	if (m_menu->m_menuIndex > maxIndex)
		m_menu->m_menuIndex = 0;

	// 上下キー選択
	if (InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Down) ||
		InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::S))
	{
		SoundManager::GetInstance().Play(L"SELECT");
		if (m_menu->m_menuIndex > maxIndex) m_menu->m_menuIndex = 0;
	}
	else if (InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Up) ||
		InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::W))
	{
		SoundManager::GetInstance().Play(L"SELECT");
		if (m_menu->m_menuIndex < 0) m_menu->m_menuIndex = maxIndex;
	}

	// インデックスのループ処理
	if (m_menu->m_menuIndex > maxIndex) m_menu->m_menuIndex = 0;
	if (m_menu->m_menuIndex < 0) m_menu->m_menuIndex = maxIndex;

	// 決定処理
	if (InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Space))
	{
		SoundManager::GetInstance().Play(L"DECISION");

		// クリア時の遷移
		if (m_result == ResultType::CLEAR)
		{
			sound.Stop(L"CLEAR");

			if (m_stage == ResultStage::THIRD)
			{

				// ボタン構成 (0:リトライ, 1:セレクト)
				if (m_menu->m_menuIndex == 0)
				{
					ResultScene::SetGlobalStage(ResultStage::THIRD);
					ChangeLoadingScene<StageScene, LoadScene>();
				}
				else ChangeScene<SelectScene>();
				return;
			}
			else
			{
				// ボタン構成 (0:次へ, 1:リトライ, 2:セレクト)
				if (m_menu->m_menuIndex == 0) 
				{
					// 次へ
					if (m_stage == ResultStage::TUTORIAL) ResultScene::SetGlobalStage(ResultStage::FIRST);
					else if (m_stage == ResultStage::FIRST) ResultScene::SetGlobalStage(ResultStage::SECOND);
					else if (m_stage == ResultStage::SECOND) ResultScene::SetGlobalStage(ResultStage::THIRD);
				
					ChangeLoadingScene<StageScene, LoadScene>();
				}
				else if (m_menu->m_menuIndex == 1) 
				{
					// リトライ処理
					if (m_stage == ResultStage::TUTORIAL)
					{
						// チュートリアルなら専用のシーンへ
						ChangeLoadingScene<TutorialScene, LoadScene>();
					}
					else
					{
						// それ以外なら現在のステージを予約してリトライ
						ResultScene::SetGlobalStage(m_stage);
						ChangeLoadingScene<StageScene, LoadScene>();
					}
				}
				else 
				{
					// セレクト
					ChangeScene<SelectScene>();
				}
				return;
			}
		}
		// ゲームオーバー時の遷移
		else if (m_result == ResultType::GAMEOVER)
		{
			// ボタン構成 (0:リトライ, 1:セレクト)
			if (m_menu->m_menuIndex == 0) 
			{
				// 現在のステージをリトライ予約
				ResultScene::SetGlobalStage(m_stage);
				ChangeLoadingScene<StageScene, LoadScene>();
			}
			else 
			{
				// セレクトシーンへ
				ChangeScene<SelectScene>();
			}
			return;
		}
	}
}

/*
* @brief 描画処理
*
* @param[in]  なし
* 
* @return     なし
*/
void ResultScene::Render()
{
	auto* debugFont = GetUserResources()->GetDebugFont();

	// 背景の描画
	m_background->Render();

	switch (m_result)
	{
		// クリア時の描画処理
		case ResultType::CLEAR:
		{
			m_spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, GetUserResources()->GetCommonStates()->NonPremultiplied());

			// タスクマネージャーの描画処理
			m_taskManager.Render();

			m_spriteBatch->Draw(m_timeFont.Get(), ScreenManager::Pos(160.0f, 180.0f), nullptr,
				DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
				ScreenManager::Scale(1.0f, 1.0f));
			m_spriteBatch->Draw(m_bestTimeFont.Get(), ScreenManager::Pos(700.0f, 180.0f), nullptr,
				DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
				ScreenManager::Scale(1.0f, 1.0f));
			m_spriteBatch->Draw(m_selectKey.Get(), ScreenManager::Pos(40.0f, 640.0f), nullptr,
				DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
				ScreenManager::Scale(1.0f, 1.0f));

			// ステージ名の描画
			switch (m_stage)
			{
			case ResultStage::TUTORIAL:
				m_spriteBatch->Draw(m_tutorialstageFont.Get(), ScreenManager::Pos(300.0f, 60.0f), nullptr,
					DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
					ScreenManager::Scale(1.0f, 1.0f));
				break;
			case ResultStage::FIRST:
				m_spriteBatch->Draw(m_firststageFont.Get(), ScreenManager::Pos(370.0f, 60.0f), nullptr,
					DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
					ScreenManager::Scale(1.0f, 1.0f));
				break;
			case ResultStage::SECOND:
				m_spriteBatch->Draw(m_secondstageFont.Get(), ScreenManager::Pos(370.0f, 60.0f), nullptr,
					DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
					ScreenManager::Scale(1.0f, 1.0f));
				break;
			case ResultStage::THIRD:
				m_spriteBatch->Draw(m_thirdstageFont.Get(), ScreenManager::Pos(370.0f, 60.0f), nullptr,
					DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
					ScreenManager::Scale(1.0f, 1.0f));
				break;
			case ResultStage::NONE:
				break;
			}
			m_spriteBatch->Draw(m_clearFont.Get(), ScreenManager::Pos(800.0f, 60.0f), nullptr,
				DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
				ScreenManager::Scale(1.0f, 1.0f));

			m_spriteBatch->End();

			// メニューの描画
			m_menu->Render();
			break;
		}
		// ゲームオーバー時の描画処理
		case ResultType::GAMEOVER:
		{
			m_spriteBatch->Begin();
			m_spriteBatch->Draw(m_gameoverFont.Get(), ScreenManager::Pos(270.0f, 130.0f), nullptr,
				DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
				ScreenManager::Scale(1.0f, 1.0f));
			m_spriteBatch->Draw(m_selectKey.Get(), ScreenManager::Pos(40.0f, 640.0f), nullptr,
				DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
				ScreenManager::Scale(1.0f, 1.0f));
			m_spriteBatch->End();

			// メニューの描画
			m_menu->Render();
			break;
		}
		case ResultType::NONE:
			break;
	}
	debugFont->Render(GetUserResources()->GetCommonStates());
}

/*
* @brief　終了処理
*
* @param[in]  なし
* 
* @return     なし
*/
void ResultScene::Finalize()
{
	OnDeviceLost();
}

/*
* @brief　秒から分秒表記に変換する関数
*
* @param[in]  timeSec 秒数
* 
* @return   "hh:mm:ss"形式の文字列
*/
std::string ResultScene::FormatTimeHMS(float timeSec)
{
	// 秒数を整数に変換
	int total = static_cast<int>(timeSec);

	// 時間、分、秒を計算
	int hours = total / SECONDS_IN_HOUR;
	int minutes = (total % SECONDS_IN_HOUR) / SECONDS_IN_MIN;
	int seconds = total % SECONDS_IN_MIN;

	// "hh:mm:ss"形式の文字列を作成
	std::ostringstream oss;
	oss << std::setw(PAD_WIDTH) << std::setfill('0') << hours << ":"
		<< std::setw(PAD_WIDTH) << std::setfill('0') << minutes << ":"
		<< std::setw(PAD_WIDTH) << std::setfill('0') << seconds;

	return oss.str();
}

/*
* @brief　分秒表記から秒に戻す関数
*
* @param[in] timeStr　"hh:mm:ss"形式の文字列
*
* @return	 秒数
*/
float ResultScene::ParseTimeHMS(const std::string& timeStr)
{
	// "hh:mm:ss"形式の文字列を分割して時間、分、秒を取得
	int h, m, s;
	char c1, c2;

	// 文字列を解析して時間、分、秒を取得
	std::istringstream iss(timeStr);
	iss >> h >> c1 >> m >> c2 >> s;

	// 解析できなかった場合
	if (iss.fail()) return PARSE_ERROR_FALLBACK;

	return static_cast<float>(h * SECONDS_IN_HOUR + m * SECONDS_IN_MIN + s);
}

/*
* @brief　クリアしたタイムを記録する
*
* @param[in]  なし
* 
* @return     なし
*/
void ResultScene::UpdateLastTime() const
{
	// ステージ名のキーを取得
	std::string key = StageEnumToString(m_stage);
	OutputDebugStringA(("Stage Key = " + key + "\n").c_str());

	// 現在のセーブデータを読み込む
	json j;
	std::ifstream i(LASTTIME_FILENAME);
	if (i.is_open())
	{
		try
		{
			i >> j;
		}
		catch (...)
		{
			j = json::object();
		}
	}
	i.close();

	// 比較無しで更新
	if (s_currentClearTime > 0.0f)
	{
		j[key] = FormatTimeHMS(s_currentClearTime);

		std::ofstream o(LASTTIME_FILENAME);
		o << std::setw(LOG_COMUMN_WIDTH) << j << std::endl;
	}

}

/*
* @brief　ベストタイム更新処理
*
* @param[in]  なし
* 
* @return     なし
*/
void ResultScene::UpdateBestTime() const
{
	// 現在のセーブデータを読み込む
	json j;
	std::ifstream i(BESTTIME_FILENAME);
	if (i.is_open())
	{
		try
		{
			i >> j;
		}
		catch (...)
		{
			j = json::object();
		}
	}
	i.close();

	// ステージ名を取得
	std::string key = StageEnumToString(m_stage);

	// 比較と更新
	float currentBest = 0.0f;

	// キーがあるか確認
	if (j.contains(key))
	{
		currentBest = ParseTimeHMS(j[key].get<std::string>());
	}

	// クリアタイム>0 かつ、ベスト未記録または更新時に保存
	if (s_currentClearTime > 0.0f &&
		(currentBest == 0.0f || s_currentClearTime < currentBest))
	{
		j[key] = FormatTimeHMS(s_currentClearTime);

		std::ofstream o(BESTTIME_FILENAME);
		o << std::setw(LOG_COMUMN_WIDTH) << j << std::endl;
	}
}

/*
* @brief  クリアタイムを取得
*
* @param[in]  なし
* 
* @return     クリアタイム（秒）
*/
float ResultScene::GetLastTimeFronJson(ResultStage stage)
{
	std::ifstream i(LASTTIME_FILENAME);
	if (!i.is_open()) return PARSE_ERROR_FALLBACK;

	json j;
	try
	{
		i >> j;
	}
	catch (...)
	{
		return PARSE_ERROR_FALLBACK;
	}
	std::string key = StageEnumToString(stage);

	// キーが存在するか確認
	if (j.contains(key))
	{
		return ParseTimeHMS(j[key].get<std::string>());
	}
	return PARSE_ERROR_FALLBACK;
}

/*
* @brief  ベストタイムを取得
*
* @param[in]  なし
* 
* @return     ベストタイム（秒）
*/
float ResultScene::GetBestTimeFromJson(ResultStage stage)
{
	std::ifstream i(BESTTIME_FILENAME);
	// ファイルがない場合
	if (!i.is_open()) return PARSE_ERROR_FALLBACK;

	json j;
	try
	{
		i >> j;
	}
	catch (...)
	{
		return PARSE_ERROR_FALLBACK;
	}
	std::string key = StageEnumToString(stage);

	// キーが存在するか確認
	if (j.contains(key))
	{
		return ParseTimeHMS(j[key].get<std::string>());
	}
	return PARSE_ERROR_FALLBACK;
}

/*
* @brief　デバイスに依存するリソースを作成する関数
*
* @param[in]  なし
* 
* @return     なし
*/
void ResultScene::CreateDeviceDependentResources()
{
	auto deviceResources = GetUserResources()->GetDeviceResources();
	auto device = deviceResources->GetD3DDevice();
	auto context = deviceResources->GetD3DDeviceContext();

	// スプライトバッチの作成
	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(context);

	// 背景の作成
	m_background = std::make_unique<BackGround>();
	m_background->Create(deviceResources);

	// テクスチャの読み込み
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/tutorialstageFont.dds", nullptr, m_tutorialstageFont.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/firstStageFont.dds", nullptr, m_firststageFont.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/secondStageFont.dds", nullptr, m_secondstageFont.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/thirdstageFont.dds", nullptr, m_thirdstageFont.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/clearTimeFont.dds", nullptr, m_timeFont.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/bestTimeFont.dds", nullptr, m_bestTimeFont.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/clearFont.dds", nullptr, m_clearFont.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/gameoverFont.dds", nullptr, m_gameoverFont.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/number.dds", nullptr, m_numberSRV.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/selectKey.dds", nullptr, m_selectKey.ReleaseAndGetAddressOf()));

	// メニューのインスタンス作成
	m_menu = std::make_unique<Menu>();

	// メニューの初期化処理
	D3D11_VIEWPORT windowInfo = deviceResources->GetScreenViewport();
	m_menu->Initialize(
		deviceResources,
		(int)windowInfo.Width,
		(int)windowInfo.Height);
}

/*
* @brief　ウィンドウサイズに依存するリソースを作成する関数
*
* @param[in]  なし
* 
* @return     なし
*/
void ResultScene::CreateWindowSizeDependentResources()
{
	auto deviceResources = GetUserResources()->GetDeviceResources();
	D3D11_VIEWPORT windowInfo = deviceResources->GetScreenViewport();

	ScreenManager::SetDeviceResources(deviceResources);

	if (m_menu)
	{
		m_menu->Initialize(
			deviceResources,
			(int)windowInfo.Width,
			(int)windowInfo.Height);
	}
}

/*
* @brief　デバイスロストしたときに呼び出される関数
*
* @param[in]  なし
* 
* @return     なし
*/
void ResultScene::OnDeviceLost()
{
	m_spriteBatch.reset();

	SoundManager::GetInstance().Update();
}
