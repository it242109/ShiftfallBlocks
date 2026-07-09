//--------------------------------------------------------------------------------------
// File: TutorialStage.cpp
//
// チュートリアルステージを構成するクラス
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "TutorialStage.h"
#include "StageObjects/Player.h"
#include "StageObjects/Enemy.h"
#include "StageObjects/StageObject.h"
#include "StageObjects/Goal.h"

#include "Gimmicks/Platform.h"
#include "Gimmicks/Switch.h"
#include "Gimmicks/Portal.h"
#include "Gimmicks/Gate.h"
#include "Gimmicks/Item.h"
#include "Gimmicks/GimmickBlock.h"

// JSONファイルを使えるようにする
using json = nlohmann::json;

// 定数の定義
const float TutorialStage::HALF_SIZE = 0.5f;												///< 半分のサイズ
const DirectX::SimpleMath::Vector3 TutorialStage::DEFAULT_BLOCK_SCALE(0.7f, 0.7f, 0.7f);	///< 生成するギミックブロックの標準サイズ
const DirectX::SimpleMath::Vector3 TutorialStage::DEFAULT_SWITCH_SCALE(1.0f, 1.0f, 1.0f);	///< スイッチオブジェクトの標準サイズ
const DirectX::SimpleMath::Vector3 TutorialStage::BLOCK_FOLLOW_OFFSET(0.0f, 1.0f, 0.0f);	///< ブロック追尾時の高さオフセット
const float TutorialStage::ITEM_SPAWN_TIMER = 3.0f;											///< アイテムがスポーンするまでの時間
const int TutorialStage::BLOCK_FOLLOW_SPEED_MULTIPLIER = 5;									///< ブロックの追従速度の倍率
const float TutorialStage::TELEPORT_DURATION = 2.0f;										///< テレポートの演出・クールダウン時間（秒）
const float TutorialStage::TIMER_END_THRESHOLD = 0.0f;										///< タイマー終了の基準値
const DirectX::SimpleMath::Vector3 TutorialStage::WORLD_UP_VECTOR(0, 1, 0);					///< 世界の真上を指す上方向ベクトル
const float TutorialStage::SELF_HIT_INIT_DIST = 0.1f;										///< レイ発射直後の自分自身への誤判定を防ぐための最小距離



/*
* @brief コンストラクタ
*
* @param[in]  なし
*
* @return なし
*/
TutorialStage::TutorialStage()
	:
	m_isSwitchOn_Key(false),
	m_isSwitchOn_Item(false),
	m_followingBlock(nullptr),
	m_teleportTimer(0.0f),
	m_swordRespawnTimer(0.0f),
	m_isStartTutorialShown(false),
	m_isSwitchShown(false),
	m_isItemShown(false),
	m_isLiftanddropShown(false)
{
}

/*
* @brief デストラクタ
*
* @param[in]  なし
*
* @return なし
*/
TutorialStage::~TutorialStage()
{
}

