//--------------------------------------------------------------------------------------
// File: GimmickBlock.h
//
// 仕掛けブロッククラス
//--------------------------------------------------------------------------------------

#pragma once
#include "SKLib/CollisionManager.h"

enum class BlockType
{
	PLATFORM,	///< 足場
	KEY,		///< カギ
	PORTAL,		///< ポータル
	ITEM,		///< アイテム
	NONE		///< なし
};

class GimmickBlock
{
public:
	// ゲッター／セッター -------------------------------------------------------------------
	// --- 当たり判定処理 ---
	// 当たり判定の取得
	const AABB& GetCollision() const;	
	// 当たり判定の大きさの取得
	const DirectX::SimpleMath::Vector3& GetAABBSize() const;

	// --- 追尾状態 ---
	// 追尾しているかどうか
	bool IsFollowing() const { return m_isFollowing; }
	// ターゲットの座標を更新
	void UpdateTargetPosition(const DirectX::SimpleMath::Vector3& targetPos) { m_targetPosition = targetPos; }

	// --- 追尾オフセット ---
	// 追尾オフセットの取得
	const DirectX::SimpleMath::Vector3& GetFollowOffset() const { return m_followOffset; }	
	// 追尾オフセットの設定
	void SetFollowOffset(const DirectX::SimpleMath::Vector3& offset) { m_followOffset = offset; }

	// --- パーティクル・設置状態 ---
	// 置かれた瞬間かどうか取得
	bool IsJustPlaced() const { return m_isJustPlaced; }
	// 置かれた瞬間のフラグをクリア
	void ClearJustPlaced() { m_isJustPlaced = false; }

	// --- ブロック種類  ---
	// ブロックの種類を取得
	BlockType GetType() const { return m_type; }

	// --- 表示・有効状態 ---
	// ブロックが有効かどうかを取得
	bool GetIsVisible() const { return m_isVisible; }
	// ブロックの有効化を設定
	void SetIsVisible(bool visible) { m_isVisible = visible; }

	// --- 座標 ---
	// 座標を取得
	DirectX::SimpleMath::Vector3 GetPosition() const { return m_position; }
	// 座標を設定
	void SetPosition(const DirectX::SimpleMath::Vector3& position);

	// --- 大きさ  ---
	// 大きさを取得
	DirectX::SimpleMath::Vector3 GetScale() const { return m_scale; }
	// 大きさを設定
	void SetScale(const DirectX::SimpleMath::Vector3& scale);

	// --- 行列 ---
	// ワールド行列を取得
	DirectX::SimpleMath::Matrix GetWorldMatrix() const;
public:
	// 関数 ---------------------------------------------------------------------------------
	// コンストラクタ
	GimmickBlock();
	// モデルの読み込み
	void Initialize(std::shared_ptr<DirectX::Model> model, const DirectX::SimpleMath::Vector3& pos, 
		const DirectX::SimpleMath::Vector3& scale,BlockType type);
	// 更新処理
	void Update(float elapsedTime);
	// ブロックの処理を有効化する
	void OnActivate();
	// 当たり判定の更新
	void UpdateCollision();										
	// 当たり判定のチェック
	bool CheckCollision(const AABB& otherAABB);					
	// 追尾を開始
	void StartFollowing(const DirectX::SimpleMath::Vector3& targetPos, float speed);
	// 追尾を止める
	void StopFollowing(const DirectX::SimpleMath::Vector3& stopPosition);			
	// 追尾のリセット
	void Reset();
	// 追尾の更新
	void UpdateFollowing(float elapsedTime);										
	// コライダーの線
	void ColliderLine();
private:
	// 定数 ---------------------------------------------------------------------------------
	static const float DEFAULT_FOLLOW_OFFSET_Y; ///< 追尾時のデフォルトの高さオフセット
	static const float UNIT_CUBE_SIZE;			///< AABBの基準となる単位立方体のサイズ
	static const float STOP_POSITION_OFFSET_Y;	///< 停止位置から下げる高さのオフセット量

private:
	// メンバ変数 ---------------------------------------------------------------------------
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
	// ブロックが置かれたかどうか
	bool m_isJustPlaced;
	// オブジェクトが有効かどうか
	bool m_isVisible = true;
	// 追尾しているかどうか
	bool m_isFollowing = false;		
	// 追尾対象の座標
	DirectX::SimpleMath::Vector3 m_targetPosition;
	//追尾速度
	float m_followSpeed;
	// 追尾オフセット
	DirectX::SimpleMath::Vector3 m_followOffset;
	// 運んでいるかどうか
	bool m_isCarried;
	// ブロックの種類
	BlockType m_type = BlockType::NONE;
};