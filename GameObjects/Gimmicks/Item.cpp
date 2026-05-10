//--------------------------------------------------------------------------------------
// File: Item.cpp
//
// アイテムクラス
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "Item.h"

/*
* @brief 初期化処理
*
* @param[in]  なし
* 
* @return なし
*/
void Item::Initialize()
{
	CreateDeviceDependentResources();

	m_items.clear();
	m_collisions.clear();
}

/*
* @brief アイテムの追加
*
* @param[in]  position 位置
* @param[in]  type     アイテムの種類
*
* @return なし
*/
void Item::AddItem(const DirectX::SimpleMath::Vector3& position, ItemType type)
{
	ItemData newItem;
	newItem.position = position;
	newItem.scale = { 1.0f,1.0f,1.0f };
	newItem.itemType = type;
	newItem.isActive = true;

	newItem.onPickup = [type](Player* p)
		{
			switch (type)
			{
			case ItemType::SWORD:

				// 剣を拾ったときの効果
				p->AddAttack(3);
				break;

			case ItemType::SHIELD:

				// 盾を拾ったときの効果
				p->AddDefence(3);
				break;

			default:
				break;
			}
		};

	m_items.push_back(newItem);

	// コリジョンを追加
	m_collisions.push_back(AABB().CreateAABB(newItem.position, newItem.scale));
}

/*
* @brief 更新処理
*
* @param[in]  playerCollision プレイヤーの当たり判定
* @param[in]  player		　プレイヤーオブジェクトのポインタ
* 
* * @return なし
*/
void Item::Update(const AABB& playerCollision, Player* player)
{
	for (size_t i = 0; i < m_items.size(); ++i)
	{
		if (!m_items[i].isActive)
			continue;

		// 当たり判定の更新
		m_collisions[i] = m_collisions[i].CreateAABB(m_items[i].position, m_items[i].scale);

		// 当たり判定チェック
		if (m_collisions[i].CheckAABBCollision(playerCollision, m_collisions[i]))
		{
			// アイテムを入手
			if (InputManager::Get().IsKeyPressed(DirectX::Keyboard::E))
			{
				if (m_items[i].onPickup)
				{
					m_items[i].onPickup(player);
				}
				// アイテムを消す
				m_items[i].isActive = false;
			}
		}
	}
}

/*
* @brief　描画処理
*
* @param[in]  view ビュー行列
* @param[in]  proj 射影行列
* 
* @return なし
*/
void Item::Render(const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj)
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	auto states = m_states;

	for (const auto& item : m_items)
	{
		if (!item.isActive) continue;

		// スケールと平行移動行列を作成
		DirectX::SimpleMath::Matrix scale = DirectX::SimpleMath::Matrix::CreateScale(item.scale);
		DirectX::SimpleMath::Matrix trans = DirectX::SimpleMath::Matrix::CreateTranslation(item.position);

		// ワールド行列
		DirectX::SimpleMath::Matrix world = scale * trans;

		// 各アイテムの描画
		switch (item.itemType)
		{
		case ItemType::SWORD:  // 剣の描画
			m_swordModel->Draw(context, *states, world, view, proj);
			break;

		case ItemType::SHIELD: // 盾の描画
			m_shieldModel->Draw(context, *states, world, view, proj);
			break;

		default:
			break;
		}
	}
}

/*
* @brief　アイテムのクリア
*
* @param[in]  なし
*
* @return なし
*/
void Item::ClearItems()
{
	m_items.clear();
	m_collisions.clear();
}