/*
* @brief ステージの読み込み
*
* @param[in]  filePath ステージデータのファイルパス
* @param[in]  player   プレイヤーオブジェクトへのポインタ
* @param[in]  onTeleport テレポート時のコールバック関数
*
* @return なし
*/
void TutorialStage::Load(const std::string& filePath, Player* player, std::function<void()> onTeleport)
{
	// JSONステージデータをロード
	std::ifstream file(filePath);
	if (!file.is_open())
	{
		MessageBox(NULL, L"ステージファイルが見つかりません。", L"エラー", MB_OK | MB_ICONHAND);
		abort();
		return;
	}
	json data;
	file >> data;

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
		for (auto& pf : data["platforms"])
		{
			pos.push_back({ pf["position"][0].get<float>(), pf["position"][1].get<float>(), pf["position"][2].get<float>() });
			scale.push_back({ pf["scale"][0].get<float>(), pf["scale"][1].get<float>(), pf["scale"][2].get<float>() });
		}
		m_platform->LoadPlatformData(pos, scale);

		// スイッチの状態をリセットしてリサイズ
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
	if (data.contains("gimmick_blocks"))
	{
		for (auto& gb : data["gimmick_blocks"])
		{
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
			block->Initialize(targetModel, pos, DEFAULT_BLOCK_SCALE, targetType);
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
	if (data.contains("portals")) {
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

			auto teleportAction = [player, onTeleport, dx, dy, dz]() {
				// プレイヤーを移動させる
				player->SetPosition(dx, dy, dz);
				onTeleport();
				};
			// ポータルの追加
			m_portal->AddPortal({ pos, scale, pType, teleportAction }, sIdx);
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

	// ゴールの読み込み
	if (data.contains("goal"))
	{
		auto& g = data["goal"];
		m_goal->SetPosition(g["position"][0], g["position"][1], g["position"][2]);
		m_goal->SetRotation(g["rotation"][0], g["rotation"][1], g["rotation"][2]);
	}
	m_goal->Initialize();

	// スイッチの読み込み／各スイッチの追加
	m_switch->SetModel(m_switchModel.get());
	if (data.contains("switches")) 
	{
		for (auto& sw : data["switches"])
		{
			DirectX::SimpleMath::Vector3 pos = { sw["position"][0], sw["position"][1], sw["position"][2] };
			std::string typeStr = sw["type"].get<std::string>();
			int idx = sw["index"].get<int>();

			SwitchTargetType stype = SwitchTargetType::SW_PLATFORM;
			std::function<void()> action;

			// 足場
			if (typeStr == "SW_PLATFORM")
			{
				stype = SwitchTargetType::SW_PLATFORM;
				action = [this, idx]() { m_isSwitchOn_PF[idx] = true; };
			}
			// ポータル
			else if (typeStr == "SW_PORTAL")
			{
				stype = SwitchTargetType::SW_PORTAL;
				action = [this, idx]() { m_isSwitchOn_Portal[idx] = true; };
			}
			// カギ
			else if (typeStr == "SW_KEY")
			{
				stype = SwitchTargetType::SW_KEY;
				action = [this]() { m_isSwitchOn_Key = true; };
			}
			// アイテム
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
			// スイッチの追加
			m_switch->AddSwitch({ pos, DEFAULT_SWITCH_SCALE, stype, action });
		}
	}
}



/*
* @brief 更新処理
*
* @param[in]  elapsedTime   前フレームからの経過時間
* @param[in]  player        プレイヤーオブジェクトへのポインタ
* @param[in]  enemies       敵オブジェクトのリストへの参照
* @param[in]  isTeleporting テレポート中かどうか
* @param[in]  isPlayerLocked プレイヤーがロックされているかどうか
*
* @return なし
*/
void TutorialStage::Update(float elapsedTime, Player* player, std::vector<std::unique_ptr<Enemy>>& enemies, 
	bool isTeleporting, bool isPlayerLocked)
{
	// 毎フレーム時にリセット ---------------------------------
	// カギ
	m_isSwitchOn_Key = false;
	// スイッチ
	for (int i = 0; i < m_isSwitchOn_PF.size(); ++i)
	{
		m_isSwitchOn_PF[i] = false;
	}
	// ポータル
	for (int i = 0; i < m_isSwitchOn_Portal.size(); ++i)
	{
		m_isSwitchOn_Portal[i] = false;
	}
	// --------------------------------------------------------

	// 床に触れたらチュートリアルが表示される
	if (!m_isStartTutorialShown && player->IsOnFloor())
	{
		m_isStartTutorialShown = true;
	}

	// 仕掛けブロックの更新
	if (!isTeleporting)
	{
		float blockFollowSpeed = (player->GetVelocity() / elapsedTime) * BLOCK_FOLLOW_SPEED_MULTIPLIER;

		for (auto& block : m_gimmickBlocks)
		{
			if (!block)
				continue;

			// プレイヤーとブロックが当たっているか
			bool isColliding = block->CheckCollision(player->GetCollision());

			// ブロックとの接触時チュートリアル
			if (isColliding)
			{
				m_isLiftanddropShown = true;

				// 右クリックで持ち上げ／設置の処理
				if (!isPlayerLocked && InputManager::Get().IsMousePressedRight())
				{
					// 持ち上げる
					if (!block->IsFollowing() && !m_followingBlock)
					{
						SoundManager::GetInstance().Play(L"LIFT");

						block->StartFollowing(
							player->GetPosition(),
							blockFollowSpeed);

						block->SetFollowOffset(BLOCK_FOLLOW_OFFSET);
						m_followingBlock = block;
					}
					// 設置
					else if (block->IsFollowing() &&
						!player->IsJumping() &&
						player->IsOnFloor())
					{
						SoundManager::GetInstance().Play(L"PUT");

						block->StopFollowing(player->GetPosition());
						m_followingBlock = nullptr;

						if (m_blockPutParticle)
						{
							blockPutEmitterInfo emitter;
							emitter.position = block->GetPosition();
							emitter.scale = block->GetScale();

							m_blockPutParticle->SetEmitters({ emitter });
						}
					}
				}
			}

			// 追尾中の処理
			if (block->IsFollowing())
			{
				if (!m_isSwitchShown)
				{
					const auto& switches = m_switch->GetSwitches();

					for (const auto& sw : switches)
					{
						auto swHalf = sw.scale * HALF_SIZE;

						AABB switchCollider(
							sw.position - swHalf,
							sw.position + swHalf);

						if (player->GetCollision().CheckAABBCollision(
							player->GetCollision(),
							switchCollider))
						{
							m_isSwitchShown = true;
							break;
						}
					}
				}
				block->UpdateTargetPosition(player->GetPosition());
			}
			block->Update(elapsedTime);
		}
	}
	// 置いたときのパーティクルを更新
	if (m_blockPutParticle)
	{
		m_blockPutParticle->Update(elapsedTime);
	}

	// ギミックの更新処理
	m_switch->Update(m_gimmickBlocks);
	if (!isTeleporting)
	{
		m_portal->Update(player->GetCollision(), elapsedTime, m_isSwitchOn_Portal);
	}
	//m_item->Update(player->GetCollision(), player);
	m_goal->Update(player->GetCollision(), player);

	// プレイヤーがロックされていない場合のみ処理を行う
	if (!isPlayerLocked) 
	{
		m_item->Update(player->GetCollision(), player);
		// アイテムの処理
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
			m_item->Update(player->GetCollision(), player);

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
				if (m_swordRespawnTimer >= ITEM_SPAWN_TIMER)
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
			if (!m_isItemShown)
			{
				for (const auto& item : currentItems)
				{
					if (item.itemType != ItemType::SWORD || !item.isActive) continue;

					// 当たり判定の作成
					DirectX::SimpleMath::Vector3 itemHalf = item.scale * HALF_SIZE;
					AABB itemCollider(item.position - itemHalf, item.position + itemHalf);

					// プレイヤーがアイテムに触れた瞬間
					if (player->GetCollision().CheckAABBCollision(player->GetCollision(), itemCollider))
					{
						m_isItemShown = true;
						break;
					}
				}
			}
		}
	}

	// 地形との衝突判定
	player->ResetFloorHit();
	m_floor->Update(player, enemies);
	m_wall->Update(player, enemies);
	m_platform->Update(m_isSwitchOn_PF, player, enemies);

	// 扉の処理
	m_gate->Update(player, enemies);
	if (m_isSwitchOn_Key)
	{
		// 開ける処理
		m_gate->Open();
	}
	else
	{
		// 閉める処理
		m_gate->Close();
	}

	// 各パーティクルの更新処理
	// スイッチの上に出るパーティクル
	if (m_switch && m_switchParticle)
	{
		std::vector<SwitchEmitterInfo> switchEmitterInfos;

		for (const auto& sw : m_switch->GetSwitches())
		{
			SwitchEmitterInfo info;
			info.position = sw.position;
			info.scale = sw.scale;
			info.type = sw.switchType;

			switchEmitterInfos.push_back(info);
		}
		// パーティクルに位置をセットして更新
		m_switchParticle->SetEmitters(switchEmitterInfos);
		m_switchParticle->Update(elapsedTime);
	}
	// 正しいスイッチに置いたときに出るパーティクル
	if (m_switch && m_correctSwitchParticle)
	{
		std::vector<CorrectSwitchEmitterInfo> correctSwitchEmitterInfos;

		const auto& switches = m_switch->GetSwitches();
		for (size_t i = 0; i < switches.size(); ++i)
		{
			// 条件があっているかをチェック
			if (m_switch->IsSwitchOn(i))
			{
				// 条件が合っているスイッチのデータを取り出す
				const auto& sw = switches[i];

				CorrectSwitchEmitterInfo info;
				info.position = sw.position;
				info.scale = sw.scale;

				// パーティクルの発生源としてリストに追加
				correctSwitchEmitterInfos.push_back(info);
			}
		}
		m_correctSwitchParticle->SetEmitters(correctSwitchEmitterInfos);
		m_correctSwitchParticle->Update(elapsedTime);
	}
	// ポータルの上に出るパーティクル
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
	if (!isTeleporting)
	{
		isTeleporting = true;
		m_teleportTimer = TELEPORT_DURATION;
	}
	else
	{
		m_teleportTimer -= elapsedTime;
		if (m_teleportTimer <= TIMER_END_THRESHOLD)
			isTeleporting = false;
	}
}

/*
* @brief 描画処理
*
* @param[in]  context   デバイスコンテキスト
* @param[in]　view      ビュー行列
* @param[in]  proj      射影行列
*
* @return なし
*/
void TutorialStage::Render(ID3D11DeviceContext* context, const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj)
{
	auto states = m_userResources->GetCommonStates();

	// 扉
	m_gate->Render(view);
	// スイッチ
	m_switch->Render(context, view, proj, states);
	// ポータル
	m_portal->Render(context, view, proj, states, m_isSwitchOn_Portal);
	// アイテム
	m_item->Render(view, proj);
	// 床
	m_floor->Render(context, view, proj, states);
	// 壁
	m_wall->Render(context, view, proj, states);
	// 足場
	m_platform->Render(view, proj);
	// ゴール
	m_goal->Render(view);

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
			m_platformBlockModel->Draw(context, *states, world, view, proj);
			break;

		case BlockType::KEY:
			m_keyBlockModel->Draw(context, *states, world, view, proj);
			break;

		case BlockType::PORTAL:
			m_portalBlockModel->Draw(context, *states, world, view, proj);
			break;

		case BlockType::ITEM:
			m_itemBlockModel->Draw(context, *states, world, view, proj);
			break;

		default:
			break;
		}
	}

	//　各パーティクル
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
	if (portalEnabled) m_portalParticle->Render(view, proj);

	// スイッチ
	m_switchParticle->Render(view, proj);

	// ブロックを置いたときのパーティクル
	m_blockPutParticle->Render(view, proj);
	// 正しいスイッチに置いたときに出るパーティクル
	m_correctSwitchParticle->Render(view, proj);
}

