//--------------------------------------------------------------------------------------
// File: Enemy.cpp
//
// 敵の処理から描画までまとめたクラス
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "Enemy.h"
#include "SKLib/ReadData.h"

/*
* @brief コンストラクタ
*
* @param[in]  なし
* 
* @return なし
*/
Enemy::Enemy()
	: 
	m_floorHit(false),
	m_isDead(false),
	m_enemyVelocity(0.0f),
	m_verticalVelocity(0.0f)
{
}

/*
* @brief デストラクタ
*
* @param[in]  なし
* 
* @return なし
*/
Enemy::~Enemy()
{
}

/*
* @brief 初期化処理
*
* @param[in]  startPos 開始座標
* 
* @return なし
*/
void Enemy::Initialize(const DirectX::SimpleMath::Vector3& startPos)
{
	CreateDeviceDependentResources();

	// モデルの大きさを初期化
	m_enemyScale = { 1.0f,1.4f,1.0f };
	// ダメージ範囲の大きさを初期化
	m_damageScale = { 0.7f,0.7f,0.7f }; 

	// 位置設定
	m_enemyPosition = startPos;

	// 当たり判定の作成
	m_enemyCollision = m_enemyCollision.CreateAABB(m_enemyPosition, m_enemyScale);
	m_damageCollision = m_damageCollision.CreateAABB(m_enemyPosition, m_damageScale);
	m_enemyTrans = DirectX::SimpleMath::Matrix::CreateTranslation(m_enemyPosition);

	m_isDead = false;
}

/*
* @brief 更新処理
*
* @param[in]  elapsedTime 前フレームからの経過時間
* @param[in]  enemyPos    敵の位置
* 
* @return なし
*/
void Enemy::Update(float elapsedTime,const DirectX::SimpleMath::Vector3& enemyPos)
{
	// 毎フレーム床判定リセット
	m_floorHit = false;

	// 状態遷移チェック
	UpdateState(enemyPos);
	
	// 状態ごとの処理
	switch (m_state)
	{
	case EnemyState::Idle:
		// 待機処理
		UpdateIdle();
		break;

	case EnemyState::Chase: 
		// 追尾処理
		UpdateChase(enemyPos);
		break;

	case EnemyState::Dead:
		// 死亡処理
		EnemyKill(); 
		break;
	}

	// 重力適用
	if (!IsOnFloor())
	{
		ApplyGravity(elapsedTime);
	}
	else
	{
		m_verticalVelocity = 0.0f;
	}
	// AABBを更新
	m_enemyCollision = m_enemyCollision.CreateAABB(m_enemyPosition, m_enemyScale);
	m_damageCollision = m_damageCollision.CreateAABB(m_enemyPosition, m_damageScale);

	// ワールド行列を更新
	m_enemyTrans = DirectX::SimpleMath::Matrix::CreateTranslation(m_enemyPosition);
}

/*
* @brief 状態遷移処理
*
* @param[in]  enemyPos 敵の位置
* 
*　@return なし
*/
void Enemy::UpdateState(const DirectX::SimpleMath::Vector3& enemyPos)
{
	float dx = abs(enemyPos.x - m_enemyPosition.x);
	float dz = abs(enemyPos.z - m_enemyPosition.z);

	bool nearEnemy = (dx <= 15.0f && dz <= 15.0f);

	// ステートごとに状態遷移
	switch (m_state)
	{
	// 待機状態
	case EnemyState::Idle: 
		if (nearEnemy)
			m_state = EnemyState::Chase;
		break;

	// 追尾状態
	case EnemyState::Chase: 
		if (!nearEnemy)
			m_state = EnemyState::Idle;
		break;

	// 死亡状態
	case EnemyState::Dead: 
		break;
	}
}

/*
* @brief 追尾処理
*
* @param[in]  enemyPos 敵の位置
* 
* @return なし
*/
void Enemy::UpdateChase(const DirectX::SimpleMath::Vector3& enemyPos)
{
	// プレイヤーへの方向ベクトルを計算
	DirectX::SimpleMath::Vector3 toPlayer = enemyPos - m_enemyPosition;
	toPlayer.y = 0.0f;

	float distance = toPlayer.Length();
	if (distance > 0.1f)
	{
		toPlayer.Normalize();
		m_enemyForward = toPlayer;
		m_enemyVelocity = 0.03f;

		// プレイヤーに向かって移動
		m_enemyPosition += toPlayer * m_enemyVelocity;
	}
}

