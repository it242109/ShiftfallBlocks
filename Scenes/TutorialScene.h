//--------------------------------------------------------------------------------------
// File: TutorialScene.h
//
// チュートリアルステージシーンクラス
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
#include "SKLib/TutorialManager.h"
#include "Task/Number.h"

#include "GameObjects/Stages/Player.h"
#include "GameObjects/Stages/Enemy.h"
#include "GameObjects/Gimmicks/GimmickBlock.h"
#include "GameObjects/Gimmicks/Platform.h"
#include "GameObjects/Gimmicks/Switch.h"
#include "GameObjects/Stages/StageObject.h"
#include "GameObjects/Gimmicks/Portal.h"
#include "GameObjects/Gimmicks/Gate.h"
#include "GameObjects/Gimmicks/Item.h"
#include "GameObjects/Stages/Goal.h"
#include "GameObjects/UIs/Menu.h"

#include "GameObjects/UIs/HealthUI.h"
#include "GameObjects/UIs/StaminaUI.h"
#include "GameObjects/UIs/SwordUI.h"
#include "GameObjects/UIs/ShieldUI.h"

#include "GameObjects/Effects/PortalParticle.h"
#include "GameObjects/Effects/SwirlParticle.h"
#include "GameObjects/Effects/SwitchParticle.h"

class TutorialScene : public SceneBase<UserResources> ,public ICameraCollisionProvider
{
public:
	// コンストラクタ/デストラクタ
	TutorialScene();
	~TutorialScene();

	// 初期化処理
	void Initialize() override;

	// 更新
	void Update(float elapsedTime) override;

	// 描画
	void Render() override;

	// 終了処理
	void Finalize();

	// ゲームをリセット
	void ResetGame();

	// オブジェクトのリセット
	void ResetObjects();

	// カメラからの距離を計算する関数
	float GetClosestHitDistance(const DirectX::SimpleMath::Vector3& origin,
		const DirectX::SimpleMath::Vector3& direction,
		float maxDistance) const override;

	// デバイスに依存するリソースを作成する関数
	void CreateDeviceDependentResources() override;

	// ウインドウサイズに依存するリソースを作成する関数
	void CreateWindowSizeDependentResources() override;

	// デバイスロストした時に呼び出される関数
	void OnDeviceLost() override;

private:
	// カメラのポインタ
	std::unique_ptr<GameCamera> m_gameCamera;

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

	// モデル
	std::unique_ptr<DirectX::Model> m_floorModel;
	std::unique_ptr<DirectX::Model> m_wallModel;
	std::shared_ptr<DirectX::Model> m_platformBlockModel;
	std::shared_ptr<DirectX::Model> m_keyBlockModel;
	std::shared_ptr<DirectX::Model> m_portalBlockModel;
	std::shared_ptr<DirectX::Model> m_itemBlockModel;
	std::unique_ptr<DirectX::Model> m_switchModel;
	std::unique_ptr<DirectX::Model> m_portalModel;

	// スプライトバッチのポインタ
	std::unique_ptr <DirectX::SpriteBatch> m_spriteBatch;

	// テクスチャ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_explanationFirstSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_explanationSecondSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_tutorialstageFont;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_clearFont;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_numberSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_timeSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pauseKeySRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_selectKeySRV;

	// チュートリアル用のテクスチャ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_tmoveSRV; 
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_tmovingMouseSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_tliftSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_tputSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_titemSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_tattackSRV;

	// オーバーレイ用のテクスチャ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_overlayTexture;

	// モデルの平行移動の行列
	std::vector<DirectX::SimpleMath::Vector3> m_itemSpawnPositions;

	// 壁・床・足場
	std::unique_ptr<StageObject> m_floor;
	std::unique_ptr<StageObject> m_wall;
	std::unique_ptr<Platform> m_platform;

	// 各スイッチ
	std::shared_ptr<Switch> m_switch;

	// 各ポータル
	std::shared_ptr<Portal> m_portal;

	// 扉
	std::unique_ptr<Gate> m_gate;

	// アイテム
	std::unique_ptr<Item> m_item;
	std::vector<ItemData>m_independentItems;
	
	// 各パーティクル
	std::unique_ptr<PortalParticle> m_portalParticle;
	std::unique_ptr<SwirlParticle> m_swirlParticle;
	std::unique_ptr<SwitchParticle> m_switchParticle;

	// プレイヤー
	std::unique_ptr<Player> m_player;

	// 敵
	std::vector<std::unique_ptr<Enemy>> m_enemies;
	std::vector<DirectX::SimpleMath::Vector3> m_enemyStartPositions;

	// 仕掛けブロック
	std::vector<std::shared_ptr<GimmickBlock>> m_gimmickBlocks;
	std::vector<size_t> m_itemGimmickBlockIndices;				     ///< アイテム用仕掛けブロックのインデックス記録する変数
	std::shared_ptr<GimmickBlock> m_followingBlock = nullptr;		 ///< 持ち上げ中のブロックを管理する変数

	// ゴール
	std::shared_ptr<Goal> m_goal;

	// 各モデルの当たり判定
	CollisionManager m_collisionManager;

	// カメラの現在の角度を保持する変数
	float m_cameraHorizontalAngle;
	float m_cameraVerticalAngle;

	// ポーズやテレポートしているかのフラグ
	bool m_isPause;
	bool m_isTeleporting;

	// チュートリアルがアクティブかどうかのフラグ
	bool m_isTutorialActive;

	// チュートリアルマネージャーへのポインタ
	std::unique_ptr<TutorialManager> m_tutorialManager;

	// 操作説明を切り替えるフラグ
	bool m_showExplanationFirst;
	bool m_showExplanationSecond;

	// 各ギミックのON/OFF
	std::vector<bool> m_isSwitchOn_PF;		// 足場
	bool m_isSwitchOn_Key;					// カギ
	std::vector<bool> m_isSwitchOn_Portal;	// ポータル
	std::vector<bool> m_isSwitchOn_Item;	// アイテム

	//　スイッチごとの固定タイプ
	std::vector<ItemType> m_itemTypes;

	// 剣の処理
	bool m_isSwordCollected = false;
	bool m_isSwordSystemActive = false;

	// タスクマネージャー
	TaskManager m_taskManager;

	// タイマー
	float m_timer;
	bool m_isTimerActive = true;
	float m_teleportTimer;
	float m_swordRespawnTimer;

	// 数字を表示させるポインタ
	Number* m_number;

	// チュートリアルを表示させるフラグ
	bool m_liftTutorialShown;
	bool m_switchShown;
	bool m_itemShown;

	// UI
	std::unique_ptr<HealthUI> m_healthUI;
	std::unique_ptr<StaminaUI> m_staminaUI;
	std::unique_ptr<SwordUI> m_swordUI;
	std::unique_ptr<ShieldUI> m_shieldUI;

	// メニュー
	std::unique_ptr<Menu> m_pauseMenu;

	// <デバッグ>カメラの追従をON/OFF
	bool m_isFollowCamera;

	// <デバッグ>デバッグモード
	bool m_isDebugMode;
};