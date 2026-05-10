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

// 列挙体の宣言
enum class EnemyState
{
	Idle,
	Chase,
	Attack,
	Dead
};

class Enemy
{
// 関数
public:
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

	//////////////////////////////ゲッター／セッター////////////////////////////
	// 位置を取得
	const DirectX::SimpleMath::Vector3& GetPosition() const { return m_enemyPosition; }

	// 移動速度を取得
	float GetVelocity() const { return m_enemyVelocity; }

	// 重力を取得
	const float GetGravity() const { return GRAVITY; }

	// 当たり判定を取得
	const AABB& GetCollision() const { return m_enemyCollision; }
	const AABB& GetDamageCollision() const { return m_damageCollision; }

	void SetDeviceResources(DX::DeviceResources* deviceResources) { m_deviceResources = deviceResources; };
	void SetCommonStates(DirectX::CommonStates* states) { m_states = states; };

	// 床情報を設定するメソッド
	void SetFloorData(const std::vector<DirectX::SimpleMath::Vector3>& f_positions,
		const std::vector<DirectX::SimpleMath::Vector3>& f_scales);

	// 足場情報を設定するメソッド
	void SetPlatformData(const std::vector<DirectX::SimpleMath::Vector3>& pf_positions,
		const std::vector<DirectX::SimpleMath::Vector3>& pf_scales);

	// 位置を設定
	void SetPosition(float x, float y, float z);

	// 床接触状態を設定
	void SetFloorHit(bool hit) { m_floorHit = hit; };

	// 床に接触しているかどうか
	bool IsOnFloor() const { return m_floorHit; }

	// 死んだかどうか
	bool IsDead() const { return m_isDead; }
	void EnemyKill();
	void Reset(const DirectX::SimpleMath::Vector3& pos);
	////////////////////////////////////////////////////////////////////////////

	// コライダーの線
	void ColliderLine();

// 定数
private: 
	// 重力
	static constexpr float GRAVITY = -9.8f;
	static constexpr float FALL_SPEED = -3.0f;

	// ジャンプ
	static constexpr float JUMP_POWER = 5.5f;

// メンバ変数
private:
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

