//--------------------------------------------------------------------------------------
// File: Player.cpp
//
// プレイヤーの処理から描画までまとめたクラス
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "Player.h"
#include "SKLib/ReadData.h"

#include "SKLib/GameCamera.h"
#include "SKLib/InputManager.h"
#include "SKLib/SoundManager.h"
#include "SKLib/TutorialManager.h"

// 定数の定義
const float Player::GRAVITY = -9.8f;						///< 重力
const float Player::JUMP_POWER = 5.5f;						///< ジャンプの初速
const float Player::FALL_SPEED = -3.0f;						///< 落下速度
const float Player::NORMAL_SPEED = 0.15f;					///< 通常スピードの値
const float Player::DASH_SPEED = 0.3f;						///< ダッシュスピードの値
const float Player::MAX_STAMINA = 50.0f;					///< スタミナ最大値
const float Player::STAMINA_COST_PER_SEC = 20.0f;			///< 秒ごとのスタミナコスト
const float Player::STAMINA_REGEN_RATE = 5.0f;				///< スタミナ回復率
const float Player::MIN_DASH_STAMINA = 10.0f;				///< スタミナの最小値
const float Player::DASH_RECOVERY_THRESHOLD_RATIO = 0.5f;	///< ダッシュ再開が可能になるスタミナの割合
const float Player::FALL_DEATH_THRESHOLD_Y = -12.0f;		///< 落下死と判定するY座標のしきい値	
const float Player::HALF_SCALE = 0.5f;						///< 半分のサイズにする
const float Player::TOP_Y_OFFSET_THRESHOLD = 0.5f;			///< 判定対象とする床の高さの許容誤差
const float Player::DEFAULT_SURFACE_Y = 0.0f;				///< 床が見つからなかった場合のデフォルトの高さ
const float Player::SHADOW_Z_FIGHTING_OFFSET = 0.03f;		///< 地面とのちらつきを防ぐための高さオフセット（影用）
const float Player::SHADOW_SCALE_ATTENUATION = 0.05f;		///< 高さによる影の減衰率
const float Player::MIN_SHADOW_SCALE = 0.2f;				///< 影の最小スケール

const float Player::BLINK_SPEED = 5.0f;											///< 点滅速度
const float Player::INVINCIBLE_BLINK_THRESHOLD = 0.5f;							///< 点滅のしきい値
const DirectX::SimpleMath::Vector3 Player::SWORD_OFFSET = { 0.4f, 0.1f, 0.0f };	///< 剣のオフセット値
const float Player::SWORD_ROT_Y = DirectX::XMConvertToRadians(90.0f);			///< 剣のＹの向き
const float Player::SWORD_ROT_X = DirectX::XMConvertToRadians(0.0f);			///< 剣のＸの向き
const float Player::SWORD_SCALE = 0.8f;											///< 剣の大きさ
const DirectX::SimpleMath::Vector3 Player::SHIELD_OFFSET = { -0.5f, 0.1f, 0.0f };///< 盾のオフセット値
const float Player::SHIELD_ROT_Y = DirectX::XMConvertToRadians(0.0f);			///< 盾のＹの向き
const float Player::SHIELD_ROT_X = DirectX::XMConvertToRadians(0.0f);			///< 盾のＸの向き
const float Player::SHIELD_SCALE = 0.6f;										///< 盾の大きさ

const float Player::SHIELD_INVINCIBILITY_DURATION = 1.0f;	///< シールド発動時の無敵時間（秒）
const float Player::DAMAGE_INVINCIBILITY_DURATION = 2.0f;	///< 被弾時の無敵時間（秒）

const int Player::CIRCLE_SEGMENTS = 32;						///< 円を表現する分割数
const float Player::SWORD_LINE_WIDTH = 1.1f;				///< 描画する線の太さ
const float Player::SWORD_Z_FIGHTING_OFFSET = 0.1f;			///< 地面とのちらつきを防ぐための高さオフセット（剣の範囲用）
const float Player::FULL_CIRCLE_TURN_RATIO = 2.0f;			///< ラジアン計算用の全周係数（2.0 * π = 360度）

const float Player::MOUSEWHEEL_MIN_RANGE = 1.0f;			///< マウスホイールの最小値
const float Player::MOUSEWHEEL_MAX_RANGE = 5.0f;			///< マウスホイールの最大値
const float Player::RANGE_STEP = 1.0f;						///< 一回のホイールの刻みで変化させる量
const int Player::WHEEL_TICKS = 120;						///< ホイール入力をノッチ数に変換する定数

const float Player::FIELD_OF_VIEW_DEGREES = 45.0f;			///< 視野角
const float Player::NEAR_PLANE_DISTANCE = 0.1f;				///< カメラの最前面のクリップ距離
const float Player::FAR_PLANE_DISTANCE = 100.0f;			///< カメラの最遠面のクリップ距離

