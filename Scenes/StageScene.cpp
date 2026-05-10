//--------------------------------------------------------------------------------------
// File: StageScene.cpp
//
// ステージシーンクラス
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "StageScene.h"
#include "Resources/json.hpp"

using json = nlohmann::json;

/*
* @brief コンストラクタ
*
* @param[in]  なし
*
* @return なし
*/
StageScene::StageScene()
	:
	m_cameraHorizontalAngle(0.0f),
	m_isPause(false),
	m_isTeleporting(false),
	m_timer(0.0f),
	m_teleportTimer(0.0f),
	m_number(),
	m_isDebugMode(false)
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
    // ステージに応じたJSONファイルの読み込み
    std::string filePath = "Resources/Stages/stage01.json"; 
    auto currentStage = ResultScene::GetGlobalStage();
    if (currentStage == ResultScene::ResultStage::SECOND) filePath = "Resources/Stages/stage02.json"; 
    if (currentStage == ResultScene::ResultStage::THIRD)  filePath = "Resources/Stages/stage03.json"; 

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

    // ロード時間に関する処理
    {
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

	// カメラの作成　
	RECT rect = GetUserResources()->GetDeviceResources()->GetOutputSize();
	m_gameCamera = std::make_unique<GameCamera>(rect.right, rect.bottom);
	// カメラの角度を取得
	m_gameCamera->SetAngle(DirectX::XMConvertToRadians(0.0f), DirectX::XMConvertToRadians(-10.0f));
	// カメラの位置を設定
	m_gameCamera->SetDistance(8.0f);

    // プレイヤーとテレポート時の処理を渡してロード
    m_stage->Load(filePath, m_player.get(), [this]() {
        m_isTeleporting = true;
        m_teleportTimer = 2.0f;
        SoundManager::GetInstance().Play(L"TELEPORT");
    });

    // プレイヤーの初期化
    m_player->Initialize({ 2.0f, 2.0f, -3.0f });

    // タスクマネージャーの初期化／数字の初期化
    m_number = m_taskManager.AddTask<Number>(&m_spriteBatch, m_numberSRV.GetAddressOf());
    m_number->SetNumberDecimal(m_timer);
    m_number->SetPosition(DirectX::SimpleMath::Vector2(570.0f, 60.0f));

    // タイマーの初期化
    m_timer = 0.0f;
    m_isTimerActive = true;

    // メニューの初期化
    m_pauseMenu->Add(L"Resources/Textures/resume.png"
        , ScreenManager::Pos(600, 200)
        , ScreenManager::Scale(0.8f, 0.8f)
        , ANCHOR::MIDDLE_CENTER);
    m_pauseMenu->Add(L"Resources/Textures/retry.png"
        , ScreenManager::Pos(600, 400)
        , ScreenManager::Scale(0.8f, 0.8f)
        , ANCHOR::MIDDLE_CENTER);
    m_pauseMenu->Add(L"Resources/Textures/select.png"
        , ScreenManager::Pos(600, 600)
        , ScreenManager::Scale(0.8f, 0.8f)
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
	// キー入力の取得
	auto kb = DirectX::Keyboard::Get().GetState();
	DirectX::Keyboard::State keystate = DirectX::Keyboard::Get().GetState();

	// マウス入力の取得
	auto mouse = DirectX::Mouse::Get().GetState();

	// キーのインプットマネージャーの更新
	InputManager::Get().Update();

	// サウンドマネージャーの更新
	SoundManager::GetInstance().Update();

	// プレイヤーの当たり判定を取得
	const auto& playerCollider = m_player->GetCollision();

	// ステージの更新
	m_stage->Update(elapsedTime, m_player.get(), m_enemies, m_isTeleporting);

	// タイマーの更新
	if (m_isTimerActive && !m_isPause) m_timer += elapsedTime;
	m_number->SetTimer(static_cast<int>(m_timer));
	m_number->Update(elapsedTime);
	// テレポートタイマーの更新
	if (m_isTeleporting)
	{
		m_teleportTimer -= elapsedTime;
		if (m_teleportTimer <= 0.0f)
		{
			m_isTeleporting = false;
		}
		// 渦巻パーティクルの更新
		m_swirlParticle->Update(elapsedTime);
	}

	// ポーズニューの処理
	if (InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::T)) m_isPause = true;
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
	if (!m_player->IsGoal() && !m_isTeleporting && !m_isPause)
	{
		m_player->Update(elapsedTime);
	}
	
	// 敵の更新
	for (auto& outerEnemy : m_enemies)
	{
		if (!outerEnemy->IsDead() && !m_isPause)
		{
			auto platform = m_stage->GetPlatform();
			outerEnemy->SetPlatformData(platform->GetPositions(), platform->GetScales());
			outerEnemy->Update(elapsedTime, m_player->GetPosition());
		}
		// 敵の当たり判定を取得
		AABB outerEnemyCollider = outerEnemy->GetCollision();
		AABB outerEnemyDamageCollider = outerEnemy->GetDamageCollision();
		// 敵が落下したら
		if (outerEnemy->GetPosition().y <= -15.0f)
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
			m_player->GetInvincibilityTime() <= 0.0f)
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
	if (m_player->GetLives() <= 0)
	{
		SoundManager::GetInstance().Stop(L"GAMEPLAYBGM");

		// 判定
		ResultScene::SetGlobalResult(ResultScene::ResultType::GAMEOVER);
		// どのステージか
		ResultScene::SetGlobalStage(ResultScene::GetGlobalStage());

		ChangeScene<ResultScene>();
		return;
	}
	// クリア条件
	if (m_player->IsGoal())
	{
		SoundManager::GetInstance().Stop(L"GAMEPLAYBGM");

		// タイマーを止める
		m_isTimerActive = false;
		// 判定
		ResultScene::SetGlobalResult(ResultScene::ResultType::CLEAR);
		// どのステージか
		ResultScene::SetGlobalStage(ResultScene::GetGlobalStage());		// クリアタイムをセット
		ResultScene::SetGlobalClearTime(m_timer);

		ChangeScene<ResultScene>();
	};

	// カメラの更新
	m_gameCamera->Update(m_player->GetPosition(), m_stage.get());
	m_player->SetCameraHorizontalAngle(m_gameCamera->GetYAngle());
	auto eye = m_gameCamera->GetEyePosition();
	auto target = m_gameCamera->GetTargetPosition();
	DirectX::SimpleMath::Vector3 dir = target - eye;
	m_stage->UpdateBillboard(target, eye);;

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
	m_player->Render();
	// プレイヤーの影の描画
	if (!m_isTeleporting) 
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

	// ビューとプロジェクション行列を設定
	m_basicEffect->SetView(m_view);
	m_basicEffect->SetProjection(m_proj);
	m_basicEffect->Apply(context);

	context->IASetInputLayout(m_inputLayout.Get());

	// 剣の範囲の円（剣の使用回数が1以上だったら）
	if (m_player->GetAttackCount() >= 1)
		m_player->SwordRangeCircle();

	// 画像の描画（スプライトバッチを使用）
	m_spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, GetUserResources()->GetCommonStates()->NonPremultiplied());

	m_spriteBatch->Draw(m_timeSRV.Get(), ScreenManager::Pos(480.0f, 60.0f), nullptr,
		DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
		ScreenManager::Scale(1.0f, 1.0f));
	if (!m_isPause)
	{
		m_spriteBatch->Draw(m_pauseKeySRV.Get(), ScreenManager::Pos(40.0f, 40.0f), nullptr,
			DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
			ScreenManager::Scale(1.0f, 1.0f));
	}

	// タスクマネージャーの描画処理
	m_taskManager.Render();

	// ポーズメニュは画面を暗くする
	if (m_isPause)
	{
		RECT fullscreenRect{};
		fullscreenRect.left = 0;
		fullscreenRect.top = 0;
		fullscreenRect.right = (LONG)windowSize.right;
		fullscreenRect.bottom = (LONG)windowSize.bottom;

		// 黒色で半透明
		DirectX::SimpleMath::Color darkColor(0.0f, 0.0f, 0.0f, 0.5f);

		// 描画
		m_spriteBatch->Draw(m_overlayTexture.Get(), fullscreenRect, darkColor);
	}
	m_spriteBatch->End();

	// ポーズメニューの描画
	if (m_isPause)
	{
		m_spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, GetUserResources()->GetCommonStates()->NonPremultiplied());
		m_spriteBatch->Draw(m_selectKeySRV.Get(), ScreenManager::Pos(40.0f, 40.0f), nullptr,
			DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
			ScreenManager::Scale(1.0f, 1.0f));
		m_pauseMenu->Render();
		m_spriteBatch->End();
	}

	// UIの描画
	if (!m_isPause)
	{
		m_healthUI->Render();
		m_staminaUI->Render();
		m_swordUI->Render();
		m_shieldUI->Render();
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

	// 床情報を渡す
	auto floor = m_stage->GetFloor();
	auto platform = m_stage->GetPlatform();
	m_player->SetFloorData(floor->GetPositions(), floor->GetScales());
	m_player->SetPlatformData(platform->GetPositions(), platform->GetScales());
	// プレイヤーの初期化
	m_player->Initialize({ 2.0f, 2.0f, -3.0f });
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

	// 各UIのインスタンス作成
	m_healthUI = std::make_unique<HealthUI>();
	m_staminaUI = std::make_unique<StaminaUI>();
	m_swordUI = std::make_unique<SwordUI>();
	m_shieldUI = std::make_unique<ShieldUI>();

	// 各UIの初期化処理
	m_healthUI->SetPlayer(m_player.get());
	m_healthUI->Initialize(deviceResources, 1280, 720);
	m_staminaUI->SetPlayer(m_player.get());
	m_staminaUI->Initialize(deviceResources, 1280, 720);
	m_swordUI->SetPlayer(m_player.get());
	m_swordUI->Initialize(deviceResources, 1280, 720);
	m_shieldUI->SetPlayer(m_player.get());
	m_shieldUI->Initialize(deviceResources, 1280, 720);

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
			&m_inputLayout);
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
	m_proj = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView
	(
		DirectX::XMConvertToRadians(45.0f),
		static_cast<float>(rect.right) / static_cast<float>(rect.bottom),
		0.1f, 100.0f
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