/*
* @brief 待機処理
*
* @param[in]  なし
* 
* @return なし
*/
void Enemy::UpdateIdle()
{
	// 待機中は特に何もしない
	m_enemyVelocity = 0.0f;
}

/*
* @brief 重力の適用
*
* @param[in]  elapsedTime 前フレームからの経過時間
* 
* @return なし
*/
void Enemy::ApplyGravity(float elapsedTime)
{
	// 重力の適用
	if (!m_floorHit)
	{
		m_verticalVelocity += GRAVITY * elapsedTime;
		m_enemyPosition.y += m_verticalVelocity * elapsedTime;
	}
	else
	{
		m_verticalVelocity = 0.0f;
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
void Enemy::Render(const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& proj)
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	auto states = m_states;

	m_view = view;
	m_proj = proj;

	// スケール設定
	DirectX::SimpleMath::Matrix enemyScale = DirectX::SimpleMath::Matrix::CreateScale(m_enemyScale); // スケール適用

	// 回転計算
	float enemyRotationY = 0.0f;
	if (m_enemyForward.LengthSquared() > 0.0f)
	{
		enemyRotationY = atan2f(m_enemyForward.x, m_enemyForward.z);
	}
	DirectX::SimpleMath::Matrix enemyRot = DirectX::SimpleMath::Matrix::CreateRotationY(enemyRotationY);

	// ワールド行列
	DirectX::SimpleMath::Matrix enemyWorld = enemyScale * enemyRot * m_enemyTrans;

	// モデルの描画
	m_enemyModel->Draw(context, *states, enemyWorld, view, proj);
}

/*
* @brief 終了処理
*
* @param[in]  なし
* 
* @return なし
*/
void Enemy::Finalize()
{
}

/*
* @brief デバイスに依存するリソースを作成する関数
*
* @param[in]  なし
* 
* @return なし
*/
void Enemy::CreateDeviceDependentResources()
{
	auto device = m_deviceResources->GetD3DDevice();
	auto context = m_deviceResources->GetD3DDeviceContext();

	// エフェクトファクトリーの作成
	std::unique_ptr<DirectX::EffectFactory> fx = std::make_unique<DirectX::EffectFactory>(device);
	fx->SetDirectory(L"Resources/Models");

	// プリミティブバッチの初期化
	m_primitiveBatch = std::make_unique<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>(context);

	// モデルの読み込み
	m_enemyModel = DirectX::Model::CreateFromSDKMESH(device, L"Resources/Models/enemy.sdkmesh", *fx);

	// 影用の円形モデルの読み込み
	m_shadowModel = DirectX::Model::CreateFromSDKMESH(device, L"Resources/Models/circle.sdkmesh", *fx);

	// 影描画用の深度ステンシルステートを作成
	D3D11_DEPTH_STENCIL_DESC shadowDesc = {};
	shadowDesc.DepthEnable = TRUE;
	shadowDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	shadowDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	shadowDesc.StencilEnable = FALSE;

	device->CreateDepthStencilState(&shadowDesc, m_depthStencilState_Shadow.ReleaseAndGetAddressOf());

	// 影用ピクセルシェーダーの読み込み
	std::vector<uint8_t> ps = DX::ReadData(L"Resources/Shaders/PixelShader.cso");
	device->CreatePixelShader(ps.data(), ps.size(), nullptr, m_PS.ReleaseAndGetAddressOf());

	if (!m_deviceResources) return;

	// 射影行列の作成
	RECT rect = m_deviceResources->GetOutputSize();
	m_proj = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(
		DirectX::XMConvertToRadians(45.0f),
		static_cast<float>(rect.right) / static_cast<float>(rect.bottom),
		0.1f, 100.0f
	);
}

/*
* @brief 床の着地状態をリセット
*
* @param[in]  なし
* 
* @return なし
*/
void Enemy::ResetFloorHit()
{
	m_floorHit = false;
}

/*
* @brief 影の描画
*
* @param[in]  なし
* 
* @return なし
*/
void Enemy::RenderShadow()
{
	auto context = m_deviceResources->GetD3DDeviceContext();

	// ライトの方向（斜め上から - 影が見えやすい角度）
	DirectX::SimpleMath::Vector3 lightDir = DirectX::SimpleMath::Vector3(0.2f, 1.0f, 0.2f);
	lightDir.Normalize();

	// 立っている床を検出
	float enemyX = m_enemyPosition.x;
	float enemyY = m_enemyPosition.y;
	float enemyZ = m_enemyPosition.z;
	float closestSurfaceY = -FLT_MAX;
	bool foundSurface = false;

	// 床と足場の両方をチェック
	auto checkSurface = [&](const std::vector<DirectX::SimpleMath::Vector3>& posList,
		const std::vector<DirectX::SimpleMath::Vector3>& scaleList)
		{
			for (size_t i = 0; i < posList.size(); i++)
			{
				float halfWidth = scaleList[i].x * 0.5f;
				float halfDepth = scaleList[i].z * 0.5f;

				if (enemyX >= posList[i].x - halfWidth && enemyX <= posList[i].x + halfWidth &&
					enemyZ >= posList[i].z - halfDepth && enemyZ <= posList[i].z + halfDepth)
				{
					float topY = posList[i].y + (scaleList[i].y * 0.5f);
					if (topY <= enemyY + 0.5f && topY > closestSurfaceY)
					{
						closestSurfaceY = topY;
						foundSurface = true;
					}
				}
			}
		};
	// 静的な床の判定
	checkSurface(m_floorPositions, m_floorScales);
	// 動的な足場の判定を追加！
	checkSurface(m_platformPositions, m_platformScales);

	// 見つからなかったらデフォルト値に設定
	if (!foundSurface)
	{
		closestSurfaceY = 0.0f;
	}

	// Zファイティング防止のため少し上げる
	closestSurfaceY += 0.03f;

	// シャドウマトリクスの作成
	DirectX::SimpleMath::Plane groundPlane = DirectX::SimpleMath::Plane(0.0f, 1.0f, 0.0f, -closestSurfaceY);

	DirectX::SimpleMath::Matrix shadowMatrix = DirectX::SimpleMath::Matrix::CreateShadow(lightDir, groundPlane);
	// 影の描画設定
	if (foundSurface)
	{
		// 影のスケーリングを計算（ジャンプ時に影を小さくする）
		float heightDiff = m_enemyPosition.y - closestSurfaceY;
		float scaleFactor = 1.0f / (1.0f + heightDiff * 0.05f);
		scaleFactor = std::max(0.2f, scaleFactor); // 最小サイズを設定

		// 影のスケール
		DirectX::SimpleMath::Matrix shadowScale = DirectX::SimpleMath::Matrix::CreateScale(scaleFactor);
		// 回転計算
		float enemyRotationY = 0.0f;
		if (m_enemyForward.LengthSquared() > 0.0f)
		{
			enemyRotationY = atan2f(m_enemyForward.x, m_enemyForward.z);
		}
		DirectX::SimpleMath::Matrix enemyRot = DirectX::SimpleMath::Matrix::CreateRotationY(enemyRotationY);

		// 影の位置計算
		DirectX::SimpleMath::Matrix shadowTrans = DirectX::SimpleMath::Matrix::CreateTranslation(m_enemyPosition);

		// シャドウマトリクスを適用
		DirectX::SimpleMath::Matrix worldBase = shadowScale * enemyRot * shadowTrans;
		DirectX::SimpleMath::Matrix finalShadowMatrix = worldBase * shadowMatrix;

		// 影の描画
		m_shadowModel->Draw(context, *m_states, finalShadowMatrix, m_view, m_proj,
			false, [&]()
			{
				// アルファブレンドを有効にして透明な影を描画
				context->OMSetBlendState(m_states->AlphaBlend(), nullptr, 0xffffffff);

				// 深度テストは有効、深度書き込みは無効
				context->OMSetDepthStencilState(m_depthStencilState_Shadow.Get(), 1);

				// 背面カリングを無効にする
				context->RSSetState(m_states->CullNone());

				// ピクセルシェーダーの設定
				context->PSSetShader(m_PS.Get(), nullptr, 0);
			}
		);
	}
}

/*
* @brief 当たり判定の更新
*
* @param[in]  collision 当たり判定
* @param[in]  position　位置
* 
* @return なし
*/
void Enemy::UpdateCollision(const AABB& collision, const DirectX::SimpleMath::Vector3& position)
{
	if (m_enemyCollision.CheckAABBCollision(m_enemyCollision, collision))
	{
		// 重なり量を計算
		DirectX::SimpleMath::Vector3 overlap;
		overlap.x = std::min(m_enemyCollision.max.x, collision.max.x) -
			std::max(m_enemyCollision.min.x, collision.min.x);
		overlap.y = std::min(m_enemyCollision.max.y, collision.max.y) -
			std::max(m_enemyCollision.min.y, collision.min.y);
		overlap.z = std::min(m_enemyCollision.max.z, collision.max.z) -
			std::max(m_enemyCollision.min.z, collision.min.z);

		// 最も小さい重なり方向に押し戻す
		if (overlap.y <= overlap.x && overlap.y <= overlap.z)
		{
			float enemyBottom = m_enemyPosition.y - (m_enemyScale.y * 0.5f);
			float objectTop = position.y + (collision.max.y - collision.min.y) * 0.5f;

			// 着地の許容範囲
			const float landingThreshold = 0.2f;

			// 上から着地した場合
			if (enemyBottom >= objectTop - landingThreshold && m_verticalVelocity <= 0.0f)
			{
				m_enemyPosition.y = objectTop + (m_enemyScale.y * 0.5f);

				// フラグ更新
				m_floorHit = true;
				m_verticalVelocity = 0.0f;
			}
			else
			{
				// 下からぶつかった場合
				m_enemyPosition.y -= overlap.y;
				m_verticalVelocity = 0.0f;
			}
		}
		else if (overlap.x < overlap.z)
		{
			// X方向の衝突
			if (m_enemyPosition.x < position.x)
				m_enemyPosition.x -= overlap.x;
			else
				m_enemyPosition.x += overlap.x;
		}
		else
		{
			// Z方向の衝突
			if (m_enemyPosition.z < position.z)
				m_enemyPosition.z -= overlap.z;
			else
				m_enemyPosition.z += overlap.z;
		}

	}

	// AABB再生成
	m_enemyCollision = m_enemyCollision.CreateAABB(m_enemyPosition, m_enemyScale);
	m_damageCollision = m_damageCollision.CreateAABB(m_enemyPosition, m_damageScale);

}

/*
* @brief 床情報を設定するメソッド
*
* @param[in]  positions　床の位置
* @param[in]  scales　　床の大きさ
* 
* @return なし
*/
void Enemy::SetFloorData(const std::vector<DirectX::SimpleMath::Vector3>& f_positions, const std::vector<DirectX::SimpleMath::Vector3>& f_scales)
{
	m_floorPositions = f_positions;
	m_floorScales = f_scales;
}

void Enemy::SetPlatformData(const std::vector<DirectX::SimpleMath::Vector3>& pf_positions, const std::vector<DirectX::SimpleMath::Vector3>& pf_scales)
{
	m_platformPositions = pf_positions;
	m_platformScales = pf_scales;
}

/*
* @brief 位置を設定
*
* @param[in]  x　X座標
* @param[in]  y　Y座標
* @param[in]  z  Z座標
* 
* @return なし
*/
void Enemy::SetPosition(float x, float y, float z)
{
	m_enemyPosition.x = x;
	m_enemyPosition.y = y;
	m_enemyPosition.z = z;
}

/*
* @brief 敵を消す処理
*
* @param[in]  なし
* 
* @return なし
*/
void Enemy::EnemyKill()
{
	m_isDead = true;
	m_verticalVelocity = 0.0f;

	// 当たり判定を無効化
	m_enemyCollision = { {0,0,0}, {0,0,0} };
	m_damageCollision = { {0,0,0}, {0,0,0} };

}

/*
* @brief 敵のリセット
*
* @param[in]  なし
*
* @return なし
*/
void Enemy::Reset(const DirectX::SimpleMath::Vector3& positions)
{
	m_enemyPosition = positions;
	m_isDead = false;
}

/*
* @brief コライダーの線
*
* @param[in]  なし
* 
* @return なし
*/
void Enemy::ColliderLine()
{
	if (!m_primitiveBatch) return;
	
	// コライダー線の色
	auto lineColorA = DirectX::Colors::Red;
	auto lineColorB = DirectX::Colors::Purple;

	m_primitiveBatch->Begin();
	// AABBの頂点を計算
	DirectX::SimpleMath::Vector3 enemyCorners[8] =
	{
		{ m_enemyCollision.min.x, m_enemyCollision.min.y, m_enemyCollision.min.z },
		{ m_enemyCollision.max.x, m_enemyCollision.min.y, m_enemyCollision.min.z },
		{ m_enemyCollision.max.x, m_enemyCollision.max.y, m_enemyCollision.min.z },
		{ m_enemyCollision.min.x, m_enemyCollision.max.y, m_enemyCollision.min.z },
		{ m_enemyCollision.min.x, m_enemyCollision.min.y, m_enemyCollision.max.z },
		{ m_enemyCollision.max.x, m_enemyCollision.min.y, m_enemyCollision.max.z },
		{ m_enemyCollision.max.x, m_enemyCollision.max.y, m_enemyCollision.max.z },
		{ m_enemyCollision.min.x, m_enemyCollision.max.y, m_enemyCollision.max.z }
	};
	DirectX::SimpleMath::Vector3 damageCorners[8] =
	{
		{ m_damageCollision.min.x, m_damageCollision.min.y, m_damageCollision.min.z },
		{ m_damageCollision.max.x, m_damageCollision.min.y, m_damageCollision.min.z },
		{ m_damageCollision.max.x, m_damageCollision.max.y, m_damageCollision.min.z },
		{ m_damageCollision.min.x, m_damageCollision.max.y, m_damageCollision.min.z },
		{ m_damageCollision.min.x, m_damageCollision.min.y, m_damageCollision.max.z },
		{ m_damageCollision.max.x, m_damageCollision.min.y, m_damageCollision.max.z },
		{ m_damageCollision.max.x, m_damageCollision.max.y, m_damageCollision.max.z },
		{ m_damageCollision.min.x, m_damageCollision.max.y, m_damageCollision.max.z }
	};

	// 前面の線を描画
	m_primitiveBatch->DrawLine({ enemyCorners[0], lineColorA }, { enemyCorners[1], lineColorA });
	m_primitiveBatch->DrawLine({ enemyCorners[1], lineColorA }, { enemyCorners[2], lineColorA });
	m_primitiveBatch->DrawLine({ enemyCorners[2], lineColorA }, { enemyCorners[3], lineColorA });
	m_primitiveBatch->DrawLine({ enemyCorners[3], lineColorA }, { enemyCorners[0], lineColorA });

	m_primitiveBatch->DrawLine({ damageCorners[0], lineColorB }, { damageCorners[1], lineColorB });
	m_primitiveBatch->DrawLine({ damageCorners[1], lineColorB }, { damageCorners[2], lineColorB });
	m_primitiveBatch->DrawLine({ damageCorners[2], lineColorB }, { damageCorners[3], lineColorB });
	m_primitiveBatch->DrawLine({ damageCorners[3], lineColorB }, { damageCorners[0], lineColorB });

	// 背面の線を描画
	m_primitiveBatch->DrawLine({ enemyCorners[4], lineColorA }, { enemyCorners[5], lineColorA });
	m_primitiveBatch->DrawLine({ enemyCorners[5], lineColorA }, { enemyCorners[6], lineColorA });
	m_primitiveBatch->DrawLine({ enemyCorners[6], lineColorA }, { enemyCorners[7], lineColorA });
	m_primitiveBatch->DrawLine({ enemyCorners[7], lineColorA }, { enemyCorners[4], lineColorA });

	m_primitiveBatch->DrawLine({ damageCorners[4], lineColorB }, { damageCorners[5], lineColorB });
	m_primitiveBatch->DrawLine({ damageCorners[5], lineColorB }, { damageCorners[6], lineColorB });
	m_primitiveBatch->DrawLine({ damageCorners[6], lineColorB }, { damageCorners[7], lineColorB });
	m_primitiveBatch->DrawLine({ damageCorners[7], lineColorB }, { damageCorners[4], lineColorB });

	// 前面と背面をつなぐ線を描画
	m_primitiveBatch->DrawLine({ enemyCorners[0], lineColorA }, { enemyCorners[4], lineColorA });
	m_primitiveBatch->DrawLine({ enemyCorners[1], lineColorA }, { enemyCorners[5], lineColorA });
	m_primitiveBatch->DrawLine({ enemyCorners[2], lineColorA }, { enemyCorners[6], lineColorA });
	m_primitiveBatch->DrawLine({ enemyCorners[3], lineColorA }, { enemyCorners[7], lineColorA });

	m_primitiveBatch->DrawLine({ damageCorners[0], lineColorB }, { damageCorners[4], lineColorB });
	m_primitiveBatch->DrawLine({ damageCorners[1], lineColorB }, { damageCorners[5], lineColorB });
	m_primitiveBatch->DrawLine({ damageCorners[2], lineColorB }, { damageCorners[6], lineColorB });
	m_primitiveBatch->DrawLine({ damageCorners[3], lineColorB }, { damageCorners[7], lineColorB });

	m_primitiveBatch->End();
}