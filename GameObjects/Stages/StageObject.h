//--------------------------------------------------------------------------------------
// File: StageObject.h
//
// ステージオブジェクト（壁・床）クラス
//--------------------------------------------------------------------------------------
#pragma once
#include "SKLib/CollisionManager.h"
#include "SKLib/DeviceResources.h"
#include <vector>

// 前方宣言
class Player;
class Enemy;

class StageObject
{
public:
	// コンストラクタ
	StageObject();

	// デストラクタ
	virtual ~StageObject();

	// 初期化処理
	void Initialize(ID3D11Device* device, const wchar_t* path);

	// データの読み込み
	void LoadData(const std::vector<DirectX::SimpleMath::Vector3>& positions,
		const std::vector<DirectX::SimpleMath::Vector3>& scales);

	// 更新処理
	void Update(Player* player, const std::vector<std::unique_ptr<Enemy>>& enemies);

	// 描画処理
	void Render(ID3D11DeviceContext* context, const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& proj, DirectX::CommonStates* states);

	// デバイスに依存するリソースを作成する関数
	void CreateDeviceDependentResources();

	// コライダーの線
	void ColliderLine();

	// デバイスリソースの設定
	void SetDeviceResources(class DX::DeviceResources* deviceResources) { m_deviceResources = deviceResources; }

	// 位置・大きさを取得
	const std::vector<DirectX::SimpleMath::Vector3>& GetPositions() const { return m_positions; }
	const std::vector<DirectX::SimpleMath::Vector3>& GetScales() const { return m_scales; }

private:
	class DX::DeviceResources* m_deviceResources = nullptr;
	DirectX::CommonStates* m_states = nullptr;

	// 位置
	std::vector<DirectX::SimpleMath::Vector3> m_positions;
	// 大きさ
	std::vector<DirectX::SimpleMath::Vector3> m_scales;
	// コリジョン
	std::vector<AABB> m_collisions;
	// モデル
	std::unique_ptr<DirectX::Model> m_model;

	// プリミティブバッチ
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_primitiveBatch;
};