/*
* @brief コンストラクタ
*
* @param[in]  なし
* 
* @return なし
*/
Player::Player()
	: m_floorHit(false),
	  m_isJumping(false),
	  m_verticalVelocity(0.0f),
	  m_lives(0),
	  m_attackCount(0),
	  m_defenseCount(0),
	  m_isInvincible(false),
	  m_isAttacking(false),
	  m_gameCamera(nullptr),
	  m_isGoal(false),
	  m_isDead(false),
	  m_invincibilityTime(0.0f),
	  m_attackRange(1.0f),
	  m_attackTimer(0.0f),
	  m_currentSurfaceY(0.0f)
{
}

/*
* @brief デストラクタ
*
* @param[in]  なし
* 
* @return なし
*/
Player::~Player()
{
}

/*
* @brief 初期化処理
*
* @param[in]  startPos 開始座標
* 
* @return なし
*/
void Player::Initialize(const DirectX::SimpleMath::Vector3& startPos)
{
	CreateDeviceDependentResources();

	// モデルの大きさを初期化
	m_playerScale = { 1.0f,1.45f,1.0f };

	// 位置設定
	m_startPosition = startPos;
	m_playerPosition = startPos;

	// 向きを初期化
	m_playerForward = { 0.0f, 0.0f, -1.0f };

	// チェックポイントの初期化
	m_respawnPoint = startPos;

	// 当たり判定の作成
	m_playerCollision = m_playerCollision.CreateAABB(m_playerPosition, m_playerScale);
	m_playerTrans = DirectX::SimpleMath::Matrix::CreateTranslation(m_playerPosition);

	// 残機の初期化
	m_lives = 3;

	// 攻撃可能回数／防御可能回数の初期化
	m_defenseCount = 0;
	m_attackCount = 0;

	// 無敵時間の初期化
	m_invincibilityTime = 0.0f;

	// スタミナの初期化
	m_stamina = MAX_STAMINA;
}

