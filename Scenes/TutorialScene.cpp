//--------------------------------------------------------------------------------------
// File: TutorialScene.cpp
//
// チュートリアルステージシーンクラス
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "TutorialScene.h"
#include "SKLib/ReadData.h"
#include "Resources/json.hpp"
#include <fstream>

using json = nlohmann::json;

/*
* @brief コンストラクタ
*
* @param[in] なし
* 
* @return なし
*/
TutorialScene::TutorialScene()
	:
	m_cameraHorizontalAngle(0.0f),
	m_cameraVerticalAngle(0.0f),
	m_isFollowCamera(true),
	m_isPause(false),
	m_isTutorialActive(false),
	m_isSwitchOn_Key(false),
	m_isSwitchOn_Portal(false),
	m_isSwitchOn_Item(false),
	m_isDebugMode(false),
	m_showExplanationFirst(true),
	m_showExplanationSecond(false),
	m_liftTutorialShown(false),
	m_switchShown(false),
	m_itemShown(false),
	m_timer(0.0f),
	m_number{},
	m_isTeleporting(false),
	m_teleportTimer(0.0f),
	m_swordRespawnTimer(0.0f)
{
}

/*
* @brief デストラクタ
*
* @param[in] なし
* 
* @return なし
*/
TutorialScene::~TutorialScene()
{
}

