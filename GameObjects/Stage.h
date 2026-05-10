//--------------------------------------------------------------------------------------
// File: Stage.h
//
// ステージを構成するクラス
//--------------------------------------------------------------------------------------
#pragma once

#include "GameObjects/Gimmicks/Item.h"
#include "GameObjects/Effects/PortalParticle.h"
#include "GameObjects/Effects/SwirlParticle.h"
#include "GameObjects/Effects/SwitchParticle.h"
#include "SKLib/UserResources.h"
#include "SKLib/GameCamera.h"
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

class Stage : public ICameraCollisionProvider
{
public:
	// コンストラクタ／デストラクタ
	Stage();
	~Stage();

	// ステージの読み込み
	void Load(const std::string& filePath,Player* player, std::function<void()> onTeleport);

	// 更新処理
	void Update(float elapsedTime, Player* player, std::vector<std::unique_ptr<Enemy>>& enemies,bool isTeleporting);

	// 描画処理
	void Render(ID3D11DeviceContext* context, const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj); 

	// オブジェクトのリセット
	void ResetObject();

	// デバイスに依存するリソースを作成する関数
	void CreateDeviceDependentResources(ID3D11Device* device, ID3D11DeviceContext* context, 
		DX::DeviceResources* deviceResources, DirectX::CommonStates* states, UserResources* userResources);

	// ビルボードの更新
	void UpdateBillboard(const DirectX::SimpleMath::Vector3& target, const DirectX::SimpleMath::Vector3& eye);

	// カメラからの距離を計算する関数
	float GetClosestHitDistance(const DirectX::SimpleMath::Vector3& origin,
		const DirectX::SimpleMath::Vector3& direction,
		float maxDistance) const;

	// デバッグモードでの処理
	void Debug();

	// ギミックの状態を取得
	std::shared_ptr<Switch> GetSwitch() const { return m_switch; }
	std::shared_ptr<Portal> GetPortal() const { return m_portal; }

	// 床の判定を取得
	StageObject* GetFloor() const { return m_floor.get(); }
	// 足場の判定を取得
	Platform* GetPlatform() const { return m_platform.get(); }

private:
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

	// モデル
	std::shared_ptr<DirectX::Model> m_platformBlockModel;
	std::shared_ptr<DirectX::Model> m_keyBlockModel;
	std::shared_ptr<DirectX::Model> m_portalBlockModel;
	std::shared_ptr<DirectX::Model> m_itemBlockModel;
	std::unique_ptr<DirectX::Model> m_switchModel;
	std::unique_ptr<DirectX::Model> m_portalModel;

	std::unique_ptr<StageObject> m_floor;
	std::unique_ptr<StageObject> m_wall;
	std::unique_ptr<Platform> m_platform;
	std::shared_ptr<Switch> m_switch;
	std::shared_ptr<Portal> m_portal;
	std::unique_ptr<Gate> m_gate;
	std::shared_ptr<Goal> m_goal;
	// アイテム
	std::unique_ptr<Item> m_item;
	std::vector<DirectX::SimpleMath::Vector3> m_itemSpawnPositions;

	// パーティクル
	std::unique_ptr<PortalParticle> m_portalParticle;
	std::unique_ptr<SwitchParticle> m_switchParticle;

	std::vector<std::shared_ptr<GimmickBlock>> m_gimmickBlocks; ///< 仕掛けブロック群
	std::vector<size_t> m_itemGimmickBlockIndices;				///< アイテム用仕掛けブロックのインデックス記録する変数
	std::shared_ptr<GimmickBlock> m_followingBlock;             ///< プレイヤーを追従するブロック

	UserResources* m_userResources = nullptr; 

	// 各ギミックのON／OFF
	std::vector<bool> m_isSwitchOn_PF;		///< 足場
	bool m_isSwitchOn_Key;					///< カギ
	std::vector<bool> m_isSwitchOn_Portal;	///< ポータル
	std::vector<bool> m_isSwitchOn_Item;	///< アイテム

	// タイマー
	float m_teleportTimer;

	//　スイッチごとの固定タイプ
	std::vector<ItemType> m_itemTypes;
};