/*
* @brief 更新処理
*
* @param[in]  elapsedTime 前フレームからの経過時間
* 
* @return なし
*/
void Player::Update(float elapsedTime)
{
	// キー入力の取得
	auto kb = DirectX::Keyboard::Get().GetState();
	auto& mouse = DirectX::Mouse::Get();

	//現在のホイール値を取得
	int wheelDelta = mouse.GetState().scrollWheelValue;
	mouse.ResetScrollWheelValue();

	// ホイールが動いた場合のみ攻撃範囲を更新
	if (wheelDelta != 0 && m_attackCount >= 1)
	{
		UpdateAttackRange(wheelDelta);
		DirectX::Mouse::Get().ResetScrollWheelValue();
	}

	// サウンドマネージャーの更新
	auto& sound = SoundManager::GetInstance();
	sound.Update();

	// カメラの水平角度から前方ベクトルを計算
	DirectX::SimpleMath::Matrix rotationMatrix =
		DirectX::SimpleMath::Matrix::CreateRotationY(m_cameraHorizontalAngle);

	// チュートリアル表示中なら移動できないようにする
	if (m_tutorialManager && m_tutorialManager->IsPlayerLocked())
	{
		return;
	}

	// 移動方向ベクトル
	DirectX::SimpleMath::Vector3 moveDirection(0.0f, 0.0f, 0.0f);

	// キー入力で移動方向を決定
	if (kb.W) moveDirection.z -= NORMAL_SPEED;
	if (kb.S) moveDirection.z += NORMAL_SPEED;
	if (kb.A) moveDirection.x -= NORMAL_SPEED;
	if (kb.D) moveDirection.x += NORMAL_SPEED;

	// ダッシュの入力条件
	bool isDashInput = kb.LeftShift && !m_isJumping; 
	// ダッシュの実行判定
	bool isDashing = isDashInput && m_canDash && (m_stamina > 0.0f);

	// スタミナの処理
	if (isDashing)
	{
		// 消費
		m_stamina -= STAMINA_COST_PER_SEC * elapsedTime;

		// スタミナが０以下になったら
		if (m_stamina <= 0.0f)
		{
			m_stamina = 0.0f;
			m_canDash = false;
		}
	}
	else
	{
		// 回復処理
		if (m_stamina < MAX_STAMINA)
		{
			m_stamina += STAMINA_REGEN_RATE * elapsedTime;
			// 回復が最大値を超えないようにする
			if( m_stamina > MAX_STAMINA)
			{
				m_stamina = MAX_STAMINA;
			}
		}
		// スタミナが最低回復量を超えたらダッシュ再開可能にする
		if (m_stamina > MAX_STAMINA * DASH_RECOVERY_THRESHOLD_RATIO)
		{
			m_canDash = true;
		}
	}

	// 移動方向がある場合のみ処理
	if (moveDirection.LengthSquared() > 0.0f)
	{
		moveDirection.Normalize();

		// 最終的な速度を決定
		float speed = isDashing ? DASH_SPEED : NORMAL_SPEED;

		// 外部で設定されたカメラ角度に合わせて移動方向を回転
		rotationMatrix = DirectX::SimpleMath::Matrix::CreateRotationY(m_cameraHorizontalAngle);
		moveDirection = DirectX::SimpleMath::Vector3::Transform(moveDirection, rotationMatrix);

		// 移動を適用
		m_playerPosition += moveDirection * speed;

		// ダッシュパーティクルの処理
		if (isDashing && m_pDashParticle)
		{
			// エミッター情報の作成
			playerEmitterInfo info;
			info.position = m_playerPosition;
			info.scale = m_playerScale;

			std::vector<playerEmitterInfo> emitters = { info };

			// パーティクルにエミッター情報を渡す
			m_pDashParticle->SetEmitters(emitters);
		}

		// プレイヤーの向きを更新
		m_playerForward = moveDirection;
		m_playerForward.Normalize();
	}

	// ジャンプ処理
	if (InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Space) && m_floorHit && !m_isJumping)
	{
		m_isJumping = true;
		m_verticalVelocity = JUMP_POWER;
		m_floorHit = false;

		sound.Play(L"JUMP");
	}
	if (m_isJumping || !m_floorHit)
	{
		// 上昇中の処理
		m_verticalVelocity += GRAVITY * elapsedTime;
		m_playerPosition.y += m_verticalVelocity * elapsedTime;
	}
	else
	{
		m_verticalVelocity = 0.0f;
	}

	// 攻撃処理
	// 左クリックで攻撃
	if (m_attackCount >= 1 && InputManager::Get().IsMousePressedLeft())
	{
		sound.Play(L"ATTACK");

		m_attackCount--;
		m_isAttacking = true;
		m_attackTimer = 0.2f;
	}

	// 攻撃タイマーを減らす
	if (m_isAttacking) 
	{
		m_attackTimer -= elapsedTime;
		if (m_attackTimer <= 0.0f) 
		{
			m_isAttacking = false;
		}
	}
	
	// 防御時に効果音を再生
	if (m_isPlayShieldSound)
	{
		sound.Play(L"DEFENSE");
		m_isPlayShieldSound = false;
	}
	// ダメージ時に効果音を再生
	if (m_isPlayDamageSound)
	{
		sound.Play(L"DAMAGE");
		m_isPlayDamageSound = false;
	}

	// 落下したら
	if (m_playerPosition.y <= FALL_DEATH_THRESHOLD_Y)
	{
		sound.Play(L"FALL");
		// 残機を一つ減らす
		PlayerKill(); 
	}

	// 無敵時間の処理
	if (m_invincibilityTime > 0.0f)
		m_invincibilityTime -= elapsedTime;

	// リスポーン処理
	if (m_isDead) 
		Respawn();

	// AABBを更新
	m_playerCollision = m_playerCollision.CreateAABB(m_playerPosition, m_playerScale);

	// 攻撃コライダーの位置を更新
	DirectX::SimpleMath::Vector3 attackPos = m_playerPosition + (m_playerForward * m_attackRange);
	DirectX::SimpleMath::Vector3 attackScale = { 1.0f, 1.0f, 1.0f };
	m_attackCollision = m_attackCollision.CreateAABB(attackPos, attackScale);

	// ワールド行列を更新
	m_playerTrans = DirectX::SimpleMath::Matrix::CreateTranslation(m_playerPosition);
}

