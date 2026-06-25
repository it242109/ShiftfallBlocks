//--------------------------------------------------------------------------------------
// File: Player.h
//
// プレイヤーの処理から描画までまとめたクラス
//--------------------------------------------------------------------------------------
#pragma once
#include "SKLib/UserResources.h"

#include "SKLib/CollisionManager.h"
#include "GameObjects/Effects/Shadow.h"
#include "GameObjects/Effects/PlayerDashParticle.h"
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
	// ゲッター／セッター -----------------------------------------------------------------
	// --- プレイヤーの状態 ---
	// 残機
	int GetLives() const { return m_lives; }
	void SetLives(int lives) { m_lives = lives; }

	// スタミナ
	float GetStamina() const { return m_stamina; }

	// 無敵時間
	float GetInvincibilityTime() const { return m_invincibilityTime; }

	// 攻撃・防御の回数と有効化
	int GetAttackCount() const { return m_attackCount; }
	void AddAttack(int value) { m_attackCount = value; }
	int GetDefenseCount() const { return m_defenseCount; }
	void AddDefence(int value) { m_defenseCount = value; }

	// プレイヤーのアクション・死亡フラグ状態
	bool IsAttacking() const { return m_isAttacking; }
	bool IsDead() const { return m_isDead; }
	bool IsGoal() const { return m_isGoal; }
	void PlayerGoal() { m_isGoal = true; }

	// --- 移動・物理・カメラ挙動 ---
	// 位置の取得と設定
	const DirectX::SimpleMath::Vector3& GetPosition() const { return m_playerPosition; }
	void SetPosition(float x, float y, float z);

	// 移動速度の取得
	float GetVelocity() const { return NORMAL_SPEED; }

	// 重力の取得
	const float GetGravity() const { return GRAVITY; }

	// ジャンプ・空中状態
	bool IsJumping() const { return m_isJumping; }
	void SetJumping(bool jumping) { m_isJumping = jumping; }
	void SetJumpVelocity(float velocity) { m_verticalVelocity = velocity; }

	// カメラ関連
	void SetDebugCamera(GameCamera* camera) { m_gameCamera = camera; }
	void SetCameraHorizontalAngle(float angle) { m_cameraHorizontalAngle = angle; }

	// --- 当たり判定・ステージオブジェクト ---
	// 衝突判定（AABB）の取得
	const AABB& GetCollision() const { return m_playerCollision; }
	const AABB& GetAttackCollision() const { return m_attackCollision; }

	// 床・足場の接触状態と地形データの設定
	bool IsOnFloor() const { return m_floorHit; }
	void SetFloorHit(bool hit) { m_floorHit = hit; }
	void SetFloorData(const std::vector<DirectX::SimpleMath::Vector3>& f_positions, const std::vector<DirectX::SimpleMath::Vector3>& f_scales);
	void SetPlatformData(const std::vector<DirectX::SimpleMath::Vector3>& pf_positions, const std::vector<DirectX::SimpleMath::Vector3>& pf_scales);

	// --- 外部マネージャー・リソースの連携 ---
	// DirectXデバイス・描画ステートの設定
	void SetDeviceResources(DX::DeviceResources* deviceResources) { m_deviceResources = deviceResources; }
	void SetCommonStates(DirectX::CommonStates* states) { m_states = states; }

	// パーティクル・演出システム
	void SetDashParticle(PlayerDashParticle* pParticle) { m_pDashParticle = pParticle; }

	// ゲーム内システムマネージャー
	void SetTutorialManager(TutorialManager* manager) { m_tutorialManager = manager; }

public:
	// 関数 ------------------------------------------------------------------------
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

	// 剣の範囲の円
	void SwordRangeCircle();

	// コライダーの線
	void ColliderLine();

