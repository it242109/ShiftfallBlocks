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
#include "GameObjects/StageObjects/Player.h"
#include "GameObjects/StageObjects/Enemy.h"

#include "GameObjects/UIs/Menu.h"
#include "GameObjects/UIs/HealthUI.h"
#include "GameObjects/UIs/StaminaUI.h"
#include "GameObjects/UIs/ShieldUI.h"
#include "GameObjects/UIs/SwordUI.h"

class StageScene : public SceneBase<UserResources>
{
public:
	// 関数 ---------------------------------------------------------------------------------
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
	// 定数 ---------------------------------------------------------------------------------
	static const float CAMERA_DISTANCE;			///< カメラの初期の距離
	static const DirectX::SimpleMath::Vector3 PLAYER_INITIAL_POSITION;	///< プレイヤーの初期位置
	static const DirectX::SimpleMath::Vector2 NUMBER_POSITION;			///< タイマーで使われる数字の初期位置

	static const float FALLTODEATH_HEIGHT;		///< 落下死する高さ
	static const int ATTACK_COUNT;				///< 攻撃回数
	static const float WAIT_TIME;				///< ゴールまたはゲームオーバー後の待ち時間
	static const float TELEPORT_COOLDOWN_TIME;	///< テレポートした後のクールダウンタイム
	static const float TIMER_END_THRESHOLD;			///< タイマーが終了したと判定する基準値
	static const float INVINCIBILITY_END_THRESHOLD;	///< 無敵時間が終了した基準値
	static const int GAMEOVER_LIFE_COUNT;			///< ゲームオーバーとなる残機の数

	static const float FONT_INITIAL_POSITION_X;	///< クリア／ゲームオーバーフォントの初期位置
	static const float FONT_X_MAX;				///< クリア／ゲームオーバーフォントXの最大数値
	static const float FONT_SPEED;				///< クリア／ゲームオーバーフォントの移動速度

	static const float FADE_SPEED;				///< テレポートによるフェード速度
	static const float OPAQUE_OVERLAY_ALPHA;	///<　不透明のオーバーレイのα値

	static const float MENU_DEFAULT_POSITION_X; ///< メニューのデフォルトの位置X
	static const float MENU_DEFAULT_SCALE_X;	///< メニューのデフォルトの大きさX
	static const float MENU_DEFAULT_SCALE_Y;	///< メニューのデフォルトの大きさY
	static const float DEFAULT_SRV_SCALE_X;		///< ＳＲＶのデフォルトの大きさX
	static const float DEFAULT_SRV_SCALE_Y;		///< ＳＲＶのデフォルトの大きさY

	static const float FIELD_OF_VIEW_DEGREES;	///< 視野角
	static const float NEAR_PLANE_DISTANCE;		///< カメラの最前面のクリップ距離
	static const float FAR_PLANE_DISTANCE;		///< カメラの最遠面のクリップ距離

	static const int BASE_SCREEN_WIDTH;			///< ゲームの基本画面解像度（横幅）
	static const int BASE_SCREEN_HEIGHT;		///< ゲームの基本画面解像度（縦幅）

private:
	// メンバ変数 ---------------------------------------------------------------------------
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

	// UI
	std::unique_ptr<HealthUI> m_healthUI;
	std::unique_ptr<StaminaUI> m_staminaUI;
	std::unique_ptr<SwordUI> m_swordUI;
	std::unique_ptr<ShieldUI> m_shieldUI;

	// タスクマネージャー
	TaskManager m_taskManager;

	// 数字を表示させるポインタ
	Number* m_number;

	// フォントの表示位置
	float m_fontPosX;

	// タイマー
	float m_timer;
	float m_teleportTimer;
	float m_waitTimer;

	// テレポートによるフェードインのα値
	float m_teleportOverlayAlpha;

	// スプライトバッチのポインタ
	std::unique_ptr <DirectX::SpriteBatch> m_spriteBatch;

	// テクスチャ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_numberSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_timeSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pauseKeySRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_selectKeySRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_gameOverSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_gameClearSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_overlayTexture;

	// パーティクル
	std::unique_ptr<SwirlParticle> m_swirlParticle;
	std::unique_ptr<PlayerDashParticle> m_dashParticle;

	// メニュー
	std::unique_ptr<Menu> m_pauseMenu;

	// 判定変数
	bool m_isPause;
	bool m_isTeleporting;
	bool m_isTimerActive;
	bool m_isGoalWaiting;
	bool m_isGameOverWaiting;
	bool m_isSEPlayed;

	// デバッグモード
	bool m_isDebugMode;
};

