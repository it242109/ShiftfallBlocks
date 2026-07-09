//--------------------------------------------------------------------------------------
// File: StageScene.cpp
//
// ステージシーンクラス
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "StageScene.h"
#include "Resources/json.hpp"

#include "GameObjects/Gimmicks/Platform.h"
#include "GameObjects/StageObjects/StageObject.h"

// JSONファイルを使えるようにする
using json = nlohmann::json;

// 定数の定義
const float StageScene::CAMERA_DISTANCE = 8.0f;				///< カメラの初期の距離
const DirectX::SimpleMath::Vector3 StageScene::PLAYER_INITIAL_POSITION = { 2.0f, 2.0f, -3.0f };	///< プレイヤーの初期位置
const DirectX::SimpleMath::Vector2 StageScene::NUMBER_POSITION = { 570.0f, 60.0f };				///< タイマーで使われる数字の初期位置

const float StageScene::FALLTODEATH_HEIGHT = -15.0f;		///< 落下死する高さ
const int StageScene::ATTACK_COUNT = 1;						///< 攻撃回数
const float StageScene::WAIT_TIME = 3.0f;					///< ゴールまたはゲームオーバー後の待ち時間
const float StageScene::TELEPORT_COOLDOWN_TIME = 2.0f;		///< テレポートした後のクールダウンタイム
const float StageScene::TIMER_END_THRESHOLD = 0.0f;			///< タイマーが終了したと判定する基準値
const float StageScene::INVINCIBILITY_END_THRESHOLD = 0.0f;	///< 無敵時間が終了した基準値
const int StageScene::GAMEOVER_LIFE_COUNT = 0;				///< ゲームオーバーとなる残機の数

const float StageScene::FONT_INITIAL_POSITION_X = -100.0f;	///< クリア／ゲームオーバーフォントの初期位置
const float StageScene::FONT_X_MAX = 250.0f;				///< クリア／ゲームオーバーフォントXの最大数値
const float StageScene::FONT_SPEED = 700.0f;				///< クリア／ゲームオーバーフォントの移動速度

const float StageScene::FADE_SPEED = 1.0f;					///< テレポートによるフェード速度
const float StageScene::OPAQUE_OVERLAY_ALPHA = 0.95f;		///< 不透明のオーバーレイのα値

const float StageScene::MENU_DEFAULT_POSITION_X = 600.0f;	///< メニューのデフォルトの位置X
const float StageScene::MENU_DEFAULT_SCALE_X = 0.8f;		///< メニューのデフォルトの大きさX
const float StageScene::MENU_DEFAULT_SCALE_Y = 0.8f;		///< メニューのデフォルトの大きさY
const float StageScene::DEFAULT_SRV_SCALE_X = 1.0f;			///< ＳＲＶのデフォルトの大きさX
const float StageScene::DEFAULT_SRV_SCALE_Y = 1.0f;			///< ＳＲＶのデフォルトの大きさY

const int StageScene::BASE_SCREEN_WIDTH = 1280;				///< ゲームの基本画面解像度（横幅）
const int StageScene::BASE_SCREEN_HEIGHT = 720;				///< ゲームの基本画面解像度（縦幅）

const float StageScene::FIELD_OF_VIEW_DEGREES = 45.0f;		///< 視野角
const float StageScene::NEAR_PLANE_DISTANCE = 0.1f;			///< カメラの最前面のクリップ距離
const float StageScene::FAR_PLANE_DISTANCE = 100.0f;		///< カメラの最遠面のクリップ距離

/*
* @brief コンストラクタ
*
* @param[in]  なし
*
* @return なし
*/
StageScene::StageScene()
	:
	m_isPause(false),
	m_isTeleporting(false),
	m_timer(0.0f), 
	m_teleportTimer(0.0f),
	m_number(),
	m_isDebugMode(false),
	m_teleportOverlayAlpha(0.0f),
	m_waitTimer(0.0f),	
	m_isGoalWaiting(false),
	m_isGameOverWaiting(false),
	m_isTimerActive(false),
	m_fontPosX(0.0f),
	m_isSEPlayed(false)
{
}

/*
* @brief デストラクタ
*
* @param[in]  なし
*
* @return なし
*/
StageScene::~StageScene()
{
}

