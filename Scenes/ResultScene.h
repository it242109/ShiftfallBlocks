//--------------------------------------------------------------------------------------
// File: ResultScene.h
//
// リザルトシーンクラス
//--------------------------------------------------------------------------------------
#pragma once

#include "SKLib/UserResources.h"
#include "TutorialScene.h"
#include "SelectScene.h"
#include "StageScene.h"

#include "SKLib/SceneManager.h"
#include "SKLib/InputManager.h"
#include "SKLib/ScreenManager.h"
#include "SKLib/SoundManager.h"
#include "SKLib/TaskManager.h"
#include "Task/Number.h"

#include "GameObjects/Effects/BackGround.h"
#include "GameObjects/UIs/Menu.h"

#include "Resources/json.hpp"

// JSONファイルを使えるようにする
using json = nlohmann::json;

class ResultScene : public SceneBase<UserResources>
{
public:
	// 列挙体の管理 -------------------------------------------------------------------------
	enum class ResultStage // 各ステージ
	{
		TUTORIAL,	// チュートリアル
		FIRST,		// ステージ１
		SECOND,		// ステージ２
		THIRD,		// ステージ３
		NONE		// デフォルト
	};
	enum class ResultType // ゲーム結果
	{
		CLEAR,		// クリア
		GAMEOVER,	// ゲームオーバー
		NONE		// デフォルト
	};

public:
	// ゲッター／セッター -------------------------------------------------------------------
	// --- リザルト・ステージ進行  ---
	// グローバルなゲーム結果を設定
	static void SetGlobalResult(ResultType result) { s_globalResult = result; }
	// 現在のグローバルステージを設定
	static void SetGlobalStage(ResultStage stage) { s_globalStage = stage; }
	// 現在のグローバルステージを取得
	static ResultStage GetGlobalStage() { return s_globalStage; }

	// --- 現在のプレイ記録 ---
	// 今回のクリアタイムを設定
	static void SetGlobalClearTime(float time) { s_currentClearTime = time; }

	// --- 保存された記録の取得 ) ---
	// 前回のクリアタイムをJSONから取得
	static float GetLastTimeFronJson(ResultStage stage);
	// 指定ステージの自己ベストタイムをJSONから取得
	static float GetBestTimeFromJson(ResultStage stage);

public:
	// 関数 ---------------------------------------------------------------------------------
	// コンストラクタ
	ResultScene();
	~ResultScene();

	// 初期化処理
	void Initialize() override;

	// 更新処理
	void Update(float elapsedTime) override;

	// 描画処理
	void Render() override;

	// 終了処理
	void Finalize() override;

	// デバイスに依存するリソースを作成する関数
	void CreateDeviceDependentResources() override;

	// ウインドウサイズに依存するリソースを作成する関数
	void CreateWindowSizeDependentResources() override;

	// デバイスロストした時に呼び出される関数
	void OnDeviceLost() override;
private:
	// 文字列に変換
	static std::string StageEnumToString(ResultStage stage);

	// 秒から分秒表記に変換する関数
	static std::string FormatTimeHMS(float timeSec);

	// 分秒表記から秒に戻す関数
	static float ParseTimeHMS(const std::string& timeStr);

	// クリアしたタイムを記録する
	void UpdateLastTime() const;

	// ベストタイム更新処理
	void UpdateBestTime() const;

private:
	// 定数 ---------------------------------------------------------------------------------
	static const int SECONDS_IN_HOUR;		///< １時間あたりの秒数
	static const int SECONDS_IN_MIN;		///< １分あたりの秒数
	static const int PAD_WIDTH;				///< 桁そろえの文字幅
	static const int LOG_COMUMN_WIDTH;		///< 出力時の桁数指定（4ケタ）
	static const float PARSE_ERROR_FALLBACK;///< 解析失敗時のフォールバック値

private:
	// メンバ変数 ---------------------------------------------------------------------------
	// スプライトバッチのポインタ
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

	// キーボード状態を保存
	DirectX::Keyboard::State m_prevKeyboardState = {};

	// リザルトタイプ
	ResultType m_result;
	static ResultType s_globalResult;
	ResultStage m_stage;
	static ResultStage s_globalStage;

	// タイム関連
	static float s_currentClearTime;
	int m_clearTime;
	int m_bestTime;

	// タスクマネージャー
	TaskManager m_taskManager;

	// 数字を表示させるポインタ
	Number* m_clearTimeNumber;
	Number* m_bestTimeNumber;

	// 背景パーティクル
	std::unique_ptr<BackGround> m_background;

	// テクスチャ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_tutorialstageFont;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_firststageFont;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_secondstageFont;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_thirdstageFont;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_timeFont;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_bestTimeFont;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_clearFont;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_gameoverFont;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_numberSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_selectKey;

	// メニュー
	std::unique_ptr<Menu> m_menu;

	// フェード関連
	bool m_isClosing = false;
};