/*
* @brief 描画処理
*
* @param[in]  なし
* 
* @return なし
*/
void Player::Render()
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	auto states = m_states;

	// ビュー行列の取得
	if (m_gameCamera)
	{
		m_view = m_gameCamera->GetCameraMatrix();
	}
	else
	{
		// デフォルトのビュー行列を設定
		m_view = DirectX::SimpleMath::Matrix::CreateLookAt(
			DirectX::SimpleMath::Vector3(0, 10, 10),
			DirectX::SimpleMath::Vector3(0, 0, 0),
			DirectX::SimpleMath::Vector3(0, 1, 0)
		);
	}

	// プレイヤーのスケール設定
	DirectX::SimpleMath::Matrix playerScale = DirectX::SimpleMath::Matrix::CreateScale(m_playerScale);

	// プレイヤーの回転計算
	float playerRotationY = 0.0f;
	if (m_playerForward.LengthSquared() > 0.0f)
	{
		playerRotationY = atan2f(m_playerForward.x, m_playerForward.z);
	}
	DirectX::SimpleMath::Matrix playerRot = DirectX::SimpleMath::Matrix::CreateRotationY(playerRotationY);

	// プレイヤーのワールド行列
	DirectX::SimpleMath::Matrix playerWorld = playerScale * playerRot * m_playerTrans;

	if (m_invincibilityTime > 0.0f)
	{
		// 点滅のON／OFFを判定
		if (fmod(m_invincibilityTime * BLINK_SPEED, 1.0f) < INVINCIBLE_BLINK_THRESHOLD)
		{
			// モデルの描画
			m_playerModel->Draw(context, *states, playerWorld, m_view, m_proj);
		}
	}
	else
	{
		// 通常描画
		m_playerModel->Draw(context, *states, playerWorld, m_view, m_proj);
	}

	// 所持アイテムの描画：剣
	if (m_havingSwordModel && m_attackCount >= 1)
	{
		// オフセットの作成
		DirectX::SimpleMath::Matrix swordTrans = DirectX::SimpleMath::Matrix::CreateTranslation(SWORD_OFFSET);

		// 剣の回転
		DirectX::SimpleMath::Matrix swordRotY = DirectX::SimpleMath::Matrix::CreateRotationY(SWORD_ROT_Y);
		DirectX::SimpleMath::Matrix swordRotX = DirectX::SimpleMath::Matrix::CreateRotationX(SWORD_ROT_X);

		// ソードのスケール
		DirectX::SimpleMath::Matrix swordScale = DirectX::SimpleMath::Matrix::CreateScale(SWORD_SCALE);

		// プレイヤーのワールド行列×オフセット
		DirectX::SimpleMath::Matrix swordWorld = swordScale * swordRotX * swordRotY * swordTrans * playerWorld;

		// 描画
		m_havingSwordModel->Draw(context, *states, swordWorld, m_view, m_proj);
	}

	// 所持アイテムの描画：盾
	if (m_havingShieldModel && m_defenseCount >= 1)
	{
		// オフセットの作成
		DirectX::SimpleMath::Matrix shieldTrans = DirectX::SimpleMath::Matrix::CreateTranslation(SHIELD_OFFSET);

		// 盾の回転
		DirectX::SimpleMath::Matrix shieldRotY = DirectX::SimpleMath::Matrix::CreateRotationY(SHIELD_ROT_Y);
		DirectX::SimpleMath::Matrix shieldRotX = DirectX::SimpleMath::Matrix::CreateRotationY(SHIELD_ROT_X);

		// 盾のスケール
		DirectX::SimpleMath::Matrix shieldScale = DirectX::SimpleMath::Matrix::CreateScale(SHIELD_SCALE);

		// プレイヤーのワールド行列×オフセット
		DirectX::SimpleMath::Matrix shieldWorld = shieldScale * shieldRotX * shieldRotY * shieldTrans * playerWorld;

		// 描画
		m_havingShieldModel->Draw(context, *states, shieldWorld, m_view, m_proj);
	}
}

/*
* @brief 終了処理
*
* @param[in]  なし
* 
* @return なし
*/
void Player::Finalize()
{
}

/*
* @brief デバイスに依存するリソースを作成する関数
*
* @param[in]  なし
* 
* @return なし
*/
void Player::CreateDeviceDependentResources()
{
	auto device = m_deviceResources->GetD3DDevice();
	auto context = m_deviceResources->GetD3DDeviceContext();

	// スプライトバッチの作成
	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(context);

	// エフェクトファクトリーの作成
	std::unique_ptr<DirectX::EffectFactory> fx = std::make_unique<DirectX::EffectFactory>(device);
	fx->SetDirectory(L"Resources/Models");

	// プリミティブバッチの初期化
	m_primitiveBatch = std::make_unique<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>(context);

	// モデルの読み込み
	m_playerModel = DirectX::Model::CreateFromSDKMESH(device, L"Resources/Models/player.sdkmesh", *fx);

	// アイテムのモデルの読み込み
	m_havingSwordModel = DirectX::Model::CreateFromSDKMESH(device, L"Resources/Models/sword.sdkmesh", *fx);
	m_havingShieldModel = DirectX::Model::CreateFromSDKMESH(device, L"Resources/Models/shield.sdkmesh", *fx);

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
		DirectX::XMConvertToRadians(FIELD_OF_VIEW_DEGREES),
		static_cast<float>(rect.right) / static_cast<float>(rect.bottom),
		NEAR_PLANE_DISTANCE, FAR_PLANE_DISTANCE
	);
}

