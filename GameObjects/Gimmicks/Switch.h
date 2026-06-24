//--------------------------------------------------------------------------------------
// File: Switch.h
//
// スイッチクラス
//--------------------------------------------------------------------------------------
#pragma once

#include "SKLib/CollisionManager.h"
#include "GameObjects/Gimmicks/GimmickBlock.h"

// スイッチの種類の列挙体
enum class SwitchTargetType
{
	SW_PLATFORM,	///< 足場
	SW_KEY,			///< カギ
	SW_PORTAL,		///< ポータル
	SW_ITEM,		///< アイテム
	ANY,			///< デフォルト
	NONE			///< 何もない場合
};

// スイッチのデータ構造体
struct SwitchData
{
	DirectX::SimpleMath::Vector3 position;	///< 位置
	DirectX::SimpleMath::Vector3 scale;		///< 大きさ
	SwitchTargetType switchType;			///< スイッチの種類
	std::function<void()> onActivate;		///< 作動しているかどうか
};

class Switch
{
public:
	// ゲッター／セッター -------------------------------------------------------------------
	// スイッチが作動したら
	bool IsSwitchOn(size_t index) const;

	// モデルの設定
	void SetModel(DirectX::Model* model) { m_model = model; }

	// スイッチデータの取得
	const std::vector<SwitchData>& GetSwitches() const { return m_switches; }
public:
	// 関数 ---------------------------------------------------------------------------------
	// コンストラクタ／デストラクタ
	Switch();
	~Switch();

	// 初期化処理
	void Initialize(ID3D11DeviceContext* context);

	// 更新処理
	void Update(const std::vector<std::shared_ptr<GimmickBlock>>& gimmickBlocks);

	// 描画処理
	void Render(ID3D11DeviceContext* context, const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& proj, DirectX::CommonStates* states);

	// 各スイッチの追加処理
	void AddSwitch(const SwitchData& data);

	// コライダーの線
	void ColliderLine();

private:
	// 定数 ------------------------------------------------------------------------
	static const float HALF_SCALE;				///< 半分のサイズにする

private:
	// メンバ変数 ------------------------------------------------------------------
	// プリミティブバッチ
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_primitiveBatch;

	// 各スイッチ
	std::vector<SwitchData> m_switches;
	std::vector<AABB> m_collisions;
	std::vector<bool> m_switchStates;

	// モデル
	DirectX::Model* m_model = nullptr;

	// スイッチの状態
	std::vector<bool> m_isSwitchOn_Item;
};