/*
* @brief 初期化処理
*
* @param[in]  なし
*
* @return なし
*/
void StageScene::Initialize()
{
    // JSONファイルの読み込み
	std::string filePath = SelectScene::GetCurrentStageFilePath();

    // 敵の読み込み
    m_enemyStartPositions.clear();
    std::ifstream fileStream(filePath);
    if (fileStream.is_open())
    {
        json data;
        fileStream >> data;

        if (data.contains("enemies"))
        {
            for (auto& e : data["enemies"])
            {
                m_enemyStartPositions.push_back
                ({
                    e["position"][0].get<float>(),
                    e["position"][1].get<float>(),
                    e["position"][2].get<float>()
                });
            }
        }
    }

    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();

	// カメラの作成　
	RECT rect = GetUserResources()->GetDeviceResources()->GetOutputSize();
	m_gameCamera = std::make_unique<GameCamera>(rect.right, rect.bottom);

	// カメラの角度を取得
	m_gameCamera->SetAngle(DirectX::XMConvertToRadians(0.0f), DirectX::XMConvertToRadians(-10.0f));
	
	// カメラの距離を設定
	m_gameCamera->SetDistance(CAMERA_DISTANCE);

    // プレイヤーとテレポート時の処理を渡してロード
    m_stage->Load(filePath, m_player.get(), [this]() {
        m_isTeleporting = true;
		m_teleportOverlayAlpha = 0.0f;
        m_teleportTimer = TELEPORT_COOLDOWN_TIME;
        SoundManager::GetInstance().Play(L"TELEPORT");
    });

    // タスクマネージャーの初期化／数字の初期化
    m_number = m_taskManager.AddTask<Number>(&m_spriteBatch, m_numberSRV.GetAddressOf());
    m_number->SetNumberDecimal(m_timer);
    m_number->SetPosition(DirectX::SimpleMath::Vector2(NUMBER_POSITION));

    // タイマーの初期化
    m_timer = 0.0f;
    m_isTimerActive = true;

	// ゲームクリア／ゲームオーバーフォントの初期化
	m_fontPosX = FONT_INITIAL_POSITION_X;

    // メニューの初期化
    m_pauseMenu->Add(L"Resources/Textures/resume.png"
        , ScreenManager::Pos(MENU_DEFAULT_POSITION_X, 200)
        , ScreenManager::Scale(MENU_DEFAULT_SCALE_X, MENU_DEFAULT_SCALE_Y)
        , ANCHOR::MIDDLE_CENTER);
    m_pauseMenu->Add(L"Resources/Textures/retry.png"
        , ScreenManager::Pos(MENU_DEFAULT_POSITION_X, 400)
        , ScreenManager::Scale(MENU_DEFAULT_SCALE_X, MENU_DEFAULT_SCALE_Y)
        , ANCHOR::MIDDLE_CENTER);
	m_pauseMenu->Add(L"Resources/Textures/select.png"
		, ScreenManager::Pos(MENU_DEFAULT_POSITION_X, 600)
		, ScreenManager::Scale(MENU_DEFAULT_SCALE_X, MENU_DEFAULT_SCALE_Y)
		, ANCHOR::MIDDLE_CENTER);

    // ステージの初期化
	ResetGame();
}