/*
* @brief 影の描画
*
* @param[in]  なし
* 
* @return なし
*/
void Player::RenderShadow()
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	const auto defaultLightDir = DirectX::SimpleMath::Vector3(0.2f, 1.0f, 0.2f);

	// ライトの方向（斜め上から - 影が見えやすい角度）
	DirectX::SimpleMath::Vector3 lightDir = defaultLightDir;
	lightDir.Normalize();

	// プレイヤーが立っている床を検出
	float playerX = m_playerPosition.x;
	float playerY = m_playerPosition.y;
	float playerZ = m_playerPosition.z;
	float closestSurfaceY = m_currentSurfaceY;

	// 床と足場の両方をチェックする共通ラムダ式
	auto checkSurface = [&](const std::vector<DirectX::SimpleMath::Vector3>& posList,
		const std::vector<DirectX::SimpleMath::Vector3>& scaleList)
		{
			for (size_t i = 0; i < posList.size(); ++i)
			{
				// オブジェクトの半分の幅・奥行きを計算
				float halfWidth = scaleList[i].x * HALF_SCALE;
				float halfDepth = scaleList[i].z * HALF_SCALE;

				// このオブジェクトのXZ範囲
				float minX = posList[i].x - halfWidth;
				float maxX = posList[i].x + halfWidth;
				float minZ = posList[i].z - halfDepth;
				float maxZ = posList[i].z + halfDepth;

				// プレイヤーがXZ範囲内にいるかチェック
				if (playerX >= minX && playerX <= maxX &&
					playerZ >= minZ && playerZ <= maxZ)
				{
					// このオブジェクトの上面のY座標
					float topY = posList[i].y + (scaleList[i].y * HALF_SCALE);

					// プレイヤーが上にいる場合、より高い上面を候補にする
					if (topY <= playerY && topY > closestSurfaceY)
					{
						closestSurfaceY = topY;
					}
				}
			}
		};

	// 静的な床の判定を実行
	checkSurface(m_floorPositions, m_floorScales);

	// 動的な足場の判定を実行
	checkSurface(m_platformPositions, m_platformScales);

	// Zファイティング防止のため、少し上げる
	closestSurfaceY += SHADOW_Z_FIGHTING_OFFSET;

	// シャドウマトリクスの作成
	DirectX::SimpleMath::Plane groundPlane = DirectX::SimpleMath::Plane(0.0f, 1.0f, 0.0f, -closestSurfaceY);
	DirectX::SimpleMath::Matrix shadowMatrix = DirectX::SimpleMath::Matrix::CreateShadow(lightDir, groundPlane);

	// 影の描画設定
	if (m_floorHit || m_isJumping)
	{
		// 影のスケーリングを計算
		float scaleFactor = 1.0f;

		if (m_isJumping)
		{
			//ジャンプ中は影を小さくする
			float heightDiff = m_playerPosition.y - closestSurfaceY;
			scaleFactor = 1.0f / (1.0f + heightDiff * SHADOW_SCALE_ATTENUATION);
			scaleFactor = std::max(MIN_SHADOW_SCALE, scaleFactor);
		}

		DirectX::SimpleMath::Matrix shadowScale = DirectX::SimpleMath::Matrix::CreateScale(scaleFactor);

		// プレイヤーの回転計算
		float playerRotationY = 0.0f;
		if (m_playerForward.LengthSquared() > 0.0f)
		{
			playerRotationY = atan2f(m_playerForward.x, m_playerForward.z);
		}
		DirectX::SimpleMath::Matrix playerRot = DirectX::SimpleMath::Matrix::CreateRotationY(playerRotationY);

		// 影の位置計算
		DirectX::SimpleMath::Vector3 shadowPos = m_playerPosition;
		DirectX::SimpleMath::Matrix shadowTrans = DirectX::SimpleMath::Matrix::CreateTranslation(shadowPos);

		// 影のワールド行列を作成
		DirectX::SimpleMath::Matrix shadowWorldBase = shadowScale * playerRot * shadowTrans;

		// シャドウマトリクスを適用
		DirectX::SimpleMath::Matrix finalShadowMatrix = shadowWorldBase * shadowMatrix;

		// 影の描画
		m_shadowModel->Draw(context, *m_states, finalShadowMatrix, m_view, m_proj,
			false, [&]()
			{
				// アルファブレンドを有効にして透明な影を描画
				context->OMSetBlendState(m_states->AlphaBlend(), nullptr, 0xffffffff);

				// 深度テストは有効、深度書き込みは無効
				context->OMSetDepthStencilState(m_depthStencilState_Shadow.Get(), 1);

				// 背面カリングを無効にする（影が裏返ったときも見えるように）
				context->RSSetState(m_states->CullNone());

				// ピクセルシェーダーの設定
				context->PSSetShader(m_PS.Get(), nullptr, 0);
			}
		);
	}
}