/*
* @brief オブジェクトのリセット
*
* @param[in]  なし
*
* @return なし
*/
void TutorialStage::ResetObject()
{
	m_isSwitchShown = false;
	m_isItemShown = false;
	m_isLiftanddropShown = false;

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

	// アイテム関連の内部状態をリセット
	m_isSwordSystemActive = false;
	m_isSwordCollected = false;
	m_swordRespawnTimer = 0.0f;

	// アイテムリストをクリア
	m_item->ClearItems();
}

/*
* @brief  デバイスに依存するリソースを作成する関数
*
* @param[in]  device　		Direct3Dデバイスのポインタ
* @param[in]  context		デバイスコンテキストへのポインタ
* @param[in]  deviceResources デバイスリソースへのポインタ
* @param[in]  states　		共通ステートへのポインタ
* @param[in]  userResources ユーザーリソースへのポインタ
*
* @return なし
*/
void TutorialStage::CreateDeviceDependentResources(ID3D11Device* device, ID3D11DeviceContext* context,
	DX::DeviceResources* deviceResources, DirectX::CommonStates* states, UserResources* userResources)
{
	m_userResources = userResources;

	std::unique_ptr<DirectX::EffectFactory> fx;
	fx = std::make_unique<DirectX::EffectFactory>(device);
	fx->SetDirectory(L"Resources/Models");

	// モデルの読み込み＆オブジェクトの初期化
	m_platformBlockModel = DirectX::Model::CreateFromSDKMESH(device, L"Resources/Models/platform_block.sdkmesh", *fx);
	m_keyBlockModel = DirectX::Model::CreateFromSDKMESH(device, L"Resources/Models/key.sdkmesh", *fx);
	m_portalBlockModel = DirectX::Model::CreateFromSDKMESH(device, L"Resources/Models/portal_block.sdkmesh", *fx);
	m_itemBlockModel = DirectX::Model::CreateFromSDKMESH(device, L"Resources/Models/item_block.sdkmesh", *fx);
	m_switchModel = DirectX::Model::CreateFromSDKMESH(device, L"Resources/Models/switch.sdkmesh", *fx);
	m_portalModel = DirectX::Model::CreateFromSDKMESH(device, L"Resources/Models/portal.sdkmesh", *fx);
	m_floor = std::make_unique<StageObject>();
	m_floor->Initialize(device, L"Resources/Models/floor.sdkmesh");
	m_wall = std::make_unique<StageObject>();
	m_wall->Initialize(device, L"Resources/Models/wall.sdkmesh");

	// カメラの初期化
	RECT rect = deviceResources->GetOutputSize();
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	m_camera = std::make_unique<GameCamera>(width, height);

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
	// ポータル
	m_portalParticle = std::make_unique<PortalParticle>();
	m_portalParticle->Create(deviceResources);
	// スイッチ
	m_switchParticle = std::make_unique<SwitchParticle>();
	m_switchParticle->Create(deviceResources);
	// ブロックを置いたときのパーティクル
	m_blockPutParticle = std::make_unique<BlockPutParticle>();
	m_blockPutParticle->Create(deviceResources);
	// 正しいスイッチに置いたときに出るパーティクル
	m_correctSwitchParticle = std::make_unique<CorrectSwitchParticle>();
	m_correctSwitchParticle->Create(deviceResources);

	// ゴールの生成
	m_goal = std::make_shared<Goal>();
	m_goal->SetDeviceResources(deviceResources);
	m_goal->SetCommonStates(states);
}