/*
* @brief 更新処理
*
* @param[in]  elapsedTime 前フレームからの経過時間
*
* @return なし
*/
void StageScene::Update(float elapsedTime)
{
	// タイマーの更新　
	if (m_isTimerActive && !m_isPause) m_timer += elapsedTime;
	m_number->SetTimer(static_cast<int>(m_timer));
	m_number->Update(elapsedTime);
	// テレポート処理の更新
	if (m_isTeleporting)
	{
		// テレポートタイマー
		m_teleportTimer -= elapsedTime;
		if (m_teleportTimer <= TIMER_END_THRESHOLD)
		{
			m_isTeleporting = false;
		}
		// テレポートによるフェードインのα値
		m_teleportOverlayAlpha += FADE_SPEED * elapsedTime;
		if (m_teleportOverlayAlpha > OPAQUE_OVERLAY_ALPHA)
		{
			m_teleportOverlayAlpha = OPAQUE_OVERLAY_ALPHA;
		}
		// 渦巻パーティクルの更新
		m_swirlParticle->Update(elapsedTime);
	}

	// キーのインプットマネージャーの更新
	InputManager::Get().Update();

	// サウンドマネージャーの更新
	SoundManager::GetInstance().Update();

	// プレイヤーの当たり判定を取得
	const auto& playerCollider = m_player->GetCollision();

	// ステージの更新
	if (!m_isPause && !m_isGoalWaiting && !m_isGameOverWaiting)
	{
		m_stage->Update(elapsedTime, m_player.get(), m_enemies, m_isTeleporting);
	}

	// ポーズニューの処理
	if (InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::T) && !m_isGoalWaiting && !m_isGameOverWaiting) m_isPause = true;
	if (m_isPause)
	{
		if (InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Down)
			|| InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::S))
		{
			SoundManager::GetInstance().Play(L"SELECT");
			if (m_pauseMenu->m_menuIndex > 2) m_pauseMenu->m_menuIndex = 0;
		}
		if (InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Up)
			|| InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::W))
		{
			SoundManager::GetInstance().Play(L"SELECT");
			if (m_pauseMenu->m_menuIndex < 0) m_pauseMenu->m_menuIndex = 2;
		}

		// シーン切り替え
		if (m_pauseMenu->m_menuIndex == 0 && InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Space))
		{
			SoundManager::GetInstance().Play(L"DECISION");
			m_isPause = false;
			m_pauseMenu->m_menuIndex = 0;
		}
		if (m_pauseMenu->m_menuIndex == 1 && InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Space))
		{
			SoundManager::GetInstance().Play(L"DECISION");
			ResetGame();
			m_pauseMenu->m_menuIndex = 0;
		}
		if (m_pauseMenu->m_menuIndex == 2 && InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Space))
		{
			SoundManager::GetInstance().Play(L"DECISION");
			SoundManager::GetInstance().Stop(L"GAMEPLAYBGM");
			m_pauseMenu->m_menuIndex = 0;
			ChangeScene<SelectScene>();
		}
		m_pauseMenu->Update();
		return;
	}

	// プレイヤーの更新
	if (!m_isGoalWaiting && !m_isGameOverWaiting && !m_isTeleporting && !m_isPause)
	{
		m_player->Update(elapsedTime);
		// ダッシュパーティクルの更新
		m_dashParticle->Update(elapsedTime);
	}
	
	// 敵の更新
	for (auto& outerEnemy : m_enemies)
	{
		if (!outerEnemy->IsDead() && !m_isPause && !m_isGoalWaiting && !m_isGameOverWaiting)
		{
			auto platform = m_stage->GetPlatform();
			outerEnemy->SetPlatformData(platform->GetPositions(), platform->GetScales());
			outerEnemy->Update(elapsedTime, m_player->GetPosition());
		}
		// 敵の当たり判定を取得
		AABB outerEnemyCollider = outerEnemy->GetCollision();
		AABB outerEnemyDamageCollider = outerEnemy->GetDamageCollision();
		// 敵が落下したら
		if (outerEnemy->GetPosition().y <= FALLTODEATH_HEIGHT)
		{
			if (!outerEnemy->IsDead())
			{
				outerEnemy->EnemyKill();
			}
		}

		// プレイヤー攻撃判定
		if (m_player->IsAttacking())
		{
			const auto& playerAttackCollider = m_player->GetAttackCollision();
			for (auto& targetEnemy : m_enemies)
			{
				if (!targetEnemy->IsDead())
				{
					const auto& targetEnemyCollider = targetEnemy->GetCollision();
					if (playerAttackCollider.CheckAABBCollision(playerAttackCollider, targetEnemyCollider))
					{
						targetEnemy->EnemyKill();
					}
				}
			}
		}
		// 敵とぶつかったら
		if (!outerEnemy->IsDead() &&
			outerEnemyDamageCollider.CheckAABBCollision(playerCollider, outerEnemyCollider) &&
			m_player->GetInvincibilityTime() <= INVINCIBILITY_END_THRESHOLD)
		{
			// 敵とのダメージ処理
			m_player->TakeDamage();
		}
	}
	// 敵同士の当たり判定
	for (size_t i = 0; i < m_enemies.size(); ++i)
	{
		if (m_enemies[i]->IsDead()) continue;
		const AABB& enemyColliderA = m_enemies[i]->GetCollision();

		for (size_t j = i + 1; j < m_enemies.size(); ++j)
		{
			if (m_enemies[j]->IsDead())continue;
			const AABB& enemyColliderB = m_enemies[j]->GetCollision();

			// 敵同士でぶつかったら
			if (enemyColliderA.CheckAABBCollision(enemyColliderA, enemyColliderB))
			{
				m_enemies[i]->UpdateCollision(m_enemies[j]->GetCollision(), m_enemies[j]->GetPosition());
				m_enemies[j]->UpdateCollision(m_enemies[i]->GetCollision(), m_enemies[i]->GetPosition());
			}
		}
	}

	// ゲームオーバー条件
	if (m_player->GetLives() <= GAMEOVER_LIFE_COUNT && !m_isGameOverWaiting)
	{
		SoundManager::GetInstance().Stop(L"GAMEPLAYBGM");
		SoundManager::GetInstance().Stop(L"FALL");

		// タイマーを止める
		m_isTimerActive = false;
		// 判定
		ResultScene::SetGlobalResult(ResultScene::ResultType::GAMEOVER);
		// どのステージか
		ResultScene::SetGlobalStage(ResultScene::GetGlobalStage());

		m_isGameOverWaiting = true;
		m_waitTimer = 0.0f;
		return;
	}
	// ゲームオーバー後の処理
	if (m_isGameOverWaiting)
	{
		// 効果音を再生
		if (!m_isSEPlayed)
		{
			SoundManager::GetInstance().Play(L"GAMEOVER");
			m_isSEPlayed = true;
		}
		// フォントの位置を更新
		if (m_fontPosX < FONT_X_MAX)
		{
			// 右に移動
			m_fontPosX += elapsedTime * FONT_SPEED;

			if (m_fontPosX > FONT_X_MAX)
			{
				m_fontPosX = FONT_X_MAX;
			}
		}
		m_waitTimer += elapsedTime;
		// 一定時間経過したらリザルトシーンに遷移
		if (m_waitTimer >= WAIT_TIME)
		{
			ChangeScene<ResultScene>();
			return;
		}
	}
	// クリア判定処理
	if (m_player->IsGoal() && !m_isGoalWaiting)
	{
		SoundManager::GetInstance().Stop(L"GAMEPLAYBGM");

		// タイマーを止める
		m_isTimerActive = false;
		// 判定
		ResultScene::SetGlobalResult(ResultScene::ResultType::CLEAR);
		// どのステージか
		ResultScene::SetGlobalStage(ResultScene::GetGlobalStage());
		// クリアタイムをセット
		ResultScene::SetGlobalClearTime(m_timer);

		// ゴール後の待機状態を開始
		m_isGoalWaiting = true;
		m_waitTimer = 0.0f;
		return;
	}
	// ゴール後の処理
	if (m_isGoalWaiting)
	{
		// 効果音を再生
		if (!m_isSEPlayed)
		{
			SoundManager::GetInstance().Play(L"CLEAR");
			m_isSEPlayed = true;
		}
		// フォントの位置を更新
		if (m_fontPosX < FONT_X_MAX)
		{
			// 右に移動
			m_fontPosX += elapsedTime * FONT_SPEED;

			if (m_fontPosX > FONT_X_MAX)
			{
				m_fontPosX = FONT_X_MAX;
			}
		}
		m_waitTimer += elapsedTime;
		// 一定時間経過したらリザルトシーンに遷移
		if (m_waitTimer >= WAIT_TIME)
		{
			ChangeScene<ResultScene>();
			return;
		}
	}

	// カメラの更新
	if (!m_isGoalWaiting && !m_isGameOverWaiting && !m_isTeleporting)
	{
		m_gameCamera->Update(m_player->GetPosition(), m_stage.get());
	}	
	m_player->SetCameraHorizontalAngle(m_gameCamera->GetYAngle());
	auto eye = m_gameCamera->GetEyePosition();
	auto target = m_gameCamera->GetTargetPosition();
	DirectX::SimpleMath::Vector3 dir = target - eye;
	m_stage->UpdateBillboard(target, eye);

	// UI更新
	m_healthUI->Update();
	m_staminaUI->Update();
	m_swordUI->Update();
	m_shieldUI->Update();

	// デバッグモードのＯＮ／ＯＦＦ
	if (!m_isDebugMode && InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::L))
	{
		m_isDebugMode = true;
	}
	else if (m_isDebugMode && InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::L))
	{
		m_isDebugMode = false;
	}
}