private: 
	// 定数 ------------------------------------------------------------------------
	static const float GRAVITY;						///< 重力
	static const float JUMP_POWER;					///< ジャンプの初速
	static const float FALL_SPEED;					///< 落下速度
	static const float NORMAL_SPEED;				///< 通常スピードの値
	static const float DASH_SPEED;					///< ダッシュスピードの値
	static const float MAX_STAMINA;					///< スタミナ最大値
	static const float STAMINA_COST_PER_SEC;		///< 秒ごとのスタミナコスト
	static const float STAMINA_REGEN_RATE;			///< スタミナ回復率
	static const float MIN_DASH_STAMINA;			///< スタミナの最小値
	static const float DASH_RECOVERY_THRESHOLD_RATIO;///< ダッシュ再開が可能になるスタミナの割合
	static const float FALL_DEATH_THRESHOLD_Y;		///< 落下死と判定するY座標のしきい値
	static const float HALF_SCALE;					///< 半分のサイズにする

	static const float TOP_Y_OFFSET_THRESHOLD;		///< 判定対象とする床の高さの許容誤差
	static const float DEFAULT_SURFACE_Y;			///< 床が見つからなかった場合のデフォルトの高さ
	static const float SHADOW_Z_FIGHTING_OFFSET;	///< 地面とのちらつきを防ぐための高さオフセット（影用）
	static const float SHADOW_SCALE_ATTENUATION;	///< 高さによる影の減衰率
	static const float MIN_SHADOW_SCALE;			///< 影の最小スケール

	static const float BLINK_SPEED;							///< 点滅速度
	static const float INVINCIBLE_BLINK_THRESHOLD;			///< 点滅のしきい値
	static const DirectX::SimpleMath::Vector3 SWORD_OFFSET; ///< 剣のオフセット値
	static const float SWORD_ROT_Y;							///< 剣のＹの向き
	static const float SWORD_ROT_X;							///< 剣のＸの向き
	static const float SWORD_SCALE;							///< 剣の大きさ
	static const DirectX::SimpleMath::Vector3 SHIELD_OFFSET;///< 盾のオフセット値
	static const float SHIELD_ROT_Y;						///< 盾のＹの向き
	static const float SHIELD_ROT_X;						///< 盾のＸの向き
	static const float SHIELD_SCALE;						///< 盾の大きさ

	static const float SHIELD_INVINCIBILITY_DURATION;	///< シールド発動時の無敵時間（秒）
	static const float DAMAGE_INVINCIBILITY_DURATION;	///< 被弾時の無敵時間（秒）

	static const int CIRCLE_SEGMENTS;				///< 円を表現する分割数
	static const float SWORD_LINE_WIDTH;			///< 描画する線の太さ
	static const float SWORD_Z_FIGHTING_OFFSET;		///< 地面とのちらつきを防ぐための高さオフセット（剣の範囲用）
	static const float FULL_CIRCLE_TURN_RATIO;		///< ラジアン計算用の全周係数（2.0 * π = 360度）

	static const float MOUSEWHEEL_MIN_RANGE;		///< マウスホイールの最小値
	static const float MOUSEWHEEL_MAX_RANGE;		///< マウスホイールの最大値
	static const float RANGE_STEP;					///< 一回のホイールの刻みで変化させる量
	static const int WHEEL_TICKS;					///< ホイール入力をノッチ数に変換する定数

	static const float FIELD_OF_VIEW_DEGREES;		///< 視野角
	static const float NEAR_PLANE_DISTANCE;			///< カメラの最前面のクリップ距離
	static const float FAR_PLANE_DISTANCE;			///< カメラの最遠面のクリップ距離

private:
	// メンバ変数 ------------------------------------------------------------------
	// デバッグカメラのポインタ
	GameCamera* m_gameCamera;

	// スプライトバッチのポインタ
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

	//　デバイスリソース
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
	float m_currentSurfaceY;

	// プレイヤーの向き
	DirectX::SimpleMath::Vector3 m_playerForward;

	// ジャンプメンバ変数
	bool m_isJumping;
	float m_verticalVelocity;

	// スタミナ
	float m_stamina = MAX_STAMINA;
	bool m_canDash = true;

	// ダッシュパーティクル
	PlayerDashParticle* m_pDashParticle = nullptr;

	// 残機
	int m_lives;

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
	float m_attackTimer;

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

	// カメラの水平角度
	float m_cameraHorizontalAngle = 0.0f;
};

