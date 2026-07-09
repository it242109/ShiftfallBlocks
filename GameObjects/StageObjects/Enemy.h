//--------------------------------------------------------------------------------------
// File: Enemy.h
//
// 敵の処理から描画までまとめたクラス
//--------------------------------------------------------------------------------------
#pragma once
#include "SKLib/UserResources.h"

#include "SKLib/CollisionManager.h"
#include "SKLib/InputManager.h"
#include "SKLib/SoundManager.h"

#include "GameObjects/Effects/Shadow.h"

// 列挙体の管理：敵の状態
enum class EnemyState
{
	Idle,	// 待機
	Chase,	// 追尾
	Dead	// 死亡
};

class Enemy
{
public:
	// ゲッター／セッター -----------------------------------
	// --- 位置 ---
	// 位置を取得
	const DirectX::SimpleMath::Vector3& GetPosition() const { return m_enemyPosition; }
	// 位置を設定
	void SetPosition(float x, float y, float z);

	// --- 移動速度・物理 ---
	// 移動速度を取得
	float GetVelocity() const { return m_enemyVelocity; }
	// 重力を取得
	const float GetGravity() const { return GRAVITY; }

	// --- 当たり判定 ---
	// 当たり判定を取得
	const AABB& GetCollision() const { return m_enemyCollision; }
	const AABB& GetDamageCollision() const { return m_damageCollision; }

	// --- 床・足場情報  ---
	// 床接触状態を設定
	void SetFloorHit(bool hit) { m_floorHit = hit; };
	// 床に接触しているかどうか
	bool IsOnFloor() const { return m_floorHit; }
	// 床情報を設定するメソッド
	void SetFloorData(const std::vector<DirectX::SimpleMath::Vector3>& f_positions,
		const std::vector<DirectX::SimpleMath::Vector3>& f_scales);
	// 足場情報を設定するメソッド
	void SetPlatformData(const std::vector<DirectX::SimpleMath::Vector3>& pf_positions,
		const std::vector<DirectX::SimpleMath::Vector3>& pf_scales);

	// --- 生死状態  ---
	// 死んだかどうか
	bool IsDead() const { return m_isDead; }
	// 敵を死亡させる
	void EnemyKill();
	// リセット
	void Reset(const DirectX::SimpleMath::Vector3& pos);

	// ---  外部マネージャー・リソースの連携 ---
	// DirectXデバイス・描画ステートの設定
	void SetDeviceResources(DX::DeviceResources* deviceResources) { m_deviceResources = deviceResources; };
	void SetCommonStates(DirectX::CommonStates* states) { m_states = states; };

public:
	// 関数 --------------------------------------------------
	// コンストラクタ／デストラクタ
	Enemy();
	~Enemy();

	// 初期化処理
	void Initialize(const DirectX::SimpleMath::Vector3& startPos);

	// 更新処理
	void Update(float elapsedTime, const DirectX::SimpleMath::Vector3& enemyPos);

	// 状態遷移処理
	void UpdateState(const DirectX::SimpleMath::Vector3& enemyPos);

	// 追尾処理
	void UpdateChase(const DirectX::SimpleMath::Vector3& enemyPos);

	// 待機処理
	void UpdateIdle();

	// 重力の適用
	void ApplyGravity(float elapsedTime);

	// 描画処理
	void Render(const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& proj);

	// 終了処理
	void Finalize();

	// デバイスに依存するリソースを作成する関数
	void CreateDeviceDependentResources();

	//　床の着地状態をリセット
	void ResetFloorHit();

	// 影を描画するメソッド
	void RenderShadow();

	// 当たり判定の更新
	void UpdateCollision(const AABB& collision, const DirectX::SimpleMath::Vector3& position);
	
	// コライダーの線
	void ColliderLine();

private: 
	// 定数 ----------------------------------------------
	static const float GRAVITY;						///< 重力
	static const float FALL_SPEED;					///< 落下速度
	static const float SEARCH_RANGE;				///< 索敵範囲のしきい値
	static const float NEAR_ZERO_THRESHOLD;			///< 最小距離のしきい値
	static const float MOVE_SPEED;					///< 移動速度
	static const float FALLING_THRESHOLD;			///< 垂直速度が下向きであることを判定するしきい値
	static const float HALF_SCALE;					///< 半分のサイズにする

	static const float TOP_Y_OFFSET_THRESHOLD;		///< 判定対象とする床の高さの許容誤差
	static const float DEFAULT_SURFACE_Y;			///< 床が見つからなかった場合のデフォルトの高さ
	static const float Z_FIGHTING_OFFSET;			///< Zファイティング（チラつき）防止用の微小な浮かせ幅
	static const float SHADOW_SCALE_ATTENUATION;	///< 高さによる影の減衰率
	static const float MIN_SHADOW_SCALE;			///< 影の最小スケール

	static const float FIELD_OF_VIEW_DEGREES;   ///< 視野角
	static const float NEAR_PLANE_DISTANCE;		///< カメラの最前面のクリップ距離
	static const float FAR_PLANE_DISTANCE;		///< カメラの最遠面のクリップ距離

private:
	// メンバ変数 ----------------------------------------
	DX::DeviceResources* m_deviceResources = nullptr;
	DirectX::CommonStates* m_states = nullptr;

	// 敵の状態
	EnemyState m_state = EnemyState::Idle;

	// ビュー行列
	DirectX::SimpleMath::Matrix m_view;

	// 射影行列
	DirectX::SimpleMath::Matrix m_proj;

	// ベーシックエフェクト
	std::unique_ptr<DirectX::BasicEffect> m_basicEffect;

	// プリミティブバッチ
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_primitiveBatch;

	// インプットレイアウト
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

	// モデル
	std::unique_ptr<DirectX::Model> m_enemyModel;

	// モデルの位置
	DirectX::SimpleMath::Vector3 m_enemyPosition;

	// 速度
	float m_enemyVelocity;
	// 垂直方向の速度
	float m_verticalVelocity;

	// スケール
	DirectX::SimpleMath::Vector3 m_enemyScale;
	DirectX::SimpleMath::Vector3 m_damageScale;

	// モデルの平行移動の行列
	DirectX::SimpleMath::Matrix m_enemyTrans;

	// 当たり判定
	AABB m_enemyCollision;
	AABB m_damageCollision;

	// 影
	Shadow m_shadow;

	// 向き
	DirectX::SimpleMath::Vector3 m_enemyForward;

	// 床に触れているかどうか
	bool m_floorHit;

	// 死んだかどうか
	bool m_isDead;

	// 開始位置
	DirectX::SimpleMath::Vector3 m_startPosition;

	// 影描画用のメンバ変数
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState_Shadow;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PS;
	std::unique_ptr<DirectX::Model> m_shadowModel;

	// 床の情報（影を落とすために必要）
	std::vector<DirectX::SimpleMath::Vector3> m_floorPositions;
	std::vector<DirectX::SimpleMath::Vector3> m_floorScales;
	std::vector<DirectX::SimpleMath::Vector3> m_platformPositions;
	std::vector<DirectX::SimpleMath::Vector3> m_platformScales;
};