/*
* @brief 描画処理
*
* @param[in]  なし
*
* @return なし
*/
void StageScene::Render()
{
	auto context = GetUserResources()->GetDeviceResources()->GetD3DDeviceContext();
	auto windowSize = GetUserResources()->GetDeviceResources()->GetOutputSize();
	m_view = m_gameCamera->GetCameraMatrix();

	// ステージの描画
	m_stage->Render(context, m_view, m_proj);

	// プレイヤー
	if (!m_isGameOverWaiting)
	{
		m_player->Render();
	}
	// ダッシュパーティクルの描画
	if (m_dashParticle && m_gameCamera)
	{
		// プレイヤーの位置に合わせてビルボード行列を更新
		m_dashParticle->Billboard(
			m_gameCamera->GetTargetPosition(),
			m_gameCamera->GetEyePosition(),
			DirectX::SimpleMath::Vector3::Up
		);

		// パーティクルを描画
		m_dashParticle->Render(m_view, m_proj);
	}
	// プレイヤーの影の描画
	if (!m_isTeleporting && !m_isGameOverWaiting)
	{
		m_player->RenderShadow();
	}
	// 敵
	for (auto& enemy : m_enemies)
	{
		if(!enemy->IsDead())
		{
			enemy->Render(m_view,m_proj);
		}
	}
	// 敵の影の描画
	for (auto& enemy : m_enemies)
	{
		if (enemy->IsDead()) continue;
		enemy->RenderShadow();
	}

	// 渦巻パーティクルの描画
	if (m_isTeleporting) m_swirlParticle->Render();

	// ダッシュパーティクルの描画
	m_dashParticle->Render(m_view, m_proj);

	// ビューとプロジェクション行列を設定
	m_basicEffect->SetView(m_view);
	m_basicEffect->SetProjection(m_proj);
	m_basicEffect->Apply(context);

	context->IASetInputLayout(m_inputLayout.Get());

	// 剣の範囲の円（剣の使用回数がATTACK_COUNT以上だったら）
	if (m_player->GetAttackCount() >= ATTACK_COUNT)
		m_player->SwordRangeCircle();

	// 画像の描画（スプライトバッチを使用）
	m_spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, GetUserResources()->GetCommonStates()->NonPremultiplied());

	// 画面全体の矩形
	RECT fullscreenRect{};
	fullscreenRect.left = 0;
	fullscreenRect.top = 0;
	fullscreenRect.right = (LONG)windowSize.right;
	fullscreenRect.bottom = (LONG)windowSize.bottom;

	// ポーズメニューは画面を暗くする
	if (m_isPause)
	{
		// 黒色で半透明
		DirectX::SimpleMath::Color darkColor(0.0f, 0.0f, 0.0f, 0.5f);
		// 描画
		m_spriteBatch->Draw(m_overlayTexture.Get(), fullscreenRect, darkColor);
	}
	// ポータルでワープ中
	if (m_isTeleporting)
	{
		DirectX::SimpleMath::Color darkPurpleColor(0.3f,0.0f,0.4f,m_teleportOverlayAlpha);

		m_spriteBatch->Draw(
			m_overlayTexture.Get(),
			fullscreenRect,
			darkPurpleColor);
	}
	// クリア時
	if (m_isGoalWaiting)
	{
		// 白色で半透明
		DirectX::SimpleMath::Color whiteColor(1.0f, 1.0f, 1.0f, 0.5f);
		// 描画
		m_spriteBatch->Draw(m_overlayTexture.Get(), fullscreenRect, whiteColor);
	}
	// ゲームオーバー時
	if (m_isGameOverWaiting)
	{
		// 黒色でほぼ不透明
		DirectX::SimpleMath::Color darkoOpaqueColor(0.0f, 0.0f, 0.0f, 0.2f);
		// 描画
		m_spriteBatch->Draw(m_overlayTexture.Get(), fullscreenRect, darkoOpaqueColor);
	}

	// タイムの画像を描画
	m_spriteBatch->Draw(m_timeSRV.Get(), ScreenManager::Pos(480.0f, 60.0f), nullptr,
		DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
		ScreenManager::Scale(DEFAULT_SRV_SCALE_X, DEFAULT_SRV_SCALE_Y));
	// タスクマネージャーの描画処理
	m_taskManager.Render();

	// 「Ｔキー：ポーズ」の画像を描画
	if (!m_isPause && !m_isGoalWaiting && !m_isGameOverWaiting )
	{
		m_spriteBatch->Draw(m_pauseKeySRV.Get(), ScreenManager::Pos(40.0f, 40.0f), nullptr,
			DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
			ScreenManager::Scale(DEFAULT_SRV_SCALE_X, DEFAULT_SRV_SCALE_Y));
	}
	m_spriteBatch->End();


	// UIの描画
	if (!m_isPause && !m_isTeleporting && !m_isGoalWaiting && !m_isGameOverWaiting)
	{
		m_healthUI->Render();
		m_staminaUI->Render();
		m_swordUI->Render();
		m_shieldUI->Render();
	}

	// ステージクリアのフォントの描画
	if (m_isGoalWaiting)
	{
		m_spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, GetUserResources()->GetCommonStates()->NonPremultiplied());

		m_spriteBatch->Draw(m_gameClearSRV.Get(), ScreenManager::Pos(m_fontPosX, 250.0f), nullptr,
			DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
			ScreenManager::Scale(DEFAULT_SRV_SCALE_X, DEFAULT_SRV_SCALE_Y));
		
		m_spriteBatch->End();
	}
	// ゲームオーバーのフォントの描画
	if (m_isGameOverWaiting)
	{
		m_spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, GetUserResources()->GetCommonStates()->NonPremultiplied());

		m_spriteBatch->Draw(m_gameOverSRV.Get(), ScreenManager::Pos(m_fontPosX, 250.0f), nullptr,
			DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
			ScreenManager::Scale(DEFAULT_SRV_SCALE_X, DEFAULT_SRV_SCALE_Y));
		
		m_spriteBatch->End();
	}
	// ポーズメニューの描画
	if (m_isPause)
	{
		m_spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, GetUserResources()->GetCommonStates()->NonPremultiplied());

		m_spriteBatch->Draw(m_selectKeySRV.Get(), ScreenManager::Pos(40.0f, 40.0f), nullptr,
			DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
			ScreenManager::Scale(DEFAULT_SRV_SCALE_X, DEFAULT_SRV_SCALE_Y));
		m_pauseMenu->Render();

		m_spriteBatch->End();
	}

	// デバッグモードでの描画
