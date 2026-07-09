//--------------------------------------------------------------------------------------
// File: Portal.cpp
//
// ポータルクラス
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "Portal.h"
#include "Item.h"

// 定数の定義
const float Portal::TIMER_END_THRESHOLD = 0.0f;		///< タイマーが終了したと判定する基準値
const float Portal::INVALID_SET_INDEX = -1;			///< ポータルセットが指定されていない場合のインデックス値
const float Portal::HALF_SCALE = 0.5f;				///< 半分のサイズにする
const float Portal::TELEPORT_COOLDOWN_TIME = 2.0f;	///< テレポート発動後のクールダウン時間

/*
* @brief 初期化処理
*
* @param[in]  context デバイスコンテキスト
*
* @return なし
*/
void Portal::Initialize(ID3D11DeviceContext* context)
{
	m_portals.clear();
	m_collisions.clear();
	m_isTeleporting = false;
	m_isTeleported = false;
	m_teleportTimer = 0.0f;

	m_primitiveBatch = std::make_unique<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>(context);
}

/*
* @brief 更新処理
*
* @param[in]  playerCollision プレイヤーの当たり判定
* @param[in]  elapsedTime 前フレームからの経過時間
* @param[in]  portalStates ポータルセットの状態
* 
* @return なし
*/
void Portal::Update(const AABB& playerCollision, float elapsedTime, const std::vector<bool>& portalStates)
{
	// テレポート中の処理
	if (m_isTeleporting)
	{
		// テレポートのクールダウンタイムを減少させる
		m_teleportTimer -= elapsedTime;
		if (m_teleportTimer <= TIMER_END_THRESHOLD)
		{
			// テレポート完了後の処理
			m_isTeleporting = false;
			m_isTeleported = false;
		}
		return; // テレポート中は無効にしておく
	}

	size_t pt_count = std::min(m_portals.size(), m_collisions.size());

	for (size_t i = 0; i < pt_count; ++i)
	{
		// ポータルセットの状態を確認
		const auto& pt = m_portals[i];
		int setIndex = m_portalSetIndices[i];

		// ポータルセットが有効になっている場合
		if (setIndex == INVALID_SET_INDEX || (setIndex >= 0 && setIndex < portalStates.size() 
			&& portalStates[setIndex]))
		{
			DirectX::SimpleMath::Vector3 half = pt.scale * HALF_SCALE;
			m_collisions[i] = AABB(pt.position - half, pt.position + half);

			const auto& portalCollision = m_collisions[i];

			// 衝突チェック
			if (playerCollision.CheckAABBCollision(playerCollision, portalCollision))
			{
				switch (pt.portalType)
				{
				// ポータルの種類に応じた処理
				case PortalTargetType::MOVEON:
				case PortalTargetType::RETURN:
					if (!m_isTeleporting)
					{
						m_isTeleporting = true;
						m_isTeleported = true;
						m_teleportTimer = TELEPORT_COOLDOWN_TIME;

						// ポータル通過時の処理
						if (pt.onTeleport)
						{
							pt.onTeleport();
						}
					}
					break;
				}
			}
		}
		else
		{
			// ポータルセットが無効な場合は当たり判定をリセット
			m_collisions[i] = AABB(DirectX::SimpleMath::Vector3::Zero, DirectX::SimpleMath::Vector3::Zero);
		}
	}
}

/*
* @brief 描画処理
*
* @param[in]  context      デバイスコンテキスト
* @param[in]  view         ビュー行列
* @param[in]  proj         射影行列
* @param[in]  states       共通ステートオブジェクト
* @param[in]  portalStates ポータルセットの状態
*
* @return なし
*/
void Portal::Render(ID3D11DeviceContext* context, const DirectX::SimpleMath::Matrix& view, 
	const DirectX::SimpleMath::Matrix& proj, DirectX::CommonStates* states, const std::vector<bool>& portalStates)
{
	for (size_t i = 0; i < m_portals.size(); ++i)
	{
		// ポータルセットの状態を確認
		const auto& pt = m_portals[i];
		int setIndex = m_portalSetIndices[i];

		// ポータルセットが有効になっている場合
		if (setIndex == -1 || (setIndex >= 0 && setIndex < portalStates.size() && portalStates[setIndex]))
		{
			DirectX::SimpleMath::Matrix scale = DirectX::SimpleMath::Matrix::CreateScale(pt.scale);
			DirectX::SimpleMath::Matrix trans = DirectX::SimpleMath::Matrix::CreateTranslation(pt.position);
			DirectX::SimpleMath::Matrix world = scale * trans;

			m_model->Draw(context, *states, world, view, proj);
		}
	}
}

