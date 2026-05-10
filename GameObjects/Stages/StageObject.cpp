//--------------------------------------------------------------------------------------
// File: StageObject.cpp
//
// ステージオブジェクト（壁・床）クラス
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "StageObject.h"
#include "Player.h"
#include "Enemy.h"

/*
* @brief コンストラクタ
*
* @param[in]  なし
*
* @return なし
*/
StageObject::StageObject()
{
}

/*
* @brief デストラクタ
*
* @param[in]  なし
*
* @return なし
*/
StageObject::~StageObject()
{
}

/*
* @brief 初期化処理
*
* @param[in]  device　Direct3Dデバイスのポインタ
* @param[in]  path   モデルのパス 
*
* @return なし
*/
void StageObject::Initialize(ID3D11Device* device, const wchar_t* path)
{
	std::unique_ptr<DirectX::EffectFactory> fx = std::make_unique<DirectX::EffectFactory>(device);
	fx->SetDirectory(L"Resources/Models");

	// モデルの読み込み
	m_model = DirectX::Model::CreateFromSDKMESH(device, path, *fx);
}

void StageObject::LoadData(const std::vector<DirectX::SimpleMath::Vector3>& positions, const std::vector<DirectX::SimpleMath::Vector3>& scales)
{
	m_positions = positions;
	m_scales = scales;
	m_collisions.clear();
	for (size_t i = 0; i < m_positions.size(); ++i) 
	{
		DirectX::SimpleMath::Vector3 half = m_scales[i] * 0.5f;
		m_collisions.push_back(AABB(m_positions[i] - half, m_positions[i] + half));
	}
}

/*
* @brief 更新処理
*
* @param[in]  player プレイヤーのポインタ
* @param[in]  enemy  複数の敵のポインタを格納したベクター
*
* @return なし
*/
void StageObject::Update(Player* player, const std::vector<std::unique_ptr<Enemy>>& enemies)
{
	// プレイヤーと敵が存在しない場合は処理をスキップ
	if (!player && enemies.empty()) return;

	// 壁・床の当たり判定処理
	for (size_t i = 0; i < m_collisions.size(); ++i)
	{
		if (player)
		{
			player->UpdateCollision(m_collisions[i], m_positions[i]);
		}
		for (auto& enemy : enemies)
		{
			if (enemy)
			{
				enemy->UpdateCollision(m_collisions[i], m_positions[i]);
			}
		}
	}
}

/*
* @brief 描画処理
*
* @param[in]  context デバイスコンテキスト
* @param[in]  view    ビュー行列
* @param[in]  proj    射影行列
* @param[in]  states  共通ステートオブジェクト
*
* @return なし
*/
void StageObject::Render(ID3D11DeviceContext* context, const DirectX::SimpleMath::Matrix& view, 
	const DirectX::SimpleMath::Matrix& proj, DirectX::CommonStates* states)
{
	if (!m_model) return;

	// 壁・床の描画処理
	for (size_t i = 0; i < m_positions.size(); ++i) 
	{
		DirectX::SimpleMath::Matrix world = DirectX::SimpleMath::Matrix::CreateScale(m_scales[i]) * DirectX::SimpleMath::Matrix::CreateTranslation(m_positions[i]);
		m_model->Draw(context, *states, world, view, proj);
	}
}

/*
* @brief デバイスに依存するリソースを作成する関数
*
* @param[in]  なし
*
* @return なし
*/
void StageObject::CreateDeviceDependentResources()
{
	auto context = m_deviceResources->GetD3DDeviceContext();

	// プリミティブバッチの初期化
	m_primitiveBatch = std::make_unique<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>(context);
}

/*
* @brief コライダーの線
*
* @param[in]  なし
*
* @return なし
*/
void StageObject::ColliderLine()
{
	if (!m_primitiveBatch) return;

	auto lineColor = DirectX::Colors::Black;
	m_primitiveBatch->Begin();

	// 複数のオブジェクト
	for (const auto& col : m_collisions)
	{
		DirectX::SimpleMath::Vector3 platformCorners[8] =
		{
			{ col.min.x, col.min.y, col.min.z },
			{ col.max.x, col.min.y, col.min.z },
			{ col.max.x, col.max.y, col.min.z },
			{ col.min.x, col.max.y, col.min.z },
			{ col.min.x, col.min.y, col.max.z },
			{ col.max.x, col.min.y, col.max.z },
			{ col.max.x, col.max.y, col.max.z },
			{ col.min.x, col.max.y, col.max.z },
		};

		/*/////////////////////////////////////線の描画///////////////////////////////////////////*/

		// 前面の線を描画
		m_primitiveBatch->DrawLine({ platformCorners[0], lineColor }, { platformCorners[1], lineColor });
		m_primitiveBatch->DrawLine({ platformCorners[1], lineColor }, { platformCorners[2], lineColor });
		m_primitiveBatch->DrawLine({ platformCorners[2], lineColor }, { platformCorners[3], lineColor });
		m_primitiveBatch->DrawLine({ platformCorners[3], lineColor }, { platformCorners[0], lineColor });

		// 背面の線を描画
		m_primitiveBatch->DrawLine({ platformCorners[4], lineColor }, { platformCorners[5], lineColor });
		m_primitiveBatch->DrawLine({ platformCorners[5], lineColor }, { platformCorners[6], lineColor });
		m_primitiveBatch->DrawLine({ platformCorners[6], lineColor }, { platformCorners[7], lineColor });
		m_primitiveBatch->DrawLine({ platformCorners[7], lineColor }, { platformCorners[4], lineColor });

		// 前面と背面をつなぐ線を描画
		m_primitiveBatch->DrawLine({ platformCorners[0], lineColor }, { platformCorners[4], lineColor });
		m_primitiveBatch->DrawLine({ platformCorners[1], lineColor }, { platformCorners[5], lineColor });
		m_primitiveBatch->DrawLine({ platformCorners[2], lineColor }, { platformCorners[6], lineColor });
		m_primitiveBatch->DrawLine({ platformCorners[3], lineColor }, { platformCorners[7], lineColor });

		/*/////////////////////////////////////線の描画///////////////////////////////////////////*/
	}

	m_primitiveBatch->End();
}
