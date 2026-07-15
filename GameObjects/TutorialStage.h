//--------------------------------------------------------------------------------------
// File: TutorialStage.cpp
//
// チュートリアルステージを構成するクラス
//--------------------------------------------------------------------------------------
#pragma once

#include "GameObjects/Gimmicks/Item.h"
#include "GameObjects/Effects/PortalParticle.h"
#include "GameObjects/Effects/SwirlParticle.h"
#include "GameObjects/Effects/SwitchParticle.h"
#include "GameObjects/Effects/CorrectSwitchParticle.h"
#include "GameObjects/Effects/BlockPutParticle.h"
#include "SKLib/GameCamera.h"
#include "SKLib/UserResources.h"
#include <string>
#include "Resources/json.hpp"

// 前方宣言
class UserResources;
class Player;
class Enemy;
class StageObject;
class Platform;
class Switch;
class Portal;
class Gate;
class Goal;
class Item;
class GimmickBlock;

class TutorialStage : public ICameraCollisionProvider
{
public:
	// ゲッター／セッター -------------------------------------------------------------------
	// --- ギミック関連  ---
	// スイッチの状態を取得
	std::shared_ptr<Switch> GetSwitch() const { return m_switch; }
	// ポータルの状態を取得
	std::shared_ptr<Portal> GetPortal() const { return m_portal; }

	// --- ステージ・足場判定  ---
	// 床の判定（オブジェクト）を取得
	StageObject* GetFloor() const { return m_floor.get(); }
	// 足場の判定を取得
	Platform* GetPlatform() const { return m_platform.get(); }

	// --- チュートリアル進行状況  ---
	// 開始チュートリアルが表示されたかどうかを取得
	bool IsStartTutorialShown() const { return m_isStartTutorialShown; }
	// スイッチチュートリアルが表示されたかどうかを取得
	bool IsSwitchTutorialShown() const { return m_isSwitchShown; }
	// アイテムチュートリアルが表示されたかどうかを取得
	bool IsItemTutorialShown() const { return m_isItemShown; }
	// 持ち上げ・投げチュートリアルが表示されたかどうかを取得
	bool IsLiftAndDropTutorialShown() const { return m_isLiftanddropShown; }

	// --- 衝突・レイキャスト処理  ---
	// カメラからの距離を計算する関数
	float GetClosestHitDistance(const DirectX::SimpleMath::Vector3& origin,
		const DirectX::SimpleMath::Vector3& direction,
		float maxDistance) const;

public:
	// 関数 ---------------------------------------------------------------------------------
	// コンストラクタ／デストラクタ
	TutorialStage();
	~TutorialStage();
	// ステージの読み込み
	void Load(const std::string& filePath, Player* player, std::function<void()> onTeleport);
	// 更新処理
	void Update(float elapsedTime, Player* player, std::vector<std::unique_ptr<Enemy>>& enemies, bool isTeleporting, bool isPlayerLocked);
	// 描画処理
	void Render(ID3D11DeviceContext* context, const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj);
	// オブジェクトのリセット
	void ResetObject();
	// デバイスに依存するリソースを作成する関数
	void CreateDeviceDependentResources(ID3D11Device* device, ID3D11DeviceContext* context,
		DX::DeviceResources* deviceResources, DirectX::CommonStates* states, UserResources* userResources);
	// ビルボードの更新
	void UpdateBillboard(const DirectX::SimpleMath::Vector3& target, const DirectX::SimpleMath::Vector3& eye);
	// デバッグモードでの処理
	void Debug();

private:
	// 定数 ------------------------------------------------------------------------
	static const float HALF_SIZE;									///< 半分のサイズ

	static const DirectX::SimpleMath::Vector3 DEFAULT_BLOCK_SCALE;	///< 生成するギミックブロックの標準サイズ
	static const DirectX::SimpleMath::Vector3 DEFAULT_SWITCH_SCALE;	///< スイッチオブジェクトの標準サイズ
	static const DirectX::SimpleMath::Vector3 BLOCK_FOLLOW_OFFSET;	///< ブロック追尾時の高さオフセット

	static const float ITEM_SPAWN_TIMER;							///< アイテムがスポーンするまでの時間
	static const int BLOCK_FOLLOW_SPEED_MULTIPLIER;					///< ブロックの追従速度の倍率