/*
* @brief ダメージ処理
*
* @param[in]  なし
* 
* @return なし
*/
void Player::TakeDamage()
{
	if (m_invincibilityTime > 0.0f) return;

	// 防御処理
	if (m_defenseCount > 0)
	{
		m_defenseCount--;
		m_invincibilityTime = SHIELD_INVINCIBILITY_DURATION;
		m_isPlayShieldSound = true;
	}
	// ダメージ処理
	else if (m_lives > 0) 
	{
		m_lives--;
		m_invincibilityTime = DAMAGE_INVINCIBILITY_DURATION;
		m_isPlayDamageSound = true;
		m_isDead = true;
	}
}

/*
* @brief プレイヤーのリスポーン
*
* @param[in]  なし
* 
* @return なし
*/
void Player::Respawn()
{
	// スタート位置に戻す
	m_playerPosition = m_startPosition;

	// 各処理のリセット
	m_isDead = false;
	m_isJumping = false;
	m_floorHit = false;
	m_verticalVelocity = 0.0f;

	// 所持品のリセット
	m_attackCount = 0;
	m_defenseCount = 0;

	m_playerTrans = DirectX::SimpleMath::Matrix::CreateTranslation(m_playerPosition);
	m_playerCollision = m_playerCollision.CreateAABB(m_playerPosition, m_playerScale);
}

/*
* @brief 当たり判定の更新
*
* @param[in]  collision 当たり判定
* @param[in]  position　位置
* 
* @return なし
*/
void Player::UpdateCollision(const AABB& collision, const DirectX::SimpleMath::Vector3& position)
{
	if (!m_playerCollision.CheckAABBCollision(m_playerCollision, collision))
		return;

	// プレイヤーと足場の重なり量を計算
	DirectX::SimpleMath::Vector3 overlap;
	overlap.x = std::min(m_playerCollision.max.x, collision.max.x) -
		std::max(m_playerCollision.min.x, collision.min.x);
	overlap.y = std::min(m_playerCollision.max.y, collision.max.y) -
		std::max(m_playerCollision.min.y, collision.min.y);
	overlap.z = std::min(m_playerCollision.max.z, collision.max.z) -
		std::max(m_playerCollision.min.z, collision.min.z);

	// 最も小さい重なり方向に押し戻す
	if (overlap.y <= overlap.x && overlap.y <= overlap.z)
	{
		float playerBottom = m_playerPosition.y - (m_playerScale.y * 0.5f);
		float objectTop = position.y + (collision.max.y - collision.min.y) * 0.5f;

		// 着地の許容範囲
		const float landingThreshold = 0.2f;

		// 上から着地した場合
		if (playerBottom >= objectTop - landingThreshold && m_verticalVelocity <= 0.0f)
		{
			// プレイヤーを床の上に正しく置く
			m_playerPosition.y = objectTop + (m_playerScale.y * 0.5f);

			// フラグ更新
			m_floorHit = true;
			m_isJumping = false;
			m_verticalVelocity = 0.0f;

			// 着地した面のY座標を記録
			m_currentSurfaceY = objectTop;
		}
		else
		{
			// 下からぶつかった場合
			m_playerPosition.y -= overlap.y;
			m_verticalVelocity = 0.0f;
		}
	}
	else if (overlap.x < overlap.z)
	{
		// X方向の衝突
		if (m_playerPosition.x < position.x)
			m_playerPosition.x -= overlap.x;
		else
			m_playerPosition.x += overlap.x;
	}
	else
	{
		// Z方向の衝突
		if (m_playerPosition.z < position.z)
			m_playerPosition.z -= overlap.z;
		else
			m_playerPosition.z += overlap.z;
	}

	// AABB再生成
	m_playerCollision = m_playerCollision.CreateAABB(m_playerPosition, m_playerScale);
}

/*
* @brief マウスホイール入力に基づいて攻撃範囲を更新する
*
* @param[in] wheelDelta マウスホイールの回転量
* 
* @return なし
*/
void Player::UpdateAttackRange(int wheelDelta)
{
	// 変化量を計算
	float delta = (float)wheelDelta / WHEEL_TICKS * RANGE_STEP;

	// 攻撃範囲を更新
	m_attackRange += delta;

	// 範囲制限の間に収める
#ifdef __cpp_lib_clamp
	m_attackRange = std::clamp(m_attackRange, MIN_RANGE, MAX_RANGE);
#else
	m_attackRange = std::max(MOUSEWHEEL_MIN_RANGE, std::min(MOUSEWHEEL_MAX_RANGE, m_attackRange));
#endif // __cpp_lib_clamp
}

