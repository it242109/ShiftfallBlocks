//--------------------------------------------------------------------------------------
// File: Gate.h
//
// ゲートクラス
//--------------------------------------------------------------------------------------
#pragma once

#include "SKLib/CollisionManager.h"
#include "SKLib/SoundManager.h"
#include "SKLib/GameCamera.h"

#include "GameObjects/StageObjects/Player.h"
#include "GameObjects/StageObjects/Enemy.h"

class Gate
{
public:
	// ゲッター／セッター -------------------------------------------------------------------
	// --- 開閉状態 ---
	// 開いているかどうか
	bool IsOpen() const { return m_isOpen; }
	// 開く
	void Open() { m_isOpen = true; }
	// 閉じる
	void Close() { m_isOpen = false; }

	// --- 位置 ---
	// 位置を設定
	void SetPosition(float x, float y, float z);

	// --- カメラ ---
	// カメラを設定
	void SetCamera(std::unique_ptr<GameCamera>& camera) { m_camera = std::move(camera); }

	// --- システム・グラフィックス ---
	// デバイスリソースの設定
	void SetDeviceResources(DX::DeviceResources* deviceResources) { m_deviceResources = deviceResources; }
	// 共通ステートの設定
	void SetCommonStates(DirectX::CommonStates* states) { m_states = states; }

public:
	// 関数 ---------------------------------------------------------------------------------
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
	// コライダーの線
	void ColliderLine();

private:
	// 定数 ---------------------------------------------------------------------------------
	static const float INITIAL_X;					///< 初期位置Ｘ（定数）

	static const DirectX::SimpleMath::Vector3 DEFAULT_SCALE;///< ゲートのデフォルトの大きさ

	static const float MAX_GATE_OPEN;				///< ゲートの最大開き具合
	static const float MIN_GATE_OPEN;				///< ゲートの最小開き具合
	static const float OPEN_CLOSE_SPEED;			///< ゲートの開閉速度
	static const float GATE_COLLISION_SIZE_SCALE;	///< ゲートの当たり判定の大きさのスケール
	static const float GATE_COLLISION_WIDTH_SCALE;	///< ゲートの当たり判定の幅
	static const float GATE_COLLISION_DEPTH_SCALE ;	///< ゲートの当たり判定の奥行

	static const float FIELD_OF_VIEW_DEGREES;		///< 視野角
	static const float NEAR_PLANE_DISTANCE;			///< カメラの最前面のクリップ距離
	static const float FAR_PLANE_DISTANCE;			///< カメラの最遠面のクリップ距離
private:
	// メンバ変数 ---------------------------------------------------------------------------
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
	// カメラ
	std::unique_ptr<GameCamera> m_camera;
	bool m_isCameraActive = false;
	// モデル
	std::unique_ptr<DirectX::Model> m_model;
	// モデルの位置
	DirectX::SimpleMath::Vector3 m_gatePosition;
	// モデルの大きさ
	DirectX::SimpleMath::Vector3 m_gateScale;
	// 平行移動の行列
	DirectX::SimpleMath::Matrix m_gateTrans;
	// 初期位置Ｘ
	float m_initialX;
	// 回転角度
	float m_gateRotAngle;
	// 開閉フラグ
	bool m_isOpen = false;
	// 当たり判定
	AABB m_gateCollision;
};

