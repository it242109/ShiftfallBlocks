//--------------------------------------------------------------------------------------
// File: Platform.cpp
//
// 足場クラス
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "Platform.h"

// 定数の定義
const float Platform::HALF_SCALE = 0.5f;	///< 半分のサイズにする

/*
* @brief コンストラクタ
*
* @param[in]  なし
* 
* @return なし
*/
Platform::Platform()
{
}

/*
* @brief デストラクタ
*
* @param[in]  なし
* 
* @return なし
*/
Platform::~Platform()
{
}

/*
* @brief 足場情報を登録
*
* @param[in]  positions 足場の位置のリスト
* @param[in]  scales 足場のスケールのリスト
* 
* @return なし
*/
void Platform::LoadPlatformData(const std::vector<DirectX::SimpleMath::Vector3>& positions, const std::vector<DirectX::SimpleMath::Vector3>& scales)
{
	m_positions = positions;
	m_scales = scales;
	m_collisions.resize(positions.size());
}

/*
* @brief 更新処理
*
* @param[in]  switchStates スイッチの状態のリスト
* @param[in]  player       プレイヤーへのポインタ
* @param[in]  enemies      敵のリスト
*
* @return なし
*/
void Platform::Update(const std::vector<bool>& switchStates, Player* player, std::vector<std::unique_ptr<Enemy>>& enemies)
{
	// 外部から受け取ったスイッチ状態を保存
	m_switchStates = switchStates;

	// 配置されているオブジェクトごとに処理
	for (size_t i = 0; i < m_positions.size(); ++i)
	{
		// オブジェクトの位置とスケールを取得
		const DirectX::SimpleMath::Vector3& pos = m_positions[i];
		const DirectX::SimpleMath::Vector3& scale = m_scales[i];

		// AABB生成用の半サイズを計算
		DirectX::SimpleMath::Vector3 half = scale * HALF_SCALE;

		// スイッチがONの場合のみ有効な当たり判定を設定
		if (i < switchStates.size() && switchStates[i])
			m_collisions[i] = AABB(pos - half, pos + half);
		else
			m_collisions[i] = AABB(DirectX::SimpleMath::Vector3::Zero, DirectX::SimpleMath::Vector3::Zero);

		// プレイヤーとの当たり判定
		if (player)
		{
			player->UpdateCollision(m_collisions[i], pos);
		}

		// 敵との当たり判定
		for (auto& enemy : enemies)
		{
			if (enemy && !enemy->IsDead())
			{
				enemy->UpdateCollision(m_collisions[i], pos);
			}
		}
	}
}

/*
* @brief 描画処理
*
* @param[in]  view  ビュー行列
* @param[in]  proj  射影行列
*
* @return なし
*/
void Platform::Render(const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj)
{
	if (!m_model || !m_deviceResources) return;

	auto context = m_deviceResources->GetD3DDeviceContext();
	auto states = std::make_unique<DirectX::CommonStates>(m_deviceResources->GetD3DDevice());

	// モデル全体に適用するY軸回転行列
	DirectX::SimpleMath::Matrix rot = DirectX::SimpleMath::Matrix::CreateRotationY(0.0f);

	// 配置済みモデルの数だけ描画
	for (size_t i = 0; i < m_positions.size(); i++)
	{ 
		if (i < m_switchStates.size() && !m_switchStates[i])
			continue;

		// スケール行列を生成
		DirectX::SimpleMath::Matrix scale = DirectX::SimpleMath::Matrix::CreateScale(m_scales[i]);
		// 平行移動行列を生成
		DirectX::SimpleMath::Matrix trans= DirectX::SimpleMath::Matrix::CreateTranslation(m_positions[i]);
		// ワールド行列を作成
		DirectX::SimpleMath::Matrix world = scale * rot *trans;

		// モデルの描画
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
void Platform::CreateDeviceDependentResources()
{
	auto device = m_deviceResources->GetD3DDevice();
	auto context = m_deviceResources->GetD3DDeviceContext();

	// エフェクトファクトリーの作成
	std::unique_ptr<DirectX::EffectFactory> fx = std::make_unique<DirectX::EffectFactory>(device);
	fx->SetDirectory(L"Resources/Models");

	// プリミティブバッチの初期化
	m_primitiveBatch = std::make_unique<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>(context);

	// モデルの読み込み
	m_model = DirectX::Model::CreateFromSDKMESH(device, L"Resources/Models/platform.sdkmesh", *fx);
}

/*
* @brief コライダーの線
*
* @param[in]  なし
* 
* @return なし
*/
void Platform::ColliderLine()
{
	if (!m_primitiveBatch) return;

	auto lineColor = DirectX::Colors::Black;
	m_primitiveBatch->Begin();

	// 複数の足場
	for (const auto& pf_col : m_collisions)
	{
		DirectX::SimpleMath::Vector3 platformCorners[8] =
		{
			{ pf_col.min.x, pf_col.min.y, pf_col.min.z },
			{ pf_col.max.x, pf_col.min.y, pf_col.min.z },
			{ pf_col.max.x, pf_col.max.y, pf_col.min.z },
			{ pf_col.min.x, pf_col.max.y, pf_col.min.z },
			{ pf_col.min.x, pf_col.min.y, pf_col.max.z },
			{ pf_col.max.x, pf_col.min.y, pf_col.max.z },
			{ pf_col.max.x, pf_col.max.y, pf_col.max.z },
			{ pf_col.min.x, pf_col.max.y, pf_col.max.z },
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

		/*////////////////////////////////////////////////////////////////////////////////////////*/
	}

	m_primitiveBatch->End();
}