/*
* @brief 床情報を設定するメソッド
*
* @param[in]  positions　床の位置
* @param[in]  scales　　床の大きさ
* 
* @return なし
*/
void Player::SetFloorData(const std::vector<DirectX::SimpleMath::Vector3>& f_positions, const std::vector<DirectX::SimpleMath::Vector3>& f_scales)
{
	m_floorPositions = f_positions;
	m_floorScales = f_scales;
}

/*
* @brief 足場情報を設定するメソッド
*
* @param[in]  positions　足場の位置
* @param[in]  scales　　足場の大きさ
* 
* @return なし
*/
void Player::SetPlatformData(const std::vector<DirectX::SimpleMath::Vector3>& pf_positions, const std::vector<DirectX::SimpleMath::Vector3>& pf_scales)
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
void Player::SetPosition(float x, float y, float z)
{
	m_playerPosition.x = x;
	m_playerPosition.y = y;
	m_playerPosition.z = z;
}

/*
* @brief 床の着地状態をリセット
*
* @param[in]  なし
* 
* @return なし
*/
void Player::ResetFloorHit()
{
	m_floorHit = false;
}

/*
* @brief 死亡処理
*
* @param[in]  なし
* 
* @return なし
*/
void Player::PlayerKill()
{
	// 無敵時間中だったら処理しない
	if (m_invincibilityTime > 0.0f) return;

	// プレイヤー死亡処理
	m_lives--;
	m_isDead = true;
	m_invincibilityTime = 2.0f;
}

/*
* @brief 剣の範囲の円
*
* @param[in]  なし
* 
* @return なし
*/
void Player::SwordRangeCircle()
{
	if (!m_primitiveBatch) return;

	m_primitiveBatch->Begin();

	// 線の色
	auto lineColor = DirectX::Colors::OrangeRed;

	// 攻撃コリジョンのAABBから半径と内半径を計算
	const float radius = (m_attackCollision.max.x - m_attackCollision.min.x) * HALF_SCALE;
	const float innerRadius = radius - SWORD_LINE_WIDTH;

	// 中心座標の計算
	DirectX::SimpleMath::Vector3 center;
	center.x = (m_attackCollision.min.x + m_attackCollision.max.x) * HALF_SCALE;
	center.y = m_attackCollision.min.y + SWORD_Z_FIGHTING_OFFSET;
	center.z = (m_attackCollision.min.z + m_attackCollision.max.z) * HALF_SCALE;

	// 最初のセグメントの頂点を計算
	DirectX::SimpleMath::Vector3 prevOuter, prevInner;

	// 外側の最初の頂点
	prevOuter.x = center.x + radius;
	prevOuter.y = center.y;
	prevOuter.z = center.z;

	// 内側の最初の頂点
	prevInner.x = center.x + innerRadius;
	prevInner.y = center.y;
	prevInner.z = center.z;

	// セグメントごとに線を描画
	for (int i = 1; i <= CIRCLE_SEGMENTS; ++i)
	{
		// 現在の角度（ラジアン）を計算
		float angle = (float)i * (FULL_CIRCLE_TURN_RATIO * DirectX::XM_PI / (float)CIRCLE_SEGMENTS);
		float cosA = cosf(angle);
		float sinA = sinf(angle);

		// 現在のセグメントの外周と内周の頂点を計算
		DirectX::SimpleMath::Vector3 currentOuter =
		{
			center.x + radius * cosA,
			center.y,
			center.z + radius * sinA
		};
		DirectX::SimpleMath::Vector3 currentInner =
		{
			center.x + innerRadius * cosA,
			center.y,
			center.z + innerRadius * sinA
		};

		// トライアングルリスト形式でドーナツ状の四角形を描画
		m_primitiveBatch->DrawTriangle(
			{ prevOuter,   lineColor },
			{ prevInner,   lineColor },
			{ currentInner,lineColor }
		);
		m_primitiveBatch->DrawTriangle(
			{ currentOuter,lineColor },
			{ prevOuter,   lineColor },
			{ currentInner,lineColor }
		);

		// 次のループのために頂点を更新
		prevOuter = currentOuter;
		prevInner = currentInner;
	}
	m_primitiveBatch->End();
}