/*
* @brief 初期化処理
*
* @param[in] なし
* 
* @return なし
*/
void TutorialScene::Initialize()
{
	m_prevKeyboardState = {};

	// JSONステージデータをロード
	std::ifstream file("Resources/Stages/tutorial.json");
	if (!file.is_open())
	{
		MessageBox(NULL, L"ステージファイルが見つかりません。", L"エラー", MB_OK | MB_ICONHAND);
		abort();
		return;
	}
	json data;
	file >> data;

	// 敵の読み込み
	m_enemyStartPositions.clear();
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

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();

	// ロード時間に関する処理
	{
		std::this_thread::sleep_for(std::chrono::seconds(3));
	}

	// チュートリアルマネージャーの初期化
	m_tutorialManager = std::make_unique<TutorialManager>();
	m_player->SetTutorialManager(m_tutorialManager.get());

	// カメラの作成　
	RECT rect = GetUserResources()->GetDeviceResources()->GetOutputSize();
	m_gameCamera = std::make_unique<GameCamera>(rect.right, rect.bottom);

	// カメラの角度を取得
	m_gameCamera->SetAngle(DirectX::XMConvertToRadians(0.0f), DirectX::XMConvertToRadians(-10.0f));

	// カメラの位置を設定
	m_gameCamera->SetDistance(8.0f);

	// 床の読み込み
	{
		std::vector<DirectX::SimpleMath::Vector3> pos, scale;
		for (auto& f : data["floors"])
		{
			pos.push_back({ f["position"][0].get<float>(), f["position"][1].get<float>(), f["position"][2].get<float>() });
			scale.push_back({ f["scale"][0].get<float>(), f["scale"][1].get<float>(), f["scale"][2].get<float>() });
		}
		m_floor->LoadData(pos, scale);
	}
	// 壁の読み込み
	{
		std::vector<DirectX::SimpleMath::Vector3> pos, scale;
		for (auto& w : data["walls"])
		{
			pos.push_back({ w["position"][0].get<float>(), w["position"][1].get<float>(), w["position"][2].get<float>() });
			scale.push_back({ w["scale"][0].get<float>(), w["scale"][1].get<float>(), w["scale"][2].get<float>() });
		}
		m_wall->LoadData(pos, scale);
	}
	// スイッチで動く足場の状態を初期化
	{
		std::vector<DirectX::SimpleMath::Vector3> pos, scale;
		for (auto& pf : data["platforms"]) {
			pos.push_back({ pf["position"][0].get<float>(), pf["position"][1].get<float>(), pf["position"][2].get<float>() });
			scale.push_back({ pf["scale"][0].get<float>(), pf["scale"][1].get<float>(), pf["scale"][2].get<float>() });
		}
		m_platform->LoadPlatformData(pos, scale); //

		m_isSwitchOn_PF.clear();
		m_isSwitchOn_PF.resize(pos.size(), false);
	}
	// 扉の読み込み
	if (data.contains("gate"))
	{
		auto& g = data["gate"];
		m_gate->SetPosition(g["position"][0], g["position"][1], g["position"][2]);
	}
	m_gate->Initialize();

	// 仕掛けブロックの初期化
	m_gimmickBlocks.clear();
	m_itemGimmickBlockIndices.clear();
	if (data.contains("gimmick_blocks")) {
		for (auto& gb : data["gimmick_blocks"]) {
			auto block = std::make_shared<GimmickBlock>();

			// 座標の取得
			DirectX::SimpleMath::Vector3 pos = 
			{
				gb["position"][0].get<float>(),
				gb["position"][1].get<float>(),
				gb["position"][2].get<float>()
			};

			// タイプの判定とモデルの割り当て
			std::string typeStr = gb["type"].get<std::string>();
			std::shared_ptr<DirectX::Model> targetModel = m_platformBlockModel;
			BlockType targetType = BlockType::PLATFORM;

			if (typeStr == "KEY") 
			{
				targetModel = m_keyBlockModel;
				targetType = BlockType::KEY;
			}
			else if (typeStr == "PORTAL") 
			{
				targetModel = m_portalBlockModel;
				targetType = BlockType::PORTAL;
			}
			else if (typeStr == "ITEM") 
			{
				targetModel = m_itemBlockModel;
				targetType = BlockType::ITEM;
			}

			// 初期化
			block->Initialize(targetModel, pos, { 0.7f, 0.7f, 0.7f }, targetType);
			m_gimmickBlocks.push_back(block);

			// ITEMタイプならインデックスを保存
			if (targetType == BlockType::ITEM) 
			{
				m_itemGimmickBlockIndices.push_back(m_gimmickBlocks.size() - 1);
			}
		}
	}
	// ポータルの読み込み
	m_portal->SetModel(m_portalModel.get());
	if (data.contains("portals"))
	{
		for (auto& p : data["portals"])
		{
			// 座標情報の取得
			DirectX::SimpleMath::Vector3 pos = { p["position"][0], p["position"][1], p["position"][2] };
			DirectX::SimpleMath::Vector3 scale = { p["scale"][0], p["scale"][1], p["scale"][2] };

			// テレポート先の取得
			float dx = p["dest"][0].get<float>();
			float dy = p["dest"][1].get<float>();
			float dz = p["dest"][2].get<float>();

			// タイプの判定
			std::string tTypeStr = p["target_type"].get<std::string>();
			PortalTargetType pType = PortalTargetType::MOVEON;
			if (tTypeStr == "MOVEON") pType = PortalTargetType::MOVEON;
			else if (tTypeStr == "RETURN") pType = PortalTargetType::RETURN;

			// スイッチインデックスの取得
			int sIdx = p.value("switch_index", -1);

			// インデックス範囲チェック
			m_portal->AddPortal({
						pos, scale, pType,
						[this, dx, dy, dz]() {
							m_isTeleporting = true;
							m_player->SetPosition(dx, dy, dz);
							SoundManager::GetInstance().Play(L"TELEPORT");
						}
				}, sIdx);
		}
	}
	// ポータル用のスイッチフラグをリサイズ
	int requiredSize = m_portal->GetPortalSetCount();
	m_isSwitchOn_Portal.resize(requiredSize, false);

	// アイテム出現位置の読み込み
	m_itemSpawnPositions.clear();
	if (data.contains("items"))
	{
		for (auto& it : data["items"])
		{
			m_itemSpawnPositions.push_back({ it["position"][0], it["position"][1], it["position"][2] });
		}
	}
	m_item->Initialize();
	m_isSwitchOn_Item.assign(m_itemSpawnPositions.size(), false);
	m_itemTypes.assign(m_itemSpawnPositions.size(), ItemType::NONE);

	// ゴールの初期化
	if (data.contains("goal"))
	{
		auto& g = data["goal"];
		m_goal->SetPosition(g["position"][0], g["position"][1], g["position"][2]);
		m_goal->SetRotation(g["rotation"][0], g["rotation"][1], g["rotation"][2]);
	}
	m_goal->Initialize();

	// スイッチの読み込み／各スイッチの追加
	m_switch->SetModel(m_switchModel.get());
	if (data.contains("switches")) {
		for (auto& sw : data["switches"])
		{
			DirectX::SimpleMath::Vector3 pos = { sw["position"][0], sw["position"][1], sw["position"][2] };
			std::string typeStr = sw["type"].get<std::string>();
			int idx = sw["index"].get<int>();

			SwitchTargetType stype = SwitchTargetType::SW_PLATFORM;
			std::function<void()> action;

			if (typeStr == "SW_PLATFORM")
			{
				stype = SwitchTargetType::SW_PLATFORM;
				action = [this, idx]() { m_isSwitchOn_PF[idx] = true; };
			}
			else if (typeStr == "SW_PORTAL")
			{
				stype = SwitchTargetType::SW_PORTAL;
				action = [this, idx]() { m_isSwitchOn_Portal[idx] = true; };
			}
			else if (typeStr == "SW_KEY")
			{
				stype = SwitchTargetType::SW_KEY;
				action = [this]() { m_isSwitchOn_Key = true; };
			}
			else if (typeStr == "SW_ITEM")
			{
				stype = SwitchTargetType::SW_ITEM;
				action = [this, idx]() {
					m_isSwitchOn_Item[idx] = true;
					if (idx < m_itemGimmickBlockIndices.size())
					{
						size_t gIdx = m_itemGimmickBlockIndices[idx];
						if (gIdx < m_gimmickBlocks.size()) m_gimmickBlocks[gIdx]->SetIsVisible(false);
					}
					};
			}

			m_switch->AddSwitch({ pos, {1.0f, 1.0f, 1.0f}, stype, action });
		}
	}
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

	// 各オブジェクトの初期化
	ResetObjects();
}

