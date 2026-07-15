//--------------------------------------------------------------------------------------
// File: CollisionManager.h
//
// 当たり判定の管理クラス
//--------------------------------------------------------------------------------------

#pragma once
class CollisionManager
{
public:
	// 関数 ---------------------------------------------------------------------------------
	// コンストラクタ
	CollisionManager();
	// デストラクタ
	~CollisionManager();
};

// AABB当たり判定
struct AABB
{
public:
	// 関数 ---------------------------------------------------------------------------------
	// コンストラクタ(AABB)
	AABB();
	// デフォルトコンストラクタ(AABB)
	AABB(const DirectX::XMFLOAT3& minPoint, const DirectX::XMFLOAT3& maxPoint);
	// 当たり判定（AABB）の作成
	AABB CreateAABB(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& size);
	// 当たっているかどうか
	bool CheckAABBCollision(const AABB& a, const AABB& b) const;
	// 床との衝突判定
	float GetGroundY(const DirectX::SimpleMath::Vector3& position, const AABB& groundBox);

private:
	// 定数 ---------------------------------------------------------------------------------
	static const float HALF_DIVIDER;	///< サイズから半幅（中心からの半径）を出すための除算値
	static const float INVALID_GROUND_Y;///< 地面の上にいないと判定された場合に返す無効な高さの戻り値

public:
	// メンバ変数 ---------------------------------------------------------------------------
	DirectX::XMFLOAT3 min; // 最小点
	DirectX::XMFLOAT3 max; // 最大点

};

// OBB当たり判定
struct OBB
{
public:
	// 関数 ---------------------------------------------------------------------------------
	// OBBの当たり判定処理
	bool OBBIntersect(const OBB& obb1, const OBB& obb2);

private:
	// 定数 ---------------------------------------------------------------------------------
	static const int DIMENSIONS_3D;	///< // 3次元空間の軸数（X, Y, Zの3種類

private:
	// メンバ変数 ---------------------------------------------------------------------------
	DirectX::XMFLOAT3 center;			// 中心点
	DirectX::XMFLOAT3 halfExtents;		// 半径
	DirectX::XMFLOAT3 orientation[3];	// 回転（三角軸）
};

