//--------------------------------------------------------------------------------------
// File: SelectStage.h
//
// セレクトシーンクラス
//--------------------------------------------------------------------------------------
#pragma once

#include "SKLib/UserResources.h"

#include "SKLib/SceneManager.h"
#include "SKLib/InputManager.h"
#include "SKLib/ScreenManager.h"
#include "SKLib/SoundManager.h"
#include "SKLib/GameCamera.h"
#include "SKLib/TaskManager.h"
#include "Task/Number.h"

#include "GameObjects/UIs/Menu.h"
#include "GameObjects/Effects/BackGround.h"

class SelectScene : public SceneBase<UserResources>
{
public:
	// ゲッター／セッター -------------------------------------------------------------------
	// --- システム・グラフィックス ---
	// ステージシーンからパスを取得
	static std::string GetCurrentStageFilePath();
	// リザルトシーン等から次のステージのパスを同期させる関数
	static void SetCurrentStageFilePathByIndex(int stageIndex);

public:
	// 関数 ---------------------------------------------------------------------------------
	// コンストラクタ／デストラクタ
	SelectScene();
	~SelectScene();

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
	// 定数 ---------------------------------------------------------------------------------
	static const float DEFAULT_POSITION_X;	///< デフォルトの位置X
	static const float DEFAULT_POSITION_Y;	///< デフォルトの位置Y
	static const float DEFAULT_SCALE_X;		///< デフォルトの大きさX
	static const float DEFAULT_SCALE_Y;		///< デフォルトの大きさY
	static const float SELECTED_UI_OFFSET_POSITION_X;	///< 選択中のUI項目を移動させるX軸のオフセット量
	static const int MAX_MENUINDEX;			///< メニューインデックスの最大値

private:
	// メンバ変数 ---------------------------------------------------------------------------
	// デバイスリソースのポインタ
	std::unique_ptr<DX::DeviceResources>    m_deviceResources;

	// スプライトバッチのポインタ
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

	// キーボード状態を保存
	DirectX::Keyboard::State m_prevKeyboardState = {};

	// ステージを読み込むための変数
	std::vector<std::string> m_stageFilePaths;

	// 各ステージのラストタイムを取得
	int m_lastTime_Tutorial;
	int m_lastTime_FirstStage;
	int m_lastTime_SecondStage;
	int m_lastTime_ThirdStage;

	// 各ステージのベストタイムを取得
	int m_bestTime_Tutorial;
	int m_bestTime_FirstStage;
	int m_bestTime_SecondStage;
	int m_bestTime_ThirdStage;

	// タスクマネージャー
	TaskManager m_taskManager;

	// 数字を表示させるポインタ
	Number* m_lastTimeNumber;
	Number* m_bestTimeNumber;

	// テクスチャ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_downSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_upSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_selectSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_selectFontSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_lastTimeSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_bestTimeSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_numberSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_selectKeySRV;

	// 背景パーティクル
	std::unique_ptr<BackGround> m_background;

	// メニュー
	std::unique_ptr<Menu> m_selectMenu;

	// キーが押されたかどうか
	bool m_isKeyPressed = false;;

	// フェード関連
	bool m_isClosingTutorial = false;
	bool m_isClosingFirstStage = false;
	bool m_isClosingSecondStage = false;
	bool m_isClosingThirdStage = false;
	bool m_isClosingTitle = false;
};

