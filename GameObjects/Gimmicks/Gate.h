//--------------------------------------------------------------------------------------
// File: Gate.h
//
// ゲートクラス
//--------------------------------------------------------------------------------------
#pragma once

#include "SKLib/CollisionManager.h"
#include "SKLib/SoundManager.h"

#include "GameObjects/Stages/Player.h"
#include "GameObjects/Stages/Enemy.h"

class Gate
{
public:
	// コンストラクタ／デストラクタ
	Gate();
	~Gate();
	
	// 初期化処理
	void Initialize();

	// 更新処理
	void Update(Player* player, std::vector<std::unique_ptr<Enemy>>& enemies);

	// 描画処理
	void Render(const DirectX::SimpleMath::Matrix& view);

	// デバイスに依存するリソースを作成する関数
	void CreateDeviceDependentResources();

	// 開閉処理
	void Open() { m_isOpen = true; }
	void Close() { m_isOpen = false; }
	bool IsOpen() const { return m_isOpen; }

	// デバイスリソースの設定
	void SetDeviceResources(DX::DeviceResources* deviceResources) { m_deviceResources = deviceResources; }

	// 共通ステートの設定
	void SetCommonStates(DirectX::CommonStates* states) { m_states = states; }

	// 位置を設定
	void SetPosition(float x, float y, float z);

	// コライダーの線
	void ColliderLine();

private:
	DX::DeviceResources* m_deviceResources = nullptr;
	DirectX::CommonStates* m_states = nullptr;

	// ビュー行列
	DirectX::SimpleMath::Matrix m_view;

	// 射影行列
	DirectX::SimpleMath::Matrix m_proj;

	// 平行移動の行列
	DirectX::SimpleMath::Matrix m_trans;

	// ベーシックエフェクト
	std::unique_ptr<DirectX::BasicEffect> m_basicEffect;

	// プリミティブバッチ
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_primitiveBatch;

	// インプットレイアウト
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

	// モデル
	std::unique_ptr<DirectX::Model> m_model;

	// モデルの位置
	DirectX::SimpleMath::Vector3 m_gatePosition;

	// モデルの大きさ
	DirectX::SimpleMath::Vector3 m_gateScale;

	// 平行移動の行列
	DirectX::SimpleMath::Matrix m_gateTrans;

	// 回転角度
	float m_gateRotAngle;
	const float m_rotSpeed = DirectX::XMConvertToRadians(80.0f);

	// 開閉フラグ
	bool m_isOpen = false;

	// 当たり判定
	AABB m_gateCollision;
};

