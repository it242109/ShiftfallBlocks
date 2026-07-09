//--------------------------------------------------------------------------------------
// File: Gate.cpp
//
// ゲートクラス
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "Gate.h"
#include "GameObjects/StageObjects/StageObject.h"

// 定数の定義
const float Gate::INITIAL_X = 9999.0f;					///< 初期位置Ｘ（定数）

const DirectX::SimpleMath::Vector3 Gate::DEFAULT_SCALE = { 2.2f,2.0f,1.0f };///< ゲートのデフォルトの大きさ

const float Gate::MAX_GATE_OPEN = 3.0f;					///< ゲートの最大開き具合
const float Gate::MIN_GATE_OPEN = 0.0f;					///< ゲートの最小開き具合
const float Gate::OPEN_CLOSE_SPEED = 0.07f;				///< ゲートの開閉速度
const float Gate::GATE_COLLISION_SIZE_SCALE = 1.0f;		///< ゲートの当たり判定の大きさのスケール
const float Gate::GATE_COLLISION_WIDTH_SCALE = 0.5f;	///< ゲートの当たり判定の幅
const float Gate::GATE_COLLISION_DEPTH_SCALE = 0.3f;	///< ゲートの当たり判定の奥行

const float Gate::FIELD_OF_VIEW_DEGREES = 45.0f;		///< 視野角
const float Gate::NEAR_PLANE_DISTANCE = 0.1f;			///< カメラの最前面のクリップ距離
const float Gate::FAR_PLANE_DISTANCE = 100.0f;			///< カメラの最遠面のクリップ距離

/*
* @brief コンストラクタ
*
* @param[in]  なし
* 
* @return なし
*/
Gate::Gate()
	:
	m_gateRotAngle(0.0f),
	m_initialX(0.0f)
{
}

/*
* @brief デストラクタ
*
* @param[in]  なし
* 
* @return なし
*/
Gate::~Gate()
{
}

/*
* @brief 初期化処理
*
* @param[in]  なし
* 
* @return なし
*/
void Gate::Initialize()
{
	// ゲートの大きさ
	m_gateScale = DEFAULT_SCALE;

	// 初期値の設定
	m_initialX = INITIAL_X;

	CreateDeviceDependentResources();
}

/*
* @brief 更新処理
*
* @param[in]  playerCollision プレイヤーの当たり判定
* @param[in]  enemyCollision 敵の当たり判定
* 
* @return なし
*/
void Gate::Update(Player* player, std::vector<std::unique_ptr<Enemy>>& enemies)
{
	m_gateTrans = DirectX::SimpleMath::Matrix::CreateTranslation(m_gatePosition);

	// ここに来た場合、現在の位置を初期位置にする
	if (m_initialX == INITIAL_X)
	{
		m_initialX = m_gatePosition.x;
	}

	// 左に開く限界のX座標
	float leftLimitX = m_initialX - MAX_GATE_OPEN;

	// ゲートの開閉
	if (m_isOpen)
	{
		if (m_gatePosition.x > leftLimitX)
			m_gatePosition.x -= OPEN_CLOSE_SPEED;

	}
	else if (!m_isOpen)
	{
		if (m_gatePosition.x < m_initialX)
			m_gatePosition.x += OPEN_CLOSE_SPEED;
	}
	
	// 当たり判定のサイズ
	DirectX::SimpleMath::Vector3 gateAABBHalfSize = (m_gateScale * GATE_COLLISION_SIZE_SCALE);

	// 当たり判定の調整
	gateAABBHalfSize.x *= GATE_COLLISION_WIDTH_SCALE;
	gateAABBHalfSize.z *= GATE_COLLISION_DEPTH_SCALE;

	// 当たり判定の作成
	if (!m_isOpen || m_gatePosition.x >= leftLimitX)
		m_gateCollision = AABB(m_gatePosition - gateAABBHalfSize, m_gatePosition + gateAABBHalfSize);
	else
		m_gateCollision = AABB(DirectX::SimpleMath::Vector3::Zero, DirectX::SimpleMath::Vector3::Zero);

	// プレイヤーとの当たり判定
	if (player)
	{
		player->UpdateCollision(m_gateCollision, m_gatePosition);
	}
	// 敵との当たり判定
	for (auto& enemy : enemies)
	{
		if (enemy && !enemy->IsDead())
		{
			enemy->UpdateCollision(m_gateCollision, m_gatePosition);
		}
	}
}

/*
* @brief 描画処理
*
* @param[in]  view ビュー行列
* 
* @return なし
*/
void Gate::Render(const DirectX::SimpleMath::Matrix& view)
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	auto states = m_states;

	// スケールの設定
	DirectX::SimpleMath::Matrix scale = DirectX::SimpleMath::Matrix::CreateScale(m_gateScale);

	// 回転の設定
	DirectX::SimpleMath::Matrix rot = DirectX::SimpleMath::Matrix::CreateRotationY(m_gateRotAngle);

	// ワールド行列
	DirectX::SimpleMath::Matrix world = scale * rot * m_gateTrans;

	// モデルの描画
	m_model->Draw(context, *states, world, view, m_proj);
}

/*
* @brief デバイスに依存するリソースを作成する関数
*
* @param[in]  なし
* 
* @return なし
*/
void Gate::CreateDeviceDependentResources()
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
	m_model = DirectX::Model::CreateFromSDKMESH(device, L"Resources/Models/gate.sdkmesh", *fx);

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
void Gate::SetPosition(float x, float y, float z)
{
	m_gatePosition = { x,y,z };
	m_gateTrans = DirectX::SimpleMath::Matrix::CreateTranslation(m_gatePosition);

	m_initialX = x;
}

/*
* @brief コライダーの線
*
* @param[in]  なし
* 
* @return なし
*/
void Gate::ColliderLine()
{
	if (!m_primitiveBatch) return;

	// コライダー線の色
	auto lineColor = DirectX::Colors::IndianRed;

	m_primitiveBatch->Begin();
	// AABBの頂点を計算
	DirectX::SimpleMath::Vector3 corners[8] =
	{
		{ m_gateCollision.min.x, m_gateCollision.min.y, m_gateCollision.min.z },
		{ m_gateCollision.max.x, m_gateCollision.min.y, m_gateCollision.min.z },
		{ m_gateCollision.max.x, m_gateCollision.max.y, m_gateCollision.min.z },
		{ m_gateCollision.min.x, m_gateCollision.max.y, m_gateCollision.min.z },
		{ m_gateCollision.min.x, m_gateCollision.min.y, m_gateCollision.max.z },
		{ m_gateCollision.max.x, m_gateCollision.min.y, m_gateCollision.max.z },
		{ m_gateCollision.max.x, m_gateCollision.max.y, m_gateCollision.max.z },
		{ m_gateCollision.min.x, m_gateCollision.max.y, m_gateCollision.max.z }
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

	/*///////////////////////////////////////////////////////////////////////////////*/

	m_primitiveBatch->End();
}