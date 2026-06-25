//--------------------------------------------------------------------------------------
// File: Item.h
//
// アイテムクラス
//--------------------------------------------------------------------------------------
#pragma once
#include "SKLib/UserResources.h"


#include "SKLib/CollisionManager.h"
#include "SKLib/InputManager.h"
#include "SKLib/SceneManager.h"

#include "GameObjects/StageObjects/Player.h"

// 列挙体の管理
enum class ItemType
{
	SWORD,
	SHIELD,
	NONE
};

struct ItemData
{
	DirectX::SimpleMath::Vector3 position = { 0.0f,0.0f,0.0f };	///< 位置
	DirectX::SimpleMath::Vector3 scale = { 0.0f,0.0f,0.0f };	///< 大きさ
	ItemType itemType = ItemType::NONE;							///< 種類
	bool isActive = true;										///< 有効かどうか
	std::function<void(Player*)> onPickup;						///< 持ち上げ動作
};

class Item
{
public:
	// ゲッター／セッター -------------------------------------------------------------------
	// --- アイテムデータ ---
	// アイテムのリストを取得
	const std::vector<ItemData>& GetItems() const { return m_items; }
	// 特定インデックスのアイテムの位置を設定
	void SetPosition(size_t index, const DirectX::SimpleMath::Vector3& position);
	// 特定インデックスのアイテムのスケールを設定
	void SetScale(size_t index, const DirectX::SimpleMath::Vector3& scale);

	// --- システム・グラフィックス ---
	// デバイスリソースの設定
	void SetDeviceResources(DX::DeviceResources* deviceResources) { m_deviceResources = deviceResources; }
	// 共通ステートの設定
	void SetCommonStates(DirectX::CommonStates* states) { m_states = states; }

public:
	// 関数 ---------------------------------------------------------------------------------
	// 初期化処理
	void Initialize();

	// アイテムの追加
	void AddItem(const DirectX::SimpleMath::Vector3& position, ItemType type);

	// 更新処理
	void Update(const AABB& playerCollision, Player* player);

	// 描画処理
	void Render(const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj);

	// アイテムのクリア
	void ClearItems();

	// デバイスに依存するリソースを作成する関数
	void CreateDeviceDependentResources();

	// コライダーの線
	void ColliderLine();

private:
	// 定数 ---------------------------------------------------------------------------------
	static const DirectX::SimpleMath::Vector3 DEFAULT_SCALE;///< デフォルトの大きさ

	static const int ITEM_COUNT_SWORD;						///< 剣の使用回数
	static const int ITEM_COUNT_SHIELD;						///< 盾の使用回数

	static const float FIELD_OF_VIEW_DEGREES;				///< 視野角
	static const float NEAR_PLANE_DISTANCE;					///< カメラの最前面のクリップ距離
	static const float FAR_PLANE_DISTANCE;					///< カメラの最遠面のクリップ距離

private:
	// メンバ変数 ---------------------------------------------------------------------------
	// デバイスリソース／コモンステート
	DX::DeviceResources* m_deviceResources = nullptr;
	DirectX::CommonStates* m_states = nullptr;

	// ビュー行列
	DirectX::SimpleMath::Matrix m_view;

	// 射影行列
	DirectX::SimpleMath::Matrix m_proj;

	// 平行移動の行列
	DirectX::SimpleMath::Matrix m_trans;

	// プリミティブバッチ
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_primitiveBatch;

	std::vector<ItemData> m_items;
	std::vector<AABB> m_collisions;

	// モデル
	std::unique_ptr<DirectX::Model> m_swordModel;
	std::unique_ptr<DirectX::Model> m_shieldModel;
};

