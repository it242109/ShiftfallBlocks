//--------------------------------------------------------------------------------------
// File: Player.h
//
// プレイヤーの処理から描画までまとめたクラス
//--------------------------------------------------------------------------------------
#pragma once
#include "SKLib/UserResources.h"

#include "SKLib/CollisionManager.h"
#include "GameObjects/Effects/Shadow.h"
#include "GameObjects/Gimmicks/GimmickBlock.h"

// 前方宣言
class GameCamera;
class TutorialManager;
class CollisionManager;
class InputManager;
class SoundManager;

class Player
{
public:
	// コンストラクタ／デストラクタ
	Player();
	~Player();

	// 初期化処理
	void Initialize(const DirectX::SimpleMath::Vector3& startPos);

	// 更新処理
	void Update(float elapsedTime);

	// 描画処理
	void Render();

	// 終了処理
	void Finalize();

	// デバイスに依存するリソースを作成する関数
	void CreateDeviceDependentResources();

	//　床の着地状態をリセット
	void ResetFloorHit();

	// 影を描画するメソッド
	void RenderShadow();

	// ダメージ処理
	void TakeDamage();

	// プレイヤーのリスポーン
	void Respawn();

	// 死亡処理
	void PlayerKill();

	// 当たり判定を更新
	void UpdateCollision(const AABB& collision, const DirectX::SimpleMath::Vector3& position);

	// マウスホイール入力に基づいて攻撃範囲を更新する
	void UpdateAttackRange(int wheelDelta);

	//////////////////////////////ゲッター／セッター///////////////////////////▼
	// 位置を取得
	const DirectX::SimpleMath::Vector3& GetPosition() const { return m_playerPosition; }

	// 移動速度を取得
	float GetVelocity() const { return NORMAL_SPEED; }

	// 重力を取得
	const float GetGravity() const { return GRAVITY; }

	// 当たり判定を取得
	const AABB& GetCollision() const { return m_playerCollision; }
	const AABB& GetAttackCollision() const { return m_attackCollision; }

	// スタミナを取得
	float GetStamina() const { return m_stamina; }

	// 残機を取得
	int GetLives() const { return m_lives; }

	// 各使用回数を取得
	int GetAttackCount() const { return  m_attackCount; }
	int GetDefenseCount() const { return m_defenseCount; }

	// 攻撃できるようにする
	void AddAttack(int value) { m_attackCount = value; }

	// 防御できるようにする
	void AddDefence(int value) { m_defenseCount = value; }

	// 無敵時間の取得
	float GetInvincibilityTime() const {return m_invincibilityTime;}

	// デバイスリソース／共通ステートの設定
	void SetDeviceResources(DX::DeviceResources* deviceResources) {m_deviceResources = deviceResources;}
	void SetCommonStates(DirectX::CommonStates* states) { m_states = states; }

	// 床情報を設定するメソッド
	void SetFloorData(const std::vector<DirectX::SimpleMath::Vector3>& f_positions,
		const std::vector<DirectX::SimpleMath::Vector3>& f_scales);

	// 足場情報を設定するメソッド
	void SetPlatformData(const std::vector<DirectX::SimpleMath::Vector3>& pf_positions,
		const std::vector<DirectX::SimpleMath::Vector3>& pf_scales);

	// ジャンプ状態を設定
	void SetJumping(bool jumping) { m_isJumping = jumping; }

	// ジャンプ速度を設定
	void SetJumpVelocity(float velocity) { m_verticalVelocity = velocity; }

	// 位置を設定
	void SetPosition(float x, float y, float z);

	// 床接触状態を設定（外部の当たり判定システムから呼ばれる）
	void SetFloorHit(bool hit) { m_floorHit = hit; }

	// カメラの設定
	void SetDebugCamera(GameCamera* camera) { m_gameCamera = camera;}

	// カメラの水平角度を設定
	void SetCameraHorizontalAngle(float angle) { m_cameraHorizontalAngle = angle; }

	// チェックポイントの位置を設定
	void SetRespawnPoint(const DirectX::SimpleMath::Vector3& pos, float yOffset = 0.0f);

	// 残機を設定
	void SetLives(int lives) { m_lives = lives; }

	// ジャンプしているかどうか
	bool IsJumping() const { return m_isJumping; }
	// 床に接触しているかどうか
	bool IsOnFloor() const { return m_floorHit; }

	// 攻撃しているかどうか
	bool IsAttacking() const { return m_isAttacking; }

	// ゴールしたかどうか
	bool IsGoal() const { return m_isGoal; }
	void PlayerGoal() { m_isGoal = true; }