#ifdef _DEBUG
	m_basicEffect->Apply(context);
	context->IASetInputLayout(m_inputLayout.Get());

	if (m_isDebugMode) 
	{
		// コライダーの線の色
		auto lineColor = DirectX::Colors::Black;
		auto lineColorB = DirectX::Colors::Green;

		// プレイヤーの線
		m_player->ColliderLine();
		// 敵の線
		for (auto& enemy : m_enemies)
		{
			if (!enemy->IsDead())
			{
				enemy->ColliderLine();
			}
		}
		// 各オブジェクトの線
		m_stage->Debug();

		// デバッグフォント
		auto* debugFont = GetUserResources()->GetDebugFont();
		if (m_isDebugMode)
		{
			debugFont->AddString((L"AttackCount" + std::to_wstring(m_player->GetAttackCount())).c_str(), DirectX::SimpleMath::Vector2(500.0f, 0.0f));

			// デバッグフォントの描画
			debugFont->AddString(L"StageScene", DirectX::SimpleMath::Vector2(0.0f, debugFont->GetFontHeight()));
			debugFont->AddString((L"PlayerPosX: " + std::to_wstring(m_player->GetPosition().x)).c_str(), DirectX::SimpleMath::Vector2(0.0f, 190.0f));
			debugFont->AddString((L"PlayerPosY: " + std::to_wstring(m_player->GetPosition().y)).c_str(), DirectX::SimpleMath::Vector2(0.0f, 220.0f));
			debugFont->AddString((L"PlayerPosZ: " + std::to_wstring(m_player->GetPosition().z)).c_str(), DirectX::SimpleMath::Vector2(0.0f, 250.0f));
			debugFont->AddString((L"Stamina: " + std::to_wstring(m_player->GetStamina())).c_str(), DirectX::SimpleMath::Vector2(0.0f, 456.0f));
			debugFont->AddString((L"Lives: " + std::to_wstring(m_player->GetLives())).c_str(), DirectX::SimpleMath::Vector2(0.0f, 456.0f));
			debugFont->AddString((L"invincibilityTime: " + std::to_wstring(m_player->GetInvincibilityTime())).c_str(), DirectX::SimpleMath::Vector2(0.0f, 470.0f));
			debugFont->AddString((L"Attack: " + std::to_wstring(m_player->GetAttackCount())).c_str(), DirectX::SimpleMath::Vector2(0.0f, 530.0f));
			debugFont->AddString((L"Defense: " + std::to_wstring(m_player->GetDefenseCount())).c_str(), DirectX::SimpleMath::Vector2(0.0f, 570.0f));
		}
		debugFont->Render(GetUserResources()->GetCommonStates());
	}