/*
* @brief コライダーの線
*
* @param[in] なし
* 
* @return なし
*/
void Player::ColliderLine()
{
	if (!m_primitiveBatch) return;

	// コライダー線の色
	auto lineColorA = DirectX::Colors::Red;
	auto lineColorB = DirectX::Colors::MediumPurple;

	m_primitiveBatch->Begin();
	// AABBの頂点を計算
	DirectX::SimpleMath::Vector3 playerCorners[8] =
	{
		{ m_playerCollision.min.x, m_playerCollision.min.y, m_playerCollision.min.z },
		{ m_playerCollision.max.x, m_playerCollision.min.y, m_playerCollision.min.z },
		{ m_playerCollision.max.x, m_playerCollision.max.y, m_playerCollision.min.z },
		{ m_playerCollision.min.x, m_playerCollision.max.y, m_playerCollision.min.z },
		{ m_playerCollision.min.x, m_playerCollision.min.y, m_playerCollision.max.z },
		{ m_playerCollision.max.x, m_playerCollision.min.y, m_playerCollision.max.z },
		{ m_playerCollision.max.x, m_playerCollision.max.y, m_playerCollision.max.z },
		{ m_playerCollision.min.x, m_playerCollision.max.y, m_playerCollision.max.z }
	};
	DirectX::SimpleMath::Vector3 attackCorners[8] =
	{
		{ m_attackCollision.min.x, m_attackCollision.min.y, m_attackCollision.min.z },
		{ m_attackCollision.max.x, m_attackCollision.min.y, m_attackCollision.min.z },
		{ m_attackCollision.max.x, m_attackCollision.max.y, m_attackCollision.min.z },
		{ m_attackCollision.min.x, m_attackCollision.max.y, m_attackCollision.min.z },
		{ m_attackCollision.min.x, m_attackCollision.min.y, m_attackCollision.max.z },
		{ m_attackCollision.max.x, m_attackCollision.min.y, m_attackCollision.max.z },
		{ m_attackCollision.max.x, m_attackCollision.max.y, m_attackCollision.max.z },
		{ m_attackCollision.min.x, m_attackCollision.max.y, m_attackCollision.max.z }
	};

	/*/////////////////////////////////////プレイヤーの線の描画/////////////////////////////////*/

	// 前面の線を描画
	m_primitiveBatch->DrawLine({ playerCorners[0], lineColorA }, { playerCorners[1], lineColorA });
	m_primitiveBatch->DrawLine({ playerCorners[1], lineColorA }, { playerCorners[2], lineColorA });
	m_primitiveBatch->DrawLine({ playerCorners[2], lineColorA }, { playerCorners[3], lineColorA });
	m_primitiveBatch->DrawLine({ playerCorners[3], lineColorA }, { playerCorners[0], lineColorA });

	m_primitiveBatch->DrawLine({ attackCorners[0], lineColorB }, { attackCorners[1], lineColorB });
	m_primitiveBatch->DrawLine({ attackCorners[1], lineColorB }, { attackCorners[2], lineColorB });
	m_primitiveBatch->DrawLine({ attackCorners[2], lineColorB }, { attackCorners[3], lineColorB });
	m_primitiveBatch->DrawLine({ attackCorners[3], lineColorB }, { attackCorners[0], lineColorB });

	// 背面の線を描画
	m_primitiveBatch->DrawLine({ playerCorners[4], lineColorA }, { playerCorners[5], lineColorA });
	m_primitiveBatch->DrawLine({ playerCorners[5], lineColorA }, { playerCorners[6], lineColorA });
	m_primitiveBatch->DrawLine({ playerCorners[6], lineColorA }, { playerCorners[7], lineColorA});
	m_primitiveBatch->DrawLine({ playerCorners[7], lineColorA }, { playerCorners[4], lineColorA });

	m_primitiveBatch->DrawLine({ attackCorners[4], lineColorB }, { attackCorners[5], lineColorB });
	m_primitiveBatch->DrawLine({ attackCorners[5], lineColorB }, { attackCorners[6], lineColorB });
	m_primitiveBatch->DrawLine({ attackCorners[6], lineColorB }, { attackCorners[7], lineColorB });
	m_primitiveBatch->DrawLine({ attackCorners[7], lineColorB }, { attackCorners[4], lineColorB});

	// 前面と背面をつなぐ線を描画
	m_primitiveBatch->DrawLine({ playerCorners[0], lineColorA }, { playerCorners[4], lineColorA });
	m_primitiveBatch->DrawLine({ playerCorners[1], lineColorA }, { playerCorners[5], lineColorA });
	m_primitiveBatch->DrawLine({ playerCorners[2], lineColorA }, { playerCorners[6], lineColorA });
	m_primitiveBatch->DrawLine({ playerCorners[3], lineColorA }, { playerCorners[7], lineColorA });

	m_primitiveBatch->DrawLine({ attackCorners[0], lineColorB }, { attackCorners[4], lineColorB });
	m_primitiveBatch->DrawLine({ attackCorners[1], lineColorB }, { attackCorners[5], lineColorB });
	m_primitiveBatch->DrawLine({ attackCorners[2], lineColorB }, { attackCorners[6], lineColorB });
	m_primitiveBatch->DrawLine({ attackCorners[3], lineColorB }, { attackCorners[7], lineColorB });

	m_primitiveBatch->End();
}