/*
* @brief 更新処理
*
* @param[in]  elapsedTime 前フレームからの経過時間
* 
* @return なし
*/
void TutorialScene::Update(float elapsedTime)
{
	// 毎フレーム時に更新
	m_isSwitchOn_Key = false;
	for (int i = 0; i < m_isSwitchOn_PF.size(); ++i)
	{
		m_isSwitchOn_PF[i] = false;
	}
	for (int i = 0; i < m_isSwitchOn_Portal.size(); ++i)
	{
		m_isSwitchOn_Portal[i] = false;
	}
	// カメラ追尾を常にする
	m_isFollowCamera = true;

	UNREFERENCED_PARAMETER(elapsedTime);

	// キー入力の取得
	auto kb = DirectX::Keyboard::Get().GetState();
	// マウス入力の取得
	auto mouse = DirectX::Mouse::Get().GetState();

	// キーのインプットマネージャーの更新
	InputManager::Get().Update();

	// サウンドマネージャーの更新
	SoundManager::GetInstance().Update();

	// プレイヤークラスから数値などを取得
	const auto& playerPos = m_player->GetPosition();
	const auto& playerVellocity = m_player->GetVelocity();
	const auto& playerCollider = m_player->GetCollision();
	bool isJumping = m_player->IsJumping();

	// プレイヤーが床または足場と接触しているか
	bool isOnGround = m_player->IsOnFloor();

	// キーで操作説明の切替
	if (InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::C) && m_showExplanationFirst)
	{
		m_showExplanationFirst = false;
		m_showExplanationSecond = true;
	}
	if (InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Z) && m_showExplanationSecond)
	{
		m_showExplanationFirst = true;
		m_showExplanationSecond = false;
	}

	// タイマーを動かす
	if (m_isTimerActive && !m_isPause && !m_tutorialManager->IsPlayerLocked())
	{
		m_timer += elapsedTime;
	}
	m_number->SetTimer(static_cast<int>(m_timer));
	m_number->Update(elapsedTime);

	// 仕掛けブロックの更新処理
	if (!m_isPause && !m_tutorialManager->IsPlayerLocked() && !m_isTeleporting)
	{
		float blockFollowSpeed = (playerVellocity / elapsedTime) * 5;

		for (auto& block : m_gimmickBlocks)
		{
			if (!block) continue;

			// プレイヤーとブロックの衝突判定
			if (block->CheckCollision(m_player->GetCollision()))
			{
				// 右クリックが押された瞬間のみトグル処理
				bool currRightPressed = InputManager::Get().IsMousePressedRight();
				static bool prevRightPressed = false;

				// まだチュートリアルを表示していない場合
				if (!m_liftTutorialShown)
				{
					m_tutorialManager->SetState(TutorialManager::TutorialState::LIFTANDDROP);
					m_liftTutorialShown = true;
				}

				if (currRightPressed && !prevRightPressed)
				{
					// ブロックを持ち上げる処理
					if (!block->IsFollowing() && !m_followingBlock && !m_isTeleporting)
					{
						SoundManager::GetInstance().Play(L"LIFT");
						block->StartFollowing(m_player->GetPosition(), blockFollowSpeed);
						block->SetFollowOffset({ 0.0f, 1.0f, 0.0f });
						m_followingBlock = block;
					}
					// ブロックを置く処理
					else if (block->IsFollowing() && !isJumping && isOnGround && !m_isTeleporting)
					{
						SoundManager::GetInstance().Play(L"PUT");
						block->StopFollowing(playerPos);
						m_followingBlock = nullptr;
					}
				}
				// 前フレームの状態を更新
				prevRightPressed = currRightPressed;
			}

			// 追尾中であれば毎フレーム目標位置を更新
			if (block->IsFollowing())
			{
				// チュートリアルを表示していない場合
				if (!m_switchShown)
				{
					// スイッチの情報を取得
					const auto& switches = m_switch->GetSwitches();

					for (const auto& sw : switches)
					{
						// スイッチの当たり判定を作成
						DirectX::SimpleMath::Vector3 swHalf = sw.scale * 0.5f;
						AABB switchCollider(sw.position - swHalf, sw.position + swHalf);

						// ブロックを持っている状態で触れたら
						if (playerCollider.CheckAABBCollision(playerCollider, switchCollider))
						{
							m_tutorialManager->SetState(TutorialManager::TutorialState::SWITCH);
							m_switchShown = true;
							break;
						}
					}
				}
				block->UpdateTargetPosition(playerPos);
			}
			block->Update(elapsedTime);
		}
	}

	// スイッチの処理
	m_switch->Update(m_gimmickBlocks);

	// ポータルの処理
	m_portal->Update(m_player->GetCollision(), elapsedTime, m_isSwitchOn_Portal);

	// アイテムの処理
	m_item->Update(m_player->GetCollision(), m_player.get());
	for (size_t i = 0; i < m_isSwitchOn_Item.size(); ++i)
	{
		if (m_isSwitchOn_Item[i] && !m_isSwordSystemActive)
		{
			m_isSwordSystemActive = true;
			m_itemTypes[i] = ItemType::SWORD;

			// 最初の剣を生成
			m_item->AddItem(m_itemSpawnPositions[i], m_itemTypes[i]);
		}
	}
	//　アイテムがアクティブだった場合の処理
	if (m_isSwordSystemActive)
	{
		// アイテムの処理
		m_item->Update(m_player->GetCollision(), m_player.get());

		const auto& currentItems = m_item->GetItems();
		bool isSwordActive = false;

		// フィールド上にアクティブなアイテムが存在するかチェック
		for (const auto& it : currentItems)
		{
			if (it.itemType == ItemType::SWORD && it.isActive)
			{
				isSwordActive = true;
				break;
			}
		}
		// アイテムを拾った瞬間を検知
		if (!isSwordActive && !m_isSwordCollected)
		{
			m_isSwordCollected = true;
			m_swordRespawnTimer = 0.0f;
		}
		// アイテムののリスポーンタイマー処理
		if (m_isSwordCollected)
		{
			m_swordRespawnTimer += elapsedTime;
			if (m_swordRespawnTimer >= 3.0f)
			{
				// 剣を再出現させる
				if (!m_itemSpawnPositions.empty())
				{
					// 最初の生成ポイントに固定して再出現
					m_item->AddItem(m_itemSpawnPositions[0], ItemType::SWORD);
				}
				m_isSwordCollected = false;
			}
		}
		// アイテムのチュートリアル表示
		if (!m_itemShown)
		{
			for (const auto& item : currentItems)
			{
				if (item.itemType != ItemType::SWORD || !item.isActive) continue;

				// 当たり判定の作成
				DirectX::SimpleMath::Vector3 itemHalf = item.scale * 0.5f;
				AABB itemCollider(item.position - itemHalf, item.position + itemHalf);

				if (playerCollider.CheckAABBCollision(playerCollider, itemCollider))
				{
					m_tutorialManager->SetState(TutorialManager::TutorialState::ITEM);
					m_itemShown = true;
					break;
				}
			}
		}
	}

	// ゴールの処理
	m_goal->Update(m_player->GetCollision(), m_player.get());

	// プレイヤーの着地状態をリセット
	m_player->ResetFloorHit();

	// 壁・床・足場の更新処理
	m_floor->Update(m_player.get(), m_enemies);
	m_wall->Update(m_player.get(), m_enemies);
	m_platform->Update(m_isSwitchOn_PF, m_player.get(), m_enemies);

	// スイッチで動く足場の処理
	const auto& pfPositions = m_platform->GetPositions();
	for (size_t i = 0; i < pfPositions.size(); ++i)
	{
		if (m_isSwitchOn_PF[i]) 
			// スイッチがONの時に有効化,OFFの時に無効化
			m_platform->Update(m_isSwitchOn_PF, m_player.get(), m_enemies);
	}

	// 扉の処理
	m_gate->Update(m_player.get(), m_enemies);
	if (m_isSwitchOn_Key)
	{
		m_gate->Open();
	}
	else
	{
		m_gate->Close();
	}

	// 各パーティクルの更新処理
	// ポータル:スイッチの上に出るパーティクル
	std::vector<PortalEmitterInfo> portalEmitterInfos;
	for (const auto& portal : m_portal->GetPortals())
	{
		PortalEmitterInfo info;
		info.position = portal.position;
		info.scale = portal.scale;

		portalEmitterInfos.push_back(info);
	}
	m_portalParticle->SetEmitters(portalEmitterInfos);
	m_portalParticle->Update(elapsedTime);

	// ポータル：ワープしたときにでるパーティクル
	if (!m_isTeleporting)
	{
		m_teleportTimer = 2.0f;
	}
	else
	{
		m_teleportTimer -= elapsedTime;
		if (m_teleportTimer <= 0.0f)
			m_isTeleporting = false;
	}
	m_swirlParticle->Update(elapsedTime);

	// スイッチ
	std::vector<SwitchEmitterInfo> emitterInfos;
	for (const auto& sw : m_switch->GetSwitches())
	{
		SwitchEmitterInfo info;
		info.position = sw.position;
		info.scale = sw.scale;
		info.type = sw.switchType;

		emitterInfos.push_back(info);
	}
	m_switchParticle->SetEmitters(emitterInfos);
	m_switchParticle->Update(elapsedTime);

	// タスクマネージャーの更新処理
	m_taskManager.Update(elapsedTime);

	// UIの更新処理
	m_healthUI->Update();
	m_staminaUI->Update();
	m_swordUI->Update();
	m_shieldUI->Update();

	// ポーズ画面の処理
	if (!m_isTutorialActive && InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::T))
	{
		m_isPause = true;
	}
	if (m_isPause)
	{
		if (InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Down)
			|| InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::S))
		{
			SoundManager::GetInstance().Play(L"SELECT");
			if (m_pauseMenu->m_menuIndex > 2) m_pauseMenu->m_menuIndex = 0;
		}
		if (InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Up)
		    ||InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::W))
		{
			SoundManager::GetInstance().Play(L"SELECT");
			if (m_pauseMenu->m_menuIndex < 0) m_pauseMenu->m_menuIndex = 2;
		}

		// シーン切り替え
		if (m_pauseMenu->m_menuIndex == 0 && InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Space))
		{
			SoundManager::GetInstance().Play(L"DECISION");
			m_isPause = false;
			return;
		}
		if (m_pauseMenu->m_menuIndex == 1 && InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Space))
		{
			SoundManager::GetInstance().Play(L"DECISION");
			ResetGame();
		}
		if (m_pauseMenu->m_menuIndex == 2 && InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Space))
		{
			SoundManager::GetInstance().Play(L"DECISION");
			SoundManager::GetInstance().Stop(L"TUTORIALBGM");
			ChangeScene<SelectScene>();
		}

		m_pauseMenu->Update();
		return;
	}
	// 残機が０になったら
	if (m_player->GetLives() == 0)
	{
		m_player->SetLives(3); //　復活させる
	}
	// クリア条件
	if (m_player->IsGoal() == true)
	{
		SoundManager::GetInstance().Stop(L"TUTORIALBGM");

		// タイマーを止める
		m_isTimerActive = false; 
		// 判定
		ResultScene::SetGlobalResult(ResultScene::ResultType::CLEAR);
		// どのステージか
		ResultScene::SetGlobalStage(ResultScene::ResultStage::TUTORIAL);	
		// クリアタイムをセット
		ResultScene::SetGlobalClearTime(m_timer);

		ChangeScene<ResultScene>();
	}

	// コライダーの線を描画するかどうか
	if (InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::L) && !m_isDebugMode)
	{
		m_isDebugMode = true;
	}
	else if (InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::L) && m_isDebugMode)
	{
		m_isDebugMode = false;
	}

	// カメラの更新
	m_gameCamera->Update(m_player->GetPosition(), this);
	m_player->SetCameraHorizontalAngle(m_gameCamera->GetYAngle());
	auto eye = m_gameCamera->GetEyePosition();
	auto target = m_gameCamera->GetTargetPosition();
	m_portalParticle->Billboard(target, eye, { 0,1,0 });
	m_switchParticle->Billboard(target, eye, { 0,1,0 });

	// 敵の更新
	for (auto& outerEnemy : m_enemies)
	{
		if (!outerEnemy->IsDead() && !m_isPause)
		{
			outerEnemy->Update(elapsedTime, m_player->GetPosition());
		}

		// 敵の当たり判定を取得
		AABB outerEnemyCollider = outerEnemy->GetCollision();
		AABB outerEnemyDamageCollider = outerEnemy->GetDamageCollision();

		// 敵が落下したら
		if (outerEnemy->GetPosition().y <= -10.0f)
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

	if (!m_tutorialManager->IsPlayerLocked()) // プレイヤーがロックされていないとき
	{
		m_isTutorialActive = false;

		if (InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Space))
		{
			m_tutorialManager->CompleteStep();
			OutputDebugString(L"CompleteStep() called\n");
		}
	}
	else
	{
		m_isTutorialActive = true;
		// チュートリアルの処理
		switch (m_tutorialManager->GetState())
		{
		case TutorialManager::TutorialState::MOVEMENT:
			if (InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Space))
			{
				m_tutorialManager->NextStep();
			}
			break;
		case TutorialManager::TutorialState::MOUSE:
			if (InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Space))
			{
				m_tutorialManager->NextStep();
			}
			break;
		case TutorialManager::TutorialState::LIFTANDDROP:
			if (InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Space))
			{
				m_tutorialManager->CompleteStep();
			}
			break;
		case TutorialManager::TutorialState::SWITCH:
			if (InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Space))
			{
				m_tutorialManager->CompleteStep();
			}
			break;
		case TutorialManager::TutorialState::ITEM:
			if (InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Space))
			{
				m_tutorialManager->NextStep();
			}
			break;
		case TutorialManager::TutorialState::ATTACK:
			if (InputManager::Get().IsKeyPressed(DirectX::Keyboard::Keys::Space))
			{
				m_tutorialManager->NextStep();
			}
			break;
		default:
			break;
		}
		return;
	}

	// プレイヤーの更新
	if (!m_player->IsGoal() && m_portal->IsTeleporting() == false && !m_isPause)
	{
		m_player->Update(elapsedTime);
	}

	// 現在の状態を次回の比較用に保存
	m_prevKeyboardState = kb;
}