/*
* @brief ビルボードの更新
*
* @param[in]  target ビルボードの向きを合わせる対象の位置
* @param[in]  eye    カメラの位置
*
* @return なし
*/
void TutorialStage::UpdateBillboard(const DirectX::SimpleMath::Vector3& target, const DirectX::SimpleMath::Vector3& eye)
{
	// 各パーティクルのビルボードを更新
	m_portalParticle->Billboard(target, eye, WORLD_UP_VECTOR);
	m_switchParticle->Billboard(target, eye, WORLD_UP_VECTOR);
	m_blockPutParticle->Billboard(target, eye, WORLD_UP_VECTOR);
	m_correctSwitchParticle->Billboard(target, eye, WORLD_UP_VECTOR);
}

/*
* @brief カメラからの距離を計算する関数
*
* @param[in]  origin    　レイの発射点
* @param[in]  direction 　レイの方向
* @param[in]  maxDistance 最大距離
*
* @return 衝突距離を返す（非衝突時は最大距離）
*/
float TutorialStage::GetClosestHitDistance(const DirectX::SimpleMath::Vector3& origin, const DirectX::SimpleMath::Vector3& direction, float maxDistance) const
{
	// レイを作成
	float closest = maxDistance;
	DirectX::SimpleMath::Ray ray(origin, direction);

	// 内部の地形オブジェクトに対して判定を行う
	auto Check = [&](const auto& obj) {
		if (!obj) return;
		auto& pos = obj->GetPositions();
		auto& scale = obj->GetScales();
		for (size_t i = 0; i < pos.size(); ++i)
		{
			DirectX::BoundingBox box(pos[i], scale[i] * HALF_SIZE);
			float d = 0;
			if (ray.Intersects(box, d) && d > SELF_HIT_INIT_DIST && d < closest)
			{
				closest = d;
			}
		}
		};

	// 判定チェック
	Check(m_floor);
	Check(m_wall);
	Check(m_platform);

	return closest;
}

/*
* @brief デバッグモードでの処理
*
* @param[in]  なし
*
* @return なし
*/
void TutorialStage::Debug()
{
#ifdef _DEBUG
	// アイテムの線
	for (size_t i = 0; i < m_isSwitchOn_Item.size(); ++i)
	{
		if (m_isSwitchOn_Item[i])m_item->ColliderLine();
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

#endif // _DEBUG
}