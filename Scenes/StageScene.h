//--------------------------------------------------------------------------------------
// File: StageScene.h
//
// ステージシーンクラス
//--------------------------------------------------------------------------------------
#pragma once
#include "SKLib/UserResources.h"
#include "SelectScene.h"
#include "ResultScene.h"

#include "SKLib/SceneManager.h"
#include "SKLib/GameCamera.h"
#include "SKLib/CollisionManager.h"
#include "SKLib/ScreenManager.h"
#include "SKLib/SoundManager.h"
#include "SKLib/TaskManager.h"
#include "Task/Number.h"

#include "GameObjects/Stage.h"
#include "GameObjects/Stages/Player.h"
#include "GameObjects/Stages/Enemy.h"

#include "GameObjects/UIs/Menu.h"
#include "GameObjects/UIs/HealthUI.h"
#include "GameObjects/UIs/StaminaUI.h"
#include "GameObjects/UIs/ShieldUI.h"
#include "GameObjects/UIs/SwordUI.h"

class StageScene : public SceneBase<UserResources>
{
public:
	// コンストラクタ／デストラクタ
	StageScene();
	~StageScene();

	// 初期化処理
	void Initialize() override;

	// 更新処理
	void Update(float elapsedTime) override;

	// 描画処理
	void Render() override;

	// 終了処理
	void Finalize() override;

	// ゲームのリセット
	void ResetGame();

	// デバイスに依存するリソースを作成する関数
	void CreateDeviceDependentResources() override;

	// ウインドウサイズに依存するリソースを作成する関数
	void CreateWindowSizeDependentResources() override;

	// デバイスロストした時に呼び出される関数
	void OnDeviceLost() override;

private:
	// キーボード状態を保存
	DirectX::Keyboard::State m_prevKeyboardState = {};

	// マウス状態を保存
	DirectX::Mouse::State m_prevMouseState = {};

	// ビュー行列
	DirectX::SimpleMath::Matrix m_view;

	// 射影行列
	DirectX::SimpleMath::Matrix m_proj;

	// 共通ステート
	std::unique_ptr<DirectX::CommonStates> m_states;

	// ベーシックエフェクト
	std::unique_ptr<DirectX::BasicEffect> m_basicEffect;

	// プリミティブバッチ
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_primitiveBatch;

	// インプットレイアウト
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

	// ステージ全体
	std::unique_ptr<Stage> m_stage;

	// プレイヤー／敵
	std::unique_ptr<Player> m_player;
	std::vector<std::unique_ptr<Enemy>> m_enemies;
	std::vector<DirectX::SimpleMath::Vector3> m_enemyStartPositions;

	// カメラ
	std::unique_ptr<GameCamera> m_gameCamera;
	// カメラの現在の角度を保持する変数
	float m_cameraHorizontalAngle;
	// UI
	std::unique_ptr<HealthUI> m_healthUI;
	std::unique_ptr<StaminaUI> m_staminaUI;
	std::unique_ptr<SwordUI> m_swordUI;
	std::unique_ptr<ShieldUI> m_shieldUI;

	// タスクマネージャー
	TaskManager m_taskManager;

	// 数字を表示させるポインタ
	Number* m_number;

	// タイマー
	float m_timer;
	bool m_isTimerActive = true;
	float m_teleportTimer;

	// スプライトバッチのポインタ
	std::unique_ptr <DirectX::SpriteBatch> m_spriteBatch;

	// テクスチャ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_numberSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_timeSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pauseKeySRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_selectKeySRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_overlayTexture;

	// パーティクル
	std::unique_ptr<SwirlParticle> m_swirlParticle;

	// メニュー
	std::unique_ptr<Menu> m_pauseMenu;

	// 判定変数
	bool m_isPause;
	bool m_isTeleporting;
	// デバッグモード
	bool m_isDebugMode;
};

