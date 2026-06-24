//--------------------------------------------------------------------------------------
// File: Platform.h
//
// 足場クラス
//--------------------------------------------------------------------------------------
#pragma once

#include "SKLib/CollisionManager.h"
#include "GameObjects/StageObjects/Player.h"
#include "GameObjects/StageObjects/Enemy.h"
class Platform
{
public:
	// ゲッター／セッター -------------------------------------------------------------------
	// --- デバイスリソースの設定 ---
	void SetDeviceResources(class DX::DeviceResources* deviceResources)
	{
		m_deviceResources = deviceResources;
	}

	// --- 位置・大きさ ---
	const std::vector<DirectX::SimpleMath::Vector3>& GetPositions() const { return m_positions; }
	const std::vector<DirectX::SimpleMath::Vector3>& GetScales() const { return m_scales; }

public:
	// 関数 ---------------------------------------------------------------------------------
	// コンストラクタ／デストラクタ
	Platform();
	~Platform();

	// 足場情報を登録
	void LoadPlatformData(
		const std::vector<DirectX::SimpleMath::Vector3>& positions,
		const std::vector<DirectX::SimpleMath::Vector3>& scales);

	// 更新処理
	void Update(const std::vector<bool>& switchStates, 
		Player* player, std::vector<std::unique_ptr<Enemy>>& enemies);

	// 描画処理
	void Render(const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& proj);

	//デバイスに依存するリソースを作成する関数
	void CreateDeviceDependentResources();

	// コライダーの線
	void ColliderLine();

private:
	// 定数 ---------------------------------------------------------------------------------
	static const float HALF_SCALE;					///< 半分のサイズにする

private:
	// メンバ変数 ---------------------------------------------------------------------------
	// デバイスリソース／コモンステート
	class DX::DeviceResources* m_deviceResources = nullptr;
	DirectX::CommonStates* m_states = nullptr;

	// モデル
	std::shared_ptr<DirectX::Model> m_model;

	// 足場情報
	std::vector<DirectX::SimpleMath::Vector3> m_positions; // 位置
	std::vector<DirectX::SimpleMath::Vector3> m_scales;	   // 大きさ
	std::vector<AABB> m_collisions;						   // 当たり判定

	std::vector<bool> m_switchStates;

	// プリミティブバッチ
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_primitiveBatch;
};