	static const float TELEPORT_DURATION;							///< テレポートの演出・クールダウン時間（秒）
	static const float TIMER_END_THRESHOLD;							///< タイマー終了の基準値

	static const DirectX::SimpleMath::Vector3 WORLD_UP_VECTOR;		///< 世界の真上を指す上方向ベクトル

	static const float SELF_HIT_INIT_DIST;							///< レイ発射直後の自分自身への誤判定を防ぐための最小距離
private:
	// メンバ変数 ------------------------------------------------------------------
	// ビュー行列
	DirectX::SimpleMath::Matrix m_view;
	// 射影行列
	DirectX::SimpleMath::Matrix m_proj;
	// 共通ステート
	std::unique_ptr<DirectX::CommonStates> m_states;
	// ベーシックエフェクト
	std::unique_ptr<DirectX::BasicEffect> m_basicEffect;
	// プリミティブバッチ
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_primitiveBatch;
	// 足場ブロックモデル
	std::shared_ptr<DirectX::Model> m_platformBlockModel;
	// 鍵ブロックモデル
	std::shared_ptr<DirectX::Model> m_keyBlockModel;
	// ポータルブロックモデル
	std::shared_ptr<DirectX::Model> m_portalBlockModel;
	// アイテムブロックモデル
	std::shared_ptr<DirectX::Model> m_itemBlockModel;
	// スイッチモデル
	std::unique_ptr<DirectX::Model> m_switchModel;
	// ポータルモデル
	std::unique_ptr<DirectX::Model> m_portalModel;
	// 床のオブジェクト
	std::unique_ptr<StageObject> m_floor;
	// 壁のオブジェクト
	std::unique_ptr<StageObject> m_wall;
	// 足場のオブジェクト
	std::unique_ptr<Platform> m_platform;
	// スイッチのオブジェクト
	std::shared_ptr<Switch> m_switch;
	// ポータルのオブジェクト
	std::shared_ptr<Portal> m_portal;
	// ゲートのオブジェクト
	std::unique_ptr<Gate> m_gate;
	// ゴールのオブジェクト
	std::shared_ptr<Goal> m_goal;
	// アイテム
	std::unique_ptr<Item> m_item;
	// アイテムが出現する座標のリスト
	std::vector<DirectX::SimpleMath::Vector3> m_itemSpawnPositions;
	// ポータルパーティクル
	std::unique_ptr<PortalParticle> m_portalParticle;
	// スイッチパーティクル
	std::unique_ptr<SwitchParticle> m_switchParticle;
	// ブロックを置いたときのパーティクル
	std::unique_ptr<BlockPutParticle> m_blockPutParticle;
	// 正解のときにでるパーティクル
	std::unique_ptr<CorrectSwitchParticle> m_correctSwitchParticle;
	// 仕掛けブロック群
	std::vector<std::shared_ptr<GimmickBlock>> m_gimmickBlocks;
	// アイテム用仕掛けブロックのインデックス記録する変数
	std::vector<size_t> m_itemGimmickBlockIndices;
	// プレイヤーを追従するブロック
	std::shared_ptr<GimmickBlock> m_followingBlock;
	// ユーザーリソース
	UserResources* m_userResources = nullptr;
	// カメラ
	std::unique_ptr<GameCamera> m_camera;
	// 各ギミックのON／OFF
	std::vector<bool> m_isSwitchOn_PF;		///< 足場
	bool m_isSwitchOn_Key;					///< カギ
	std::vector<bool> m_isSwitchOn_Portal;	///< ポータル
	std::vector<bool> m_isSwitchOn_Item;	///< アイテム
	// チュートリアル開始の表示フラグ
	bool m_isStartTutorialShown;
	// スイッチについてのチュートリアルの表示フラグ
	bool m_isSwitchShown;
	// アイテムについてのチュートリアルの表示フラグ
	bool m_isItemShown;
	// 持ち上げ・設置についてのチュートリアルの表示フラグ
	bool m_isLiftanddropShown;
	// 剣の処理
	bool m_isSwordCollected = false;
	bool m_isSwordSystemActive = false;
	// テレポートタイマー
	float m_teleportTimer;
	// 剣が出現するまでのタイマー
	float m_swordRespawnTimer;
	//　スイッチごとの固定タイプ
	std::vector<ItemType> m_itemTypes;
};

