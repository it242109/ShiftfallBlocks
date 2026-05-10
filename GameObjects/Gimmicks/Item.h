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

#include "GameObjects/Stages/Player.h"

// 列挙体の管理
enum class ItemType
{
	SWORD,
	SHIELD,
	NONE
};

struct ItemData
{
	DirectX::SimpleMath::Vector3 position = { 0.0f,0.0f,0.0f };
	DirectX::SimpleMath::Vector3 scale = { 0.0f,0.0f,0.0f };
	ItemType itemType = ItemType::NONE;
	bool isActive = true;
	std::function<void(Player*)> onPickup;
};

class Item
{
public:
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

	//////////////////////////////ゲッター／セッター///////////////////////////

	// デバイスリソースの設定
	void SetDeviceResources(DX::DeviceResources* deviceResources) { m_deviceResources = deviceResources; }

	// 共通ステートの設定
	void SetCommonStates(DirectX::CommonStates* states) { m_states = states; }

	// 位置を設定
	void SetPosition(size_t index, const DirectX::SimpleMath::Vector3& position);

	// スケールを設定
	void SetScale(size_t index, const DirectX::SimpleMath::Vector3& scale);

	// アイテムのリストを取得
	const std::vector<ItemData>& GetItems() const { return m_items; }
	
	/////////////////////////////////////////////////////////////////////////

	// コライダーの線
	void ColliderLine();

private:
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

