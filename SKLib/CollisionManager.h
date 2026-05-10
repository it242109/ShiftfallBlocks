//--------------------------------------------------------------------------------------
// File: CollisionManager.h
//
// 当たり判定の管理クラス
//--------------------------------------------------------------------------------------

#pragma once
class CollisionManager
{
public:
	// コンストラクタ
	CollisionManager();

	// デストラクタ
	~CollisionManager();
};

// AABB当たり判定
struct AABB
{
public:
	
	DirectX::XMFLOAT3 min; // 最小点
	DirectX::XMFLOAT3 max; // 最大点

public:
	// コンストラクタ(AABB)
	AABB(); 
	// デフォルトコンストラクタ(AABB)
	AABB(const DirectX::XMFLOAT3& minPoint, const DirectX::XMFLOAT3& maxPoint);

public:
	// 当たり判定（AABB）の作成
	AABB CreateAABB(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& size);

	// 当たっているかどうか
	bool CheckAABBCollision(const AABB& a, const AABB& b) const;

	// 床との衝突判定
	float GetGroundY(const DirectX::SimpleMath::Vector3& position, const AABB& groundBox);
};

// OBB当たり判定
struct OBB
{
private:
	DirectX::XMFLOAT3 center;			// 中心点
	DirectX::XMFLOAT3 halfExtents;		// 半径
	DirectX::XMFLOAT3 orientation[3];	// 回転（三角軸）

public:

	bool OBBIntersect(const OBB& obb1, const OBB& obb2);
};