/*
* @brief 描画処理
*
* @param[in]  なし
*/
void TutorialScene::Render()
{
	auto context = GetUserResources()->GetDeviceResources()->GetD3DDeviceContext();
	auto states = GetUserResources()->GetCommonStates();
	auto windowSize = GetUserResources()->GetDeviceResources()->GetOutputSize();
	m_view = m_gameCamera->GetCameraMatrix();

	// デフォルトのマトリックス
	DirectX::SimpleMath::Matrix rot = DirectX::SimpleMath::Matrix::CreateRotationY(0.0f);	

	////////////////////スケールと平行移動を適用 ＆ オブジェクトの描画 ////////////////////▼
	// プレイヤー
	m_player->Render();
	// 敵
	for (auto& enemy : m_enemies)
	{
		if (!enemy->IsDead())
		{
			enemy->Render(m_view, m_proj);
		}
	}
	// 扉
	m_gate->Render(m_view);
	// スイッチ
	m_switch->Render(context, m_view, m_proj, states);
	// ポータル
	m_portal->Render(context, m_view, m_proj, states, m_isSwitchOn_Portal);
	// アイテム
	m_item->Render(m_view, m_proj);
	// 床
	m_floor->Render(context, m_view, m_proj, states);
	// 壁
	m_wall->Render(context, m_view, m_proj, states);
	// 足場
	m_platform->Render(m_view, m_proj);
	// ゴール
	m_goal->Render(m_view);
	// 仕掛けブロック
	for (const auto& block : m_gimmickBlocks)
	{
		// ブロックが見つからなかったら何もしない
		if (!block || !block->GetIsVisible()) continue;

		// ワールド行列を取得
		DirectX::SimpleMath::Matrix world = block->GetWorldMatrix();

		// 種類ごとにモデルを切り替えて描画
		switch (block->GetType())
		{
		case BlockType::PLATFORM: 
			m_platformBlockModel->Draw(context, *states, world, m_view, m_proj);
			break;

		case BlockType::KEY: 
			m_keyBlockModel->Draw(context, *states, world, m_view, m_proj);
			break;

		case BlockType::PORTAL:
			m_portalBlockModel->Draw(context, *states, world, m_view, m_proj);
			break;

		case BlockType::ITEM:
			m_itemBlockModel->Draw(context, *states, world, m_view, m_proj);
			break;

		default:
			break;
		}
	}
	// プレイヤーの影の描画
	m_player->RenderShadow();
	//　各パーティクル
	{
		// ポータル
		bool portalEnabled = false;
		for (bool portalState : m_isSwitchOn_Portal)
		{
			if (portalState)
			{
				portalEnabled = true;
				break;
			}
		}
		if (portalEnabled)
		{
			m_portalParticle->Render(m_view, m_proj);
		}
		// スイッチ
		m_switchParticle->Render(m_view, m_proj);
	}
	///////////////////////////////////////////////////////////////////////////////////////▲

	// ビューとプロジェクション行列を設定
	m_basicEffect->SetView(m_view);
	m_basicEffect->SetProjection(m_proj);
	m_basicEffect->Apply(context);

	context->IASetInputLayout(m_inputLayout.Get());

	// 剣の範囲の円（剣の使用回数が1以上だったら）
	if (m_player->GetAttackCount() >= 1)
		m_player->SwordRangeCircle();

	// デバッグモード（コライダーの線）
#ifdef _DEBUG
	if (m_isDebugMode)
	{
		// 線の色を指定
		auto lineColorA = DirectX::Colors::Red;
		auto lineColorB = DirectX::Colors::Black;

		// プレイヤーの線
		m_player->ColliderLine();					
		for (auto& enemy : m_enemies)
		{
			if (!enemy->IsDead())
			{
				// 敵の線
				enemy->ColliderLine();				
			}
		}
		// スイッチの線
		m_switch->ColliderLine();
		// ポータルの線
		m_portal->ColliderLine(m_isSwitchOn_Portal);
		// 扉の線
		m_gate->ColliderLine();	
		// 壁・床の線
		m_floor->ColliderLine();
		m_wall->ColliderLine();
		// 足場の線
		m_platform->ColliderLine();		
		// ゴールの線
		m_goal->ColliderLine();						
	}

	// デバッグモード（デバッグフォント）
	auto* debugFont = GetUserResources()->GetDebugFont();
	if (m_isDebugMode)
	{
		debugFont->AddString(L"GameplayScene", DirectX::SimpleMath::Vector2(0.0f, debugFont->GetFontHeight()));
		debugFont->AddString(L"Press 'R' to Try Again", DirectX::SimpleMath::Vector2(0.0f, 70.0f));
		debugFont->AddString((L"PlayerPosX: " + std::to_wstring(m_player->GetPosition().x)).c_str(), DirectX::SimpleMath::Vector2(0.0f, 190.0f));
		debugFont->AddString((L"PlayerPosY: " + std::to_wstring(m_player->GetPosition().y)).c_str(), DirectX::SimpleMath::Vector2(0.0f, 220.0f));
		debugFont->AddString((L"PlayerPosZ: " + std::to_wstring(m_player->GetPosition().z)).c_str(), DirectX::SimpleMath::Vector2(0.0f, 250.0f));
		debugFont->AddString((L"Lives: " + std::to_wstring(m_player->GetLives())).c_str(), DirectX::SimpleMath::Vector2(0.0f, 500.0f));

		if (m_isFollowCamera)
		{
			debugFont->AddString(L"FollowCamera:ON", DirectX::SimpleMath::Vector2(0.0f, 300.0f));
		}
		else if (!m_isFollowCamera)
		{
			debugFont->AddString(L"FollowCamera:OFF", DirectX::SimpleMath::Vector2(0.0f, 300.0f));
		}
	}
	debugFont->Render(GetUserResources()->GetCommonStates());
#endif

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
	// 操作説明の描画
	if (m_showExplanationFirst)
	{
		m_spriteBatch->Draw(m_explanationFirstSRV.Get(), ScreenManager::Pos(900.0f, 30.0f), nullptr,
			DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
			ScreenManager::Scale(1.0f, 1.0f));
	}
	if (m_showExplanationSecond)
	{
		m_spriteBatch->Draw(m_explanationSecondSRV.Get(), ScreenManager::Pos(900.0f, 30.0f), nullptr,
			DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
			ScreenManager::Scale(1.0f, 1.0f));
	}

	// タスクマネージャーの描画処理
	m_taskManager.Render();

	// 渦巻パーティクルの描画
	if (m_isTeleporting) m_swirlParticle->Render();

	// チュートリアル中またはポーズメニューは画面を暗くする
	if (m_tutorialManager->IsPlayerLocked() || m_isPause)
	{
		// 画面全体を覆う矩形
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

	// チュートリアル操作説明の描画
	if (m_tutorialManager->IsPlayerLocked())
	{
		switch (m_tutorialManager->GetState())
		{
		// プレイヤーを動かす説明
		case TutorialManager::TutorialState::MOVEMENT:
			m_spriteBatch->Draw(m_tmoveSRV.Get(), ScreenManager::Pos(100.0f, 260.0f), nullptr,
				DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
				ScreenManager::Scale(1.0f, 1.0f));
			break;
		// マウスの動かし方の説明
		case TutorialManager::TutorialState::MOUSE:
			m_spriteBatch->Draw(m_tmovingMouseSRV.Get(), ScreenManager::Pos(100.0f, 260.0f), nullptr,
				DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
				ScreenManager::Scale(1.0f, 1.0f));
			break;
		case TutorialManager::TutorialState::LIFTANDDROP:
			m_spriteBatch->Draw(m_tliftSRV.Get(), ScreenManager::Pos(100.0f, 260.0f), nullptr,
				DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
				ScreenManager::Scale(1.0f, 1.0f));
			break;
		case TutorialManager::TutorialState::SWITCH:
			m_spriteBatch->Draw(m_tputSRV.Get(), ScreenManager::Pos(100.0f, 260.0f), nullptr,
				DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
				ScreenManager::Scale(1.0f, 1.0f));
			break;
		case TutorialManager::TutorialState::ITEM:
			m_spriteBatch->Draw(m_titemSRV.Get(), ScreenManager::Pos(100.0f, 260.0f), nullptr,
				DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
				ScreenManager::Scale(1.0f, 1.0f));
			break;
		case TutorialManager::TutorialState::ATTACK:
			m_spriteBatch->Draw(m_tattackSRV.Get(), ScreenManager::Pos(100.0f, 260.0f), nullptr,
				DirectX::Colors::White, 0.0f, DirectX::SimpleMath::Vector2::Zero,
				ScreenManager::Scale(1.0f, 1.0f));
			break;
		}
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
}

/*
* @brief 終了処理
*
* @param[in]  なし
*/
void TutorialScene::Finalize()
{
	m_player->Finalize();
	OnDeviceLost();
}

/*
* @brief ゲームのリセット処理
*
* @param[in]  なし
*/
void TutorialScene::ResetGame()
{
	// オブジェクトのリセット
	ResetObjects();
}

/*
* @brief オブジェクトのリセット
*
* @param[in]  なし
*/
void TutorialScene::ResetObjects()
{
	// BGMの再生
	SoundManager::GetInstance().PlayLoop(L"TUTORIALBGM", 1);

	// チュートリアルを始める
	m_tutorialManager->Start();
	m_liftTutorialShown = false;
	m_switchShown = false;
	m_itemShown = false;

	// 床情報を渡す
	m_player->SetFloorData(m_floor->GetPositions(), m_floor->GetScales());
	m_player->SetPlatformData(m_platform->GetPositions(), m_platform->GetScales());
	// プレイヤーの初期化
	m_player->Initialize({ 1.0f, 0.0f, 0.0f });
	// カメラをプレイヤークラスに渡す
	m_player->SetDebugCamera(m_gameCamera.get());

	// 敵
	for (size_t i = 0; i < m_enemies.size(); ++i)
	{
		// 初期化
		m_enemies[i]->Initialize(m_enemyStartPositions[i]);

		// 床情報を渡す
		m_enemies[i]->SetFloorData(m_floor->GetPositions(), m_floor->GetScales());
		m_enemies[i]->SetPlatformData(m_platform->GetPositions(), m_platform->GetScales());
	}

	// 仕掛けブロックのリセット
	m_followingBlock = nullptr;
	for (auto& block : m_gimmickBlocks)
	{
		if (block)
		{
			block->Reset();
		}
	}

	// スイッチフラグのリセット
	m_isSwitchOn_Key = false;
	m_isSwitchOn_PF.assign(m_isSwitchOn_PF.size(), false);

	// アイテム用仕掛けブロックの非表示解除
	for (size_t i = 0; i < m_itemGimmickBlockIndices.size(); ++i)
	{
		size_t index = m_itemGimmickBlockIndices[i];
		if (index < m_gimmickBlocks.size() && m_gimmickBlocks[index])
		{
			m_gimmickBlocks[index]->SetIsVisible(true);
		}
	}

	// アイテムのリセット
	m_isSwitchOn_Item.assign(m_itemSpawnPositions.size(), false);
	m_itemTypes.assign(m_itemSpawnPositions.size(), ItemType::NONE);

	// アイテムリストをクリア
	m_item->ClearItems();

	// ポーズ画面の解除
	m_isPause = false;

	// タイマーのリセット
	m_timer = 0.0f;
	m_isTimerActive = true;
}

float TutorialScene::GetClosestHitDistance(const DirectX::SimpleMath::Vector3& origin, const DirectX::SimpleMath::Vector3& direction, float maxDistance) const
{
	float closest = maxDistance;
	DirectX::SimpleMath::Ray ray(origin, direction);

	auto Check = [&](const auto& obj) {
		if (!obj) return;
		auto& pos = obj->GetPositions();
		auto& scale = obj->GetScales();
		for (size_t i = 0; i < pos.size(); ++i)
		{
			DirectX::BoundingBox box(pos[i], scale[i] * 0.5f);
			float d = 0;
			// 0.1f 以下の至近距離（自分自身）は無視して、壁抜け判定のみ拾う
			if (ray.Intersects(box, d) && d > 0.1f && d < closest)
			{
				closest = d;
			}
		}
		};

	Check(m_floor);
	Check(m_wall);
	Check(m_platform);

	return closest;
}

/*
* @brief デバイスに依存するリソースを作成する関数
*
* @param[in]  なし
*/
void TutorialScene::CreateDeviceDependentResources()
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

	// モデルの読み込み
	m_floorModel = DirectX::Model::CreateFromSDKMESH(device, L"Resources/Models/floor.sdkmesh", *fx);
	m_wallModel = DirectX::Model::CreateFromSDKMESH(device, L"Resources/Models/wall.sdkmesh", *fx);
	m_platformBlockModel = DirectX::Model::CreateFromSDKMESH(device, L"Resources/Models/platform_block.sdkmesh", *fx);
	m_keyBlockModel = DirectX::Model::CreateFromSDKMESH(device, L"Resources/Models/key.sdkmesh", *fx);
	m_portalBlockModel = DirectX::Model::CreateFromSDKMESH(device, L"Resources/Models/portal_block.sdkmesh", *fx);
	m_itemBlockModel = DirectX::Model::CreateFromSDKMESH(device, L"Resources/Models/item_block.sdkmesh", *fx);
	m_switchModel = DirectX::Model::CreateFromSDKMESH(device, L"Resources/Models/switch.sdkmesh", *fx);
	m_portalModel = DirectX::Model::CreateFromSDKMESH(device, L"Resources/Models/portal.sdkmesh", *fx);

	// 画像の読み込み
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/explanation1.dds", nullptr, m_explanationFirstSRV.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/explanation2.dds", nullptr, m_explanationSecondSRV.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/tutorialstageFont.dds", nullptr, m_tutorialstageFont.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/clearFont.dds", nullptr, m_clearFont.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/number.dds", nullptr, m_numberSRV.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/time.dds", nullptr, m_timeSRV.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/pauseKey.dds", nullptr, m_pauseKeySRV.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/selectKey.dds", nullptr, m_selectKeySRV.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/Tutorial/moving.dds", nullptr, m_tmoveSRV.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/Tutorial/moving_mouse.dds", nullptr, m_tmovingMouseSRV.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/Tutorial/lift_block.dds", nullptr, m_tliftSRV.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/Tutorial/put_switch.dds", nullptr, m_tputSRV.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/Tutorial/item_Ekey.dds", nullptr, m_titemSRV.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/Tutorial/item_explanation.dds", nullptr, m_tattackSRV.ReleaseAndGetAddressOf()));

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

	// 床・壁の生成
	m_floor = std::make_unique<StageObject>();
	m_floor->SetDeviceResources(deviceResources);
	m_floor->CreateDeviceDependentResources();
	m_floor->Initialize(device, L"Resources/Models/floor.sdkmesh");
	m_wall = std::make_unique<StageObject>();
	m_wall->SetDeviceResources(deviceResources);
	m_wall->Initialize(device, L"Resources/Models/wall.sdkmesh");
	m_wall->CreateDeviceDependentResources();

	// 足場の生成
	m_platform = std::make_unique<Platform>();
	m_platform->SetDeviceResources(deviceResources);
	m_platform->CreateDeviceDependentResources();

	// スイッチの生成
	m_switch = std::make_shared<Switch>();
	m_switch->Initialize(context);

	// ポータルの生成
	m_portal = std::make_shared<Portal>();
	m_portal->Initialize(context);

	// 扉の生成
	m_gate = std::make_unique<Gate>();
	m_gate->SetDeviceResources(deviceResources);
	m_gate->SetCommonStates(states);

	// アイテムの生成
	m_item = std::make_unique<Item>();
	m_item->SetDeviceResources(deviceResources);
	m_item->SetCommonStates(states);

	// 各パーティクルの生成
	m_portalParticle = std::make_unique<PortalParticle>(); // ポータル
	m_portalParticle->Create(deviceResources);
	m_swirlParticle = std::make_unique<SwirlParticle>();  // ポータルの渦巻き
	m_swirlParticle->Create(deviceResources);
	m_switchParticle = std::make_unique<SwitchParticle>(); // スイッチ
	m_switchParticle->Create(deviceResources);

	// ゴールの生成
	m_goal = std::make_shared<Goal>();
	m_goal->SetDeviceResources(deviceResources);
	m_goal->SetCommonStates(states);

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
* @param[in] なし
*/
void TutorialScene::CreateWindowSizeDependentResources()
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
* @param[in] なし
*/
void TutorialScene::OnDeviceLost()
{
	m_primitiveBatch.reset();
	m_basicEffect.reset();
	m_gameCamera.reset();

	SoundManager::GetInstance().Update();
}