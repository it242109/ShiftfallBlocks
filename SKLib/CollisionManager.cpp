//--------------------------------------------------------------------------------------
// File: CollisionManager.cpp
//
// 当たり判定の管理クラス
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "CollisionManager.h"

/*
* @brief コンストラクタ
*
* @param[in]  なし
*
* @return なし
*/
CollisionManager::CollisionManager()
{
}

/*
* @brief デストラクタ
*
* @param[in]  なし
*
* @return なし
*/
CollisionManager::~CollisionManager()
{
}

/*
* @brief デフォルトコンストラクタ（AABB）
*
* @param[in]  なし
*
* @return なし
*/
AABB::AABB()
	:
	min(),
	max()
{
}
/*
* @brief コンストラクタ（AABB）
*
* @param[in]  minPoint 最小値の座標
* @param[in]  maxPoint 最大値の座標
*
* @return なし
*/
AABB::AABB(const DirectX::XMFLOAT3& minPoint, const DirectX::XMFLOAT3& maxPoint)
	: 
	min(minPoint),
	max(maxPoint)
{
}

/*
* @brief AABB当たり判定の作成
*
* @param[in]  position 位置
* @param[in]  size     大きさ
*
* @return 当たり判定（AABB）構造体
*/
AABB AABB::CreateAABB(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& size)
{
	AABB box;

	// 最小値は座標の位置にサイズ÷2を引いた値
	box.min = 
	{ 
		position.x - size.x / 2,
		position.y - size.y / 2,
		position.z - size.z / 2 
	};

	// 最大値は座標の位置にサイズ÷2を足した値
	box.max =
	{
		position.x + size.x / 2,
		position.y + size.y / 2,
		position.z + size.z / 2
	};

	return box;
}

/*
* @brief AABB当たり判定
*
* @param[in]  a 当たり判定の構造体A
* @param[in]  b 当たり判定の構造体B
*
* @return 当たり判定の結果
*/
bool AABB::CheckAABBCollision(const AABB& a, const AABB& b) const
{
	return  (a.max.x >= b.min.x && a.min.x <= b.max.x) &&
			(a.max.y >= b.min.y && a.min.y <= b.max.y) &&
			(a.max.z >= b.min.z && a.min.z <= b.max.z);

}

/*
* @brief 床との衝突判定
*
* @param[in]  position 位置
* @param[in]  groundBox 床の当たり判定の構造体
*
* @return 当たり判定の結果
*/
float AABB::GetGroundY(const DirectX::SimpleMath::Vector3& position, const AABB& groundBox)
{
	if (position.x >= groundBox.min.x && position.x <= groundBox.max.x &&
		position.z >= groundBox.min.z && position.z <= groundBox.max.z)
	{
		if (position.y >= groundBox.min.y)
		{
			// 地面の天面の高さを返す
			return groundBox.max.y; 
		}
	}
	return -10000.0f;
}

/*
* @brief OBB当たり判定
*
* @param[in]  obbA OBBの当たり判定の構造体A
* @param[in]  obbB OBBの当たり判定の構造体B
*
* @return 当たり判定の結果
*/
bool OBB::OBBIntersect(const OBB& obbA, const OBB& obbB)
{
	// OBBの中心感のベクトル
	DirectX::XMFLOAT3 diff =
	{
		obbB.center.x - obbA.center.x,
		obbB.center.y - obbA.center.y,
		obbB.center.z - obbA.center.z,

	};

	// 各軸に対しての判定
	for (int i = 0; i < 3; ++i)
	{
		// OBB1の軸
		DirectX::XMFLOAT3 axis1 = obbA.orientation[i];
		// OBB2の軸
		DirectX::XMFLOAT3 axis2 = obbB.orientation[i];

		// 投影の計算
		float r1 = obbA.halfExtents.x * fabs(axis1.x) + 
			obbA.halfExtents.y * fabs(axis1.y) +
			obbA.halfExtents.z * fabs(axis1.z);

		float r2 = obbB.halfExtents.x * fabs(axis1.x) + 
			obbB.halfExtents.y * fabs(axis1.y) +
			obbB.halfExtents.z * fabs(axis1.z);

		// 中心間のベクトルを軸に投影
		float projection = fabs(diff.x * axis1.x + diff.y * axis1.y + diff.z * axis1.z);

		// 重なり判定
		if (projection > r1 + r2)
		{
			return false;
		}

		// OBB1の軸とOBB2の半軸の組み合わせをチェック
		for (int j = 0; j < 3; ++j)
		{
			DirectX::XMFLOAT3 axis =
			{
				axis1.y * axis2.z - axis1.z * axis2.y,
				axis1.z * axis2.x - axis1.x * axis2.z,
				axis1.x * axis2.y - axis1.y * axis2.x
			};

			 r1 = obbA.halfExtents.x * fabs(axis1.y * axis.z) + 
				obbA.halfExtents.y * fabs(axis1.z * axis.x) + 
				obbA.halfExtents.z * fabs(axis1.x * axis.y);			
			 r2 = obbB.halfExtents.x * fabs(axis2.y * axis.z) + 
				obbB.halfExtents.y * fabs(axis2.z * axis.x) + 
				obbB.halfExtents.z * fabs(axis2.x * axis.y);

			projection = fabs(diff.x * axis.x + diff.y * axis.y + diff.z * axis.z);

			// 重なり判定
			if (projection > r1 + r2)
			{
				return false; 
			}
		}
	}
	// 全ての軸で重なりが確認できた場合
	return true;
}