#endif // _DEBUG
}

/*
* @brief 終了処理
*
* @param[in]  なし
*
* @return なし
*/
void StageScene::Finalize()
{
	OnDeviceLost();
}

/*
* @brief ゲームのリセット
*
* @param[in]  なし
*
* @return なし
*/
void StageScene::ResetGame()
{
	// タイマーのリセット
	m_timer = 0.0f;

	// テレポートタイマーのリセット
	m_isTeleporting = false;
	m_teleportTimer = 0.0f;

	// ステージオブジェクトのリセット
	m_stage->ResetObject();

	// BGMの再生
	SoundManager::GetInstance().PlayLoop(L"GAMEPLAYBGM", 1);

	// 床や足場の情報を渡す
	auto floor = m_stage->GetFloor();
	auto platform = m_stage->GetPlatform();
	m_player->SetFloorData(floor->GetPositions(), floor->GetScales());
	m_player->SetPlatformData(platform->GetPositions(), platform->GetScales());
	// プレイヤーの初期化
	m_player->Initialize(PLAYER_INITIAL_POSITION);
	// カメラをプレイヤークラスに渡す
	m_player->SetDebugCamera(m_gameCamera.get());

	// 敵
	for (size_t i = 0; i < m_enemies.size(); ++i)
	{
		// 床情報を渡す
		m_enemies[i]->SetFloorData(floor->GetPositions(), floor->GetScales());
		m_enemies[i]->SetPlatformData(platform->GetPositions(), platform->GetScales());

		// 初期化
		m_enemies[i]->Initialize(m_enemyStartPositions[i]);
	}

	// ポーズ解除
	m_isPause = false;
	m_pauseMenu->m_menuIndex = 0;
}

