//--------------------------------------------------------------------------------------
// File: Goal.cpp
//
// ゴールクラス
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "Goal.h"

// 定数の定義
const float Goal::BASE_SIZE_MULTIPLIER = 1.0f;	///< 判定のベースサイズに対する倍率
const float Goal::HALF_SCALE = 0.5f;			///< 半分のサイズにする
const float Goal::HEIGHT_MULTIPLIER = 1.5f;		///< ゴール判定の高さを縦長に拡張するための倍率

const float Goal::FIELD_OF_VIEW_DEGREES = 45.0f;	///< 視野角
const float Goal::NEAR_PLANE_DISTANCE = 0.1f;		///< カメラの最前面のクリップ距離
const float Goal::FAR_PLANE_DISTANCE = 100.0f;		///< カメラの最遠面のクリップ距離


/*
* @brief 初期化処理
*
* @param[in]  なし
* 
* @return なし
*/
void Goal::Initialize()
{
	CreateDeviceDependentResources();

	// モデルの大きさを初期化
	m_goalScale = { 1.0f,1.0f,1.0f };
}

/*
* @brief 更新処理
*
* @param[in]  playerCollision プレイヤーの当たり判定
* @param[in]  player　プレイヤーのポインタ
* 
* @return なし
*/
void Goal::Update(const AABB& playerCollision, Player* player)
{
	// 当たり判定のサイズ
	DirectX::SimpleMath::Vector3 goalAABBHalfSize = (m_goalScale * BASE_SIZE_MULTIPLIER);

	// 当たり判定の調整
	goalAABBHalfSize.x *= HALF_SCALE;
	goalAABBHalfSize.y *= HEIGHT_MULTIPLIER;
	goalAABBHalfSize.z *= HALF_SCALE;

	// コライダーの作成
	m_goalCollision = AABB(m_goalPosition - goalAABBHalfSize, m_goalPosition + goalAABBHalfSize);

	// 平行移動の行列を更新
	m_trans = DirectX::SimpleMath::Matrix::CreateTranslation(m_goalPosition);

	// 当たり判定の処理
	if (m_goalCollision.CheckAABBCollision(m_goalCollision, playerCollision))
	{
		player->PlayerGoal();
	}
}

/*
* @brief 描画処理
*
* @param[in]  view ビュー行列
* @param[in]  proj 射影行列
* 
* @return なし
*/
void Goal::Render(const DirectX::SimpleMath::Matrix& view)
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	auto states = m_states;

	// スケールを設定
	DirectX::SimpleMath::Matrix scale = DirectX::SimpleMath::Matrix::CreateScale(m_goalScale);

	// 回転行列
	DirectX::SimpleMath::Matrix rot = DirectX::SimpleMath::Matrix::CreateFromYawPitchRoll(
		m_goalRotation.x,
		m_goalRotation.y,
		m_goalRotation.z
	);

	// ワールド行列
	DirectX::SimpleMath::Matrix world = scale * rot * m_trans;

	m_model->Draw(context, *states, world, view, m_proj);
}

/*
* @brief デバイスに依存するリソースを作成する関数
*
* @param[in]  なし
* 
* @return なし
*/
void Goal::CreateDeviceDependentResources()
{
	if (!m_deviceResources) return;

	auto device = m_deviceResources->GetD3DDevice();
	auto context = m_deviceResources->GetD3DDeviceContext();

	// エフェクトファクトリーの作成
	std::unique_ptr<DirectX::EffectFactory> fx = std::make_unique<DirectX::EffectFactory>(device);
	fx->SetDirectory(L"Resources/Models");

	// プリミティブバッチの初期化
	m_primitiveBatch = std::make_unique<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>(context);

	// モデルの読み込み
	m_model = DirectX::Model::CreateFromSDKMESH(device, L"Resources/Models/goal.sdkmesh", *fx);

	// 射影行列の作成
	RECT rect = m_deviceResources->GetOutputSize();
	m_proj = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(
		DirectX::XMConvertToRadians(FIELD_OF_VIEW_DEGREES),
		static_cast<float>(rect.right) / static_cast<float>(rect.bottom),
		NEAR_PLANE_DISTANCE, FAR_PLANE_DISTANCE
	);
}

/*
* @brief 座標の設定
*
* @param[in]  x X座標
* @param[in]  y Y座標
* @param[in]  z Z座標
* 
* @return なし
*/
void Goal::SetPosition(float x, float y, float z)
{
	m_goalPosition = { x,y,z };
}

/*
* @brief 角度の設定
*
* @param[in]  x X座標
* @param[in]  y Y座標
* @param[in]  z Z座標
*
* @return なし
*/
void Goal::SetRotation(float x, float y, float z)
{
	m_goalRotation = { x,y,z };
}

/*
* @brief コライダーの線
*
* @param[in]  なし
* 
* @return なし
*/
void Goal::ColliderLine()
{
	if (!m_primitiveBatch) return;

	// コライダー線の色
	auto lineColor = DirectX::Colors::BlueViolet;

	m_primitiveBatch->Begin();
	// AABBの頂点を計算
	DirectX::SimpleMath::Vector3 corners[8] =
	{
		{ m_goalCollision.min.x, m_goalCollision.min.y, m_goalCollision.min.z },
		{ m_goalCollision.max.x, m_goalCollision.min.y, m_goalCollision.min.z },
		{ m_goalCollision.max.x, m_goalCollision.max.y, m_goalCollision.min.z },
		{ m_goalCollision.min.x, m_goalCollision.max.y, m_goalCollision.min.z },
		{ m_goalCollision.min.x, m_goalCollision.min.y, m_goalCollision.max.z },
		{ m_goalCollision.max.x, m_goalCollision.min.y, m_goalCollision.max.z },
		{ m_goalCollision.max.x, m_goalCollision.max.y, m_goalCollision.max.z },
		{ m_goalCollision.min.x, m_goalCollision.max.y, m_goalCollision.max.z }
	};

	/*/////////////////////////////////////線の描画/////////////////////////////////*/

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

	m_primitiveBatch->End();
}