/*
* @brief 各ポータルの追加処理
*
* @param[in]  data ポータルのデータ
* @param[in]  setIndex ポータルセットのインデックス
*
* @return なし
*/
void Portal::AddPortal(const PortalData& data, int setIndex)
{
	m_portals.push_back(data);
	m_collisions.push_back(AABB());
	m_portalSetIndices.push_back(setIndex);
}

/*
* @brief ポータルセットの数を取得
*
* @param[in]  なし
*
* @return　ポータルセットの数
*/
int Portal::GetPortalSetCount() const
{
	// インデックスが空の場合は０を返す
	if (m_portalSetIndices.empty())
		return 0;

	int maxSet = 0;
	// 最大のセットインデックスを見つける
	for (int setIndex : m_portalSetIndices)
	{
		maxSet = std::max(maxSet, setIndex);
	}
	return maxSet + 1;
}

/*
* @brief ポータルをセットに割り当てる
*
* @param[in]  なし
*
* @return なし
*/
void Portal::AssignPortalToSet(size_t portalIndex, int setIndex)
{
	// インデックスが範囲内の場合に割り当てる
	if (portalIndex < m_portalSetIndices.size())
	{
		m_portalSetIndices[portalIndex] = setIndex;
	}
}

/*
* @brief コライダーの線
*
* @param[in]  なし
*
* @return なし
*/
void Portal::ColliderLine( const std::vector<bool>& portalStates)
{
	if (!m_primitiveBatch) return;

	auto lineColor = DirectX::Colors::Yellow;
	m_primitiveBatch->Begin();

	for (size_t i = 0; i < m_collisions.size() && i < m_portalSetIndices.size(); ++i)
	{
		int setIndex = m_portalSetIndices[i];

		if (setIndex >= 0 && setIndex < portalStates.size() && portalStates[setIndex])
		{
			const auto& p_col = m_collisions[i];
			DirectX::SimpleMath::Vector3 portalCorners[8] =
			{
				{ p_col.min.x, p_col.min.y, p_col.min.z },
				{ p_col.max.x, p_col.min.y, p_col.min.z },
				{ p_col.max.x, p_col.max.y, p_col.min.z },
				{ p_col.min.x, p_col.max.y, p_col.min.z },
				{ p_col.min.x, p_col.min.y, p_col.max.z },
				{ p_col.max.x, p_col.min.y, p_col.max.z },
				{ p_col.max.x, p_col.max.y, p_col.max.z },
				{ p_col.min.x, p_col.max.y, p_col.max.z },
			};

			/*/////////////////////////////////////線の描画///////////////////////////////////////////*/

			// 前面の線を描画
			m_primitiveBatch->DrawLine({ portalCorners[0], lineColor }, { portalCorners[1], lineColor });
			m_primitiveBatch->DrawLine({ portalCorners[1], lineColor }, { portalCorners[2], lineColor });
			m_primitiveBatch->DrawLine({ portalCorners[2], lineColor }, { portalCorners[3], lineColor });
			m_primitiveBatch->DrawLine({ portalCorners[3], lineColor }, { portalCorners[0], lineColor });

			// 背面の線を描画
			m_primitiveBatch->DrawLine({ portalCorners[4], lineColor }, { portalCorners[5], lineColor });
			m_primitiveBatch->DrawLine({ portalCorners[5], lineColor }, { portalCorners[6], lineColor });
			m_primitiveBatch->DrawLine({ portalCorners[6], lineColor }, { portalCorners[7], lineColor });
			m_primitiveBatch->DrawLine({ portalCorners[7], lineColor }, { portalCorners[4], lineColor });

			// 前面と背面をつなぐ線を描画
			m_primitiveBatch->DrawLine({ portalCorners[0], lineColor }, { portalCorners[4], lineColor });
			m_primitiveBatch->DrawLine({ portalCorners[1], lineColor }, { portalCorners[5], lineColor });
			m_primitiveBatch->DrawLine({ portalCorners[2], lineColor }, { portalCorners[6], lineColor });
			m_primitiveBatch->DrawLine({ portalCorners[3], lineColor }, { portalCorners[7], lineColor });

			/*////////////////////////////////////////////////////////////////////////////////////////*/
		}
	}

	m_primitiveBatch->End();
}