/*
* @brief　デバイスに依存するリソースを作成する関数
*
* @param[in]  なし
* 
* @return なし
*/
void Item::CreateDeviceDependentResources()
{
	if (!m_deviceResources) return;

	auto device = m_deviceResources->GetD3DDevice();
	auto context = m_deviceResources->GetD3DDeviceContext();

	// エフェクトファクトリーの作成
	std::unique_ptr<DirectX::EffectFactory> fx = std::make_unique<DirectX::EffectFactory>(device);
	fx->SetDirectory(L"Resources/Models");

	// プリミティブバッチの初期化
	m_primitiveBatch = std::make_unique<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>(context);

	// 各モデルの読み込み
	m_swordModel = DirectX::Model::CreateFromSDKMESH(device, L"Resources/Models/sword.sdkmesh", *fx);
	m_shieldModel = DirectX::Model::CreateFromSDKMESH(device, L"Resources/Models/shield.sdkmesh", *fx);

	// 射影行列の作成
	RECT rect = m_deviceResources->GetOutputSize();
	m_proj = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(
		DirectX::XMConvertToRadians(45.0f),
		static_cast<float>(rect.right) / static_cast<float>(rect.bottom),
		0.1f, 100.0f
	);
}

/*
* @brief　位置を設定
*
* @param[in]  index アイテムのインデックス
* @param[in]  position アイテムの座標
* 
* @return なし
*/
void Item::SetPosition(size_t index, const DirectX::SimpleMath::Vector3& position)
{
	if (index < m_items.size())
	{
		m_items[index].position = position;
	}
}

/*
* @brief　スケールを設定
*
* @param[in]  index アイテムのインデックス
* @param[in]  scale アイテムの大きさ
* 
* @return なし
*/
void Item::SetScale(size_t index, const DirectX::SimpleMath::Vector3& scale)
{
	if (index < m_items.size())
	{
		m_items[index].scale = scale;
	}
}

/*
* @brief コライダーの線
*
* @param[in]  なし
* 
* @return なし
*/
void Item::ColliderLine()
{
	if (!m_primitiveBatch) return;

	// コライダー線の色
	auto lineColor = DirectX::Colors::BlueViolet;
	m_primitiveBatch->Begin();

	// AABBの頂点を計算
	for (const auto& collision : m_collisions)
	{
		DirectX::SimpleMath::Vector3 corners[8] =
		{
			{ collision.min.x, collision.min.y, collision.min.z },
			{ collision.max.x, collision.min.y, collision.min.z },
			{ collision.max.x, collision.max.y, collision.min.z },
			{ collision.min.x, collision.max.y, collision.min.z },
			{ collision.min.x, collision.min.y, collision.max.z },
			{ collision.max.x, collision.min.y, collision.max.z },
			{ collision.max.x, collision.max.y, collision.max.z },
			{ collision.min.x, collision.max.y, collision.max.z }
		};

		// 前面の線を描画
		m_primitiveBatch->DrawLine({ corners[0], lineColor }, { corners[1], lineColor });
		m_primitiveBatch->DrawLine({ corners[1], lineColor }, { corners[2], lineColor });
		m_primitiveBatch->DrawLine({ corners[2], lineColor }, { corners[3], lineColor });
		m_primitiveBatch->DrawLine({ corners[3], lineColor }, { corners[0], lineColor });

		// 背面の線を描画
		m_primitiveBatch->DrawLine({ corners[4], lineColor }, { corners[5], lineColor });
		m_primitiveBatch->DrawLine({ corners[5], lineColor }, { corners[6], lineColor });
		m_primitiveBatch->DrawLine({ corners[6], lineColor }, { corners[7], lineColor });
		m_primitiveBatch->DrawLine({ corners[7], lineColor }, { corners[4], lineColor });

		// 前面と背面をつなぐ線を描画
		m_primitiveBatch->DrawLine({ corners[0], lineColor }, { corners[4], lineColor });
		m_primitiveBatch->DrawLine({ corners[1], lineColor }, { corners[5], lineColor });
		m_primitiveBatch->DrawLine({ corners[2], lineColor }, { corners[6], lineColor });
		m_primitiveBatch->DrawLine({ corners[3], lineColor }, { corners[7], lineColor });

	}
	m_primitiveBatch->End();
}
