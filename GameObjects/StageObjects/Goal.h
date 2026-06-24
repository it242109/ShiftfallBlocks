//--------------------------------------------------------------------------------------
// File: Goal.h
//
// ゴールクラス
//--------------------------------------------------------------------------------------
#pragma once

#include "SKLib/CollisionManager.h"
#include "SKLib/SceneManager.h"

#include "Player.h"

class Goal
{
public:
	// ゲッター／セッター -------------------------------------------------------------------
	// --- 位置  ---
	// 位置を設定
	void SetPosition(float x, float y, float z);

	// --- 角度  ---
	// 角度を設定
	void SetRotation(float x, float y, float z);

	// --- システム・グラフィックス  ---
	// デバイスリソースの設定
	void SetDeviceResources(DX::DeviceResources* deviceResources) { m_deviceResources = deviceResources; }

	// 共通ステートの設定
	void SetCommonStates(DirectX::CommonStates* states) { m_states = states; }

public:
	// 関数 ---------------------------------------------------------------------------------
	//	コンストラクタ／デストラクタ
	Goal() = default;
	~Goal() = default;

	// 初期化処理
	void Initialize();

	// 更新処理
	void Update(const AABB& playerCollision, Player* player);

	// 描画処理
	void Render(const DirectX::SimpleMath::Matrix& view);

	// デバイスに依存するリソースを作成する関数
	void CreateDeviceDependentResources();

	// コライダーの線
	void ColliderLine();

private:
	// 定数 ------------------------------------------------------------------------
	static const float BASE_SIZE_MULTIPLIER;	///< 判定のベースサイズに対する倍率
	static const float HALF_SCALE;				///< 半分のサイズにする
	static const float HEIGHT_MULTIPLIER;		///< ゴール判定の高さを縦長に拡張するための倍率

	static const float FIELD_OF_VIEW_DEGREES;   ///< 視野角
	static const float NEAR_PLANE_DISTANCE;		///< カメラの最前面のクリップ距離
	static const float FAR_PLANE_DISTANCE;		///< カメラの最遠面のクリップ距離

private:
	// メンバ変数 ------------------------------------------------------------------
	// デバイスリソース／コモンステート
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
	DirectX::SimpleMath::Vector3 m_goalPosition;

	// モデルの回転
	DirectX::SimpleMath::Vector3 m_goalRotation;

	// スケール
	DirectX::SimpleMath::Vector3 m_goalScale;

	// 当たり判定
	AABB m_goalCollision;
};