/*
* @brief デバイスに依存するリソースを作成する関数
*
* @param[in]  なし
*
* @return なし
*/
void StageScene::CreateDeviceDependentResources()
{
	auto deviceResources = GetUserResources()->GetDeviceResources();
	auto device = deviceResources->GetD3DDevice();
	auto context = deviceResources->GetD3DDeviceContext();
	auto states = GetUserResources()->GetCommonStates();

	std::unique_ptr<DirectX::EffectFactory> fx;
	fx = std::make_unique<DirectX::EffectFactory>(device);
	fx->SetDirectory(L"Resources/Models");

	// スプライトバッチの作成
	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(context);

	// ステージのインスタンス作成
	m_stage = std::make_unique<Stage>();
	m_stage->CreateDeviceDependentResources(device, context, deviceResources, GetUserResources()->GetCommonStates(), GetUserResources());

	// 画像の読み込み
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/number.dds", nullptr, m_numberSRV.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/time.dds", nullptr, m_timeSRV.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/pauseKey.dds", nullptr, m_pauseKeySRV.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/selectKey.dds", nullptr, m_selectKeySRV.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/StageClearFont.dds", nullptr, m_gameClearSRV.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/gameoverFont.dds", nullptr, m_gameOverSRV.ReleaseAndGetAddressOf()));

	// プレイヤー生成
	m_player = std::make_unique<Player>();
	m_player->SetDeviceResources(deviceResources);
	m_player->SetCommonStates(states);

	// 敵の生成
	m_enemies.clear();
	for (size_t i = 0; i < m_enemyStartPositions.size(); ++i)
	{
		auto enemy = std::make_unique<Enemy>();
		enemy->SetDeviceResources(deviceResources);
		enemy->SetCommonStates(states);
		m_enemies.push_back(std::move(enemy));
	}

	// ポータルの渦巻き
	m_swirlParticle = std::make_unique<SwirlParticle>();
	m_swirlParticle->Create(deviceResources);

	// ダッシュパーティクルの作成
	m_dashParticle = std::make_unique<PlayerDashParticle>();
	m_dashParticle->Create(deviceResources);
	m_player->SetDashParticle(m_dashParticle.get());

	// 各UIのインスタンス作成
	m_healthUI = std::make_unique<HealthUI>();
	m_staminaUI = std::make_unique<StaminaUI>();
	m_swordUI = std::make_unique<SwordUI>();
	m_shieldUI = std::make_unique<ShieldUI>();

	// 各UIの初期化処理
	m_healthUI->SetPlayer(m_player.get());
	m_healthUI->Initialize(deviceResources, BASE_SCREEN_WIDTH, BASE_SCREEN_HEIGHT);
	m_staminaUI->SetPlayer(m_player.get());
	m_staminaUI->Initialize(deviceResources, BASE_SCREEN_WIDTH, BASE_SCREEN_HEIGHT);
	m_swordUI->SetPlayer(m_player.get());
	m_swordUI->Initialize(deviceResources, BASE_SCREEN_WIDTH, BASE_SCREEN_HEIGHT);
	m_shieldUI->SetPlayer(m_player.get());
	m_shieldUI->Initialize(deviceResources, BASE_SCREEN_WIDTH, BASE_SCREEN_HEIGHT);

	// 画面を暗くすためのテクスチャを作成
	uint32_t pixel = 0xffffffff;
	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = &pixel;
	initData.SysMemPitch = sizeof(uint32_t);

	// 深度ステンシルの作成
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = 1;
	desc.Height = 1;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	DX::ThrowIfFailed(
		device->CreateTexture2D(&desc, &initData, texture.GetAddressOf())
	);

	DX::ThrowIfFailed(
		device->CreateShaderResourceView(texture.Get(), nullptr, m_overlayTexture.ReleaseAndGetAddressOf())
	);

	// メニューのインスタンス作成
	m_pauseMenu = std::make_unique<Menu>();

	// メニューの初期化処理
	D3D11_VIEWPORT windowInfo = deviceResources->GetScreenViewport();
	m_pauseMenu->Initialize(
		deviceResources,
		(int)windowInfo.Width,
		(int)windowInfo.Height);

	// SEのフラグをリセット
	m_isSEPlayed = false;

	// デバッグ用の線の描画
	if (!m_primitiveBatch)
	{
		m_primitiveBatch = std::make_unique<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>(context);
		m_basicEffect = std::make_unique<DirectX::BasicEffect>
			(GetUserResources()->GetDeviceResources()->GetD3DDevice());
		m_basicEffect->SetVertexColorEnabled(true);

		// インプットレイアウトの生成
		void const* shaderByteCode;
		size_t byteCodeLength;
		m_basicEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);
		device->CreateInputLayout(
			DirectX::VertexPositionColor::InputElements,
			DirectX::VertexPositionColor::InputElementCount,
			shaderByteCode, byteCodeLength,
			&m_inputLayout
		);
	}
}

/*
* @brief ウインドウサイズに依存するリソースを作成する関数
*
* @param[in]  なし
*
* @return なし
*/
void StageScene::CreateWindowSizeDependentResources()
{
	// 射影行列の作成
	RECT rect = GetUserResources()->GetDeviceResources()->GetOutputSize();
	m_proj = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(
		DirectX::XMConvertToRadians(FIELD_OF_VIEW_DEGREES),
		static_cast<float>(rect.right) / static_cast<float>(rect.bottom),
		NEAR_PLANE_DISTANCE, FAR_PLANE_DISTANCE
	);
}

/*
* @brief デバイスロストした時に呼び出される関数
*
* @param[in]  なし
*
* @return なし
*/
void StageScene::OnDeviceLost()
{
	m_primitiveBatch.reset();
	m_basicEffect.reset();
	m_gameCamera.reset();

	SoundManager::GetInstance().Update();
}