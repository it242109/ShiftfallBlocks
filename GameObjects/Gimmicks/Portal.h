//--------------------------------------------------------------------------------------
// File: Portal.h
//
// ポータルクラス
//--------------------------------------------------------------------------------------
#pragma once

#include "SKLib/CollisionManager.h"
#include "SKLib/SoundManager.h"

// 列挙体の定義
enum class PortalTargetType
{
	MOVEON, // 進む
	RETURN, // 戻る
	NONE
};

struct PortalData
{
	DirectX::SimpleMath::Vector3 position;
	DirectX::SimpleMath::Vector3 scale;
	PortalTargetType portalType;
	std::function<void()> onTeleport;
};

class Portal
{
public:
	// 初期化処理
	void Initialize(ID3D11DeviceContext* context);

	// 更新処理
	void Update(const AABB& playerCollision, float elapsedTime, const std::vector<bool>& portalStates);

	// 描画処理
	void Render(ID3D11DeviceContext* context, const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& proj, DirectX::CommonStates* states,
		const std::vector<bool>& portalStates);

	// 各ポータルの追加処理
	void AddPortal(const PortalData& data, int setIndex);

	// ポータルが作動したら
	bool IsTeleporting() const { return m_isTeleporting; }

	bool IsTeleported() const { return m_isTeleported; }
	void SetTeleported(bool state) { m_isTeleported = state; }

	// モデルの設定（セッター）
	void SetModel(DirectX::Model* model) { m_model = model; }

	// ポータルデータの取得
	const std::vector<PortalData>& GetPortals() const { return m_portals; }

	// ポータルセットの数を取得
	int GetPortalSetCount() const;

	// ポータルをセットに割り当てる
	void AssignPortalToSet(size_t portalIndex, int setIndex);

	// コライダーの線
	void ColliderLine(const std::vector<bool>& portalStates);

private:
	// プリミティブバッチ
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_primitiveBatch;

	// 各ポータル
	std::vector<PortalData> m_portals;
	std::vector<AABB> m_collisions;

	// テレポート時の変数
	bool m_isTeleporting = false;
	bool m_isTeleported = false;
	float m_teleportTimer = 0.0f;

	// モデル
	DirectX::Model* m_model = nullptr;

	// ポータルセットのインデックスを管理するベクター
	std::vector<int> m_portalSetIndices;
};

