//--------------------------------------------------------------------------------------
// File: GimmickBlock.h
//
// 仕掛けブロッククラス
//--------------------------------------------------------------------------------------

#pragma once
#include "SKLib/CollisionManager.h"

enum class BlockType
{
	PLATFORM,	// 足場
	KEY,		// カギ
	PORTAL,		// ポータル
	ITEM,		// アイテム
	NONE		// なし
};

class GimmickBlock
{
public:
	GimmickBlock();

	// モデルの読み込み
	void Initialize(std::shared_ptr<DirectX::Model> model, const DirectX::SimpleMath::Vector3& pos, 
		const DirectX::SimpleMath::Vector3& scale,BlockType type);

	// 更新処理
	void Update(float elapsedTime);

	// ブロックの処理を有効化する
	void OnActivate();

	// 当たり判定関連
	void UpdateCollision();
	bool CheckCollision(const AABB& otherAABB);
	const AABB& GetCollision() const;
	const DirectX::SimpleMath::Vector3& GetAABBSize() const;

	// 追尾関連
	void StartFollowing(const DirectX::SimpleMath::Vector3& targetPos, float speed);		///< 追尾を開始
	void StopFollowing(const DirectX::SimpleMath::Vector3& stopPosition);					///< 追尾を止める
	void Reset();
	void UpdateFollowing(float elapsedTime);												///< 追尾を更新
	void UpdateTargetPosition(const DirectX::SimpleMath::Vector3& targetPos) 
	{ m_targetPosition = targetPos; }
	
	bool IsFollowing() const{ return m_isFollowing; }											///< 追尾しているかどうか
	void SetFollowOffset(const DirectX::SimpleMath::Vector3& offset){ m_followOffset = offset; }///< 追尾オフセットの設定
	const DirectX::SimpleMath::Vector3& GetFollowOffset() const { return m_followOffset; }


	BlockType GetType() const { return m_type; }
	bool GetIsVisible() const { return m_isVisible; }
	DirectX::SimpleMath::Vector3 GetPosition() const { return m_position; }
	DirectX::SimpleMath::Vector3 GetScale() const { return m_scale; }
	DirectX::SimpleMath::Matrix GetWorldMatrix() const;

	// 設定メソッド
	void SetPosition(const DirectX::SimpleMath::Vector3& position);
	void SetScale(const DirectX::SimpleMath::Vector3& scale);
	void SetIsVisible(bool visible) { m_isVisible = visible; }

	// コライダーの線
	void ColliderLine();
private:
	// プリミティブバッチ
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_primitiveBatch;

	// 初期値
	DirectX::SimpleMath::Vector3 m_initialPosition;
	
	// ブロックのモデル
	std::shared_ptr<DirectX::Model> m_model;

	// ブロックの位置
	DirectX::SimpleMath::Vector3 m_position;

	// ブロックのスケール
	DirectX::SimpleMath::Vector3 m_scale;

	// ブロックの平行移動の行列
	DirectX::SimpleMath::Matrix m_trans;

	// 当たり判定
	DirectX::SimpleMath::Vector3 m_aabbSize;
	AABB m_collision;

	// オブジェクトが有効かどうか
	bool m_isVisible = true;

	// 追尾関連
	bool m_isFollowing = false;						
	DirectX::SimpleMath::Vector3 m_targetPosition;
	float m_followSpeed;
	DirectX::SimpleMath::Vector3 m_followOffset;
	bool m_isCarried;

	// ブロックの種類
	BlockType m_type = BlockType::NONE;
};