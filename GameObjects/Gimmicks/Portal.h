//--------------------------------------------------------------------------------------
// File: Portal.h
//
// ポータルクラス
//--------------------------------------------------------------------------------------
#pragma once

#include "SKLib/CollisionManager.h"
#include "SKLib/SoundManager.h"

// 列挙体の管理：ポータルの戻る／進む
enum class PortalTargetType
{
	MOVEON, // 進む
	RETURN, // 戻る
	NONE
};

// ポータルデータの管理
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
	// ゲッター／セッター -------------------------------------------------------------------
	// --- テレポート状態  ---
	// ポータルが作動中（移動演出中など）かどうか
	bool IsTeleporting() const { return m_isTeleporting; }
	// テレポートが完了したかどうかを取得
	bool IsTeleported() const { return m_isTeleported; }
	// テレポートの完了状態を設定
	void SetTeleported(bool state) { m_isTeleported = state; }

	// --- ポータルデータ管理 ---
	// ポータル全体のリストを取得
	const std::vector<PortalData>& GetPortals() const { return m_portals; }
	// 入り口と出口のペアの数を取得
	int GetPortalSetCount() const;

	// --- グラフィックリソース ---
	// モデルの設定（セッター）
	void SetModel(DirectX::Model* model) { m_model = model; }

public:
	// 関数 ---------------------------------------------------------------------------------
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

	// ポータルをセットに割り当てる
	void AssignPortalToSet(size_t portalIndex, int setIndex);

	// コライダーの線
	void ColliderLine(const std::vector<bool>& portalStates);

private:
	// 定数 ---------------------------------------------------------------------------------
	static const float TIMER_END_THRESHOLD;		///< タイマーが終了したと判定する基準値
	static const float INVALID_SET_INDEX;		///< ポータルセットが指定されていない場合のインデックス値
	static const float HALF_SCALE;				///< 半分のサイズにする
	static const float TELEPORT_COOLDOWN_TIME;	///< テレポート発動後のクールダウン時間

private:
	// メンバ変数 ---------------------------------------------------------------------------
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