	// 死んだかどうか
	bool IsDead() const { return m_isDead; }

	void SetTutorialManager(TutorialManager* manager) {	m_tutorialManager = manager;}
	//////////////////////////////ゲッター／セッター///////////////////////////▲

	// 剣の範囲の円
	void SwordRangeCircle();

	// コライダーの線
	void ColliderLine();

private: 
	// 定数
	static constexpr float GRAVITY = -9.8f;
	static constexpr float JUMP_POWER = 5.5f;
	static constexpr float FALL_SPEED = -3.0f;
	const float NORMAL_SPEED = 0.15f;
	const float DASH_SPEED = 0.3f;
	const float MAX_STAMINA = 50.0f;
	const float STAMINA_COST_PER_SEC = 20.0f;
	const float STAMINA_REGEN_RATE = 5.0f;
	const float MIN_DASH_STAMINA = 10.0f;

private:
	// デバッグカメラのポインタ
	GameCamera* m_gameCamera;

	// スプライトバッチのポインタ
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

	DX::DeviceResources* m_deviceResources = nullptr;
	DirectX::CommonStates* m_states = nullptr;

	// ビュー行列
	DirectX::SimpleMath::Matrix m_view;

	// 射影行列
	DirectX::SimpleMath::Matrix m_proj;

	// ベーシックエフェクト
	std::unique_ptr<DirectX::BasicEffect> m_basicEffect;
	
	// プリミティブバッチ
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_primitiveBatch;

	// デバイスコンテキスト
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;

	// インプットレイアウト
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

	// チュートリアルマネージャーへのポインタ
	TutorialManager* m_tutorialManager = nullptr;
	// テクスチャ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_attackRangeTexture;

	// モデル
	std::unique_ptr<DirectX::Model> m_playerModel;
	std::unique_ptr<DirectX::Model> m_havingSwordModel;
	std::unique_ptr<DirectX::Model> m_havingShieldModel;

	// モデルの位置
	DirectX::SimpleMath::Vector3 m_playerPosition;
	DirectX::SimpleMath::Vector3 m_havingSwordPosition;
	DirectX::SimpleMath::Vector3 m_havingShieldPosition;

	//　スケール
	DirectX::SimpleMath::Vector3 m_playerScale;

	// モデルの平行移動の行列
	DirectX::SimpleMath::Matrix m_playerTrans;

	// 当たり判定
	AABB m_playerCollision;
	AABB m_attackCollision; // 攻撃用
	float m_attackRange;

	// 攻撃範囲の円盤
	std::unique_ptr<DirectX::GeometricPrimitive> m_disk;

	// 影
	Shadow m_shadow;

	// Y座標の表面
	float m_currentSurfaceY = 0.0f;

	// プレイヤーの向き
	DirectX::SimpleMath::Vector3 m_playerForward;

	// ジャンプメンバ変数
	bool m_isJumping;
	float m_verticalVelocity;

	// スタミナ
	float m_stamina = MAX_STAMINA;
	bool m_canDash = true;

	// 残機
	int m_lives = 0;

	// 攻撃可能回数
	int m_attackCount;

	// 防御可能回数
	int m_defenseCount;

	// 無敵かどうか
	bool m_isInvincible;

	// 無敵時間
	float m_invincibilityTime = 0.0f;

	// 床に触れているかどうか
	bool m_floorHit;

	// 攻撃中かどうか
	bool m_isAttacking;

	// 攻撃時間
	float m_attackTimer = 0.0f;

	// ダメージを受けた際に再生するフラグ
	bool m_isPlayShieldSound = false;
	bool m_isPlayDamageSound = false;

	// ゴールしたかどうか
	bool m_isGoal;

	// 死んだかどうか
	bool m_isDead;

	// プレイヤーの開始位置
	DirectX::SimpleMath::Vector3 m_startPosition;
	// リスポーン地点
	DirectX::SimpleMath::Vector3 m_respawnPoint;

	// 影描画用のメンバ変数
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState_Shadow;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PS;
	std::unique_ptr<DirectX::Model> m_shadowModel;

	// 床の情報（影を落とすために必要）
	std::vector<DirectX::SimpleMath::Vector3> m_floorPositions;
	std::vector<DirectX::SimpleMath::Vector3> m_floorScales;
	std::vector<DirectX::SimpleMath::Vector3> m_platformPositions;
	std::vector<DirectX::SimpleMath::Vector3> m_platformScales;

	float m_cameraHorizontalAngle = 0.0f;
};

