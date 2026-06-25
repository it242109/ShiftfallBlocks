//--------------------------------------------------------------------------------------
// File: Camera.h
//
// メインカメラクラス
//--------------------------------------------------------------------------------------

#pragma once
#include <SimpleMath.h>

class Camera
{
public:
	// ゲッター／セッター -------------------------------------------------------------------
	// --- カメラ位置・注視点  ---
	// 視点(カメラの位置)を取得する関数
	const DirectX::SimpleMath::Vector3& GetEyePosition() { return m_eyePos; }
	// 注目点(見つめる位置)を取得する関数
	const DirectX::SimpleMath::Vector3& GetTargetPosition() { return m_targetPos; }
	// カメラの位置とターゲットを一括で指定する関数
	void SetPositionTarget(const DirectX::SimpleMath::Vector3& eye, const DirectX::SimpleMath::Vector3& target);

	// --- カメラの追従・移動速度  ---
	// カメラの位置への距離の差分に対する割合（追従レート）を設定する関数
	void SetCameraEyeMoveRate(float rate) { m_eyeMoveRate = rate; }
	// 注視点への距離の差分に対する割合（追従レート）を設定する関数
	void SetCameraTargetMoveRate(float rate) { m_targetMoveRate = rate; }

public:
	// 関数 ---------------------------------------------------------------------------------
	// コンストラクタ
	Camera();

	// デストラクタ
	~Camera() {}

	// カメラの切り替え時の初期化関数
	void InitializeCamera();

private:
	// 定数 ---------------------------------------------------------------------------------
	static const float EYE_SPEED_RATE;		///< カメラの位置への距離の差分に対する割合
	static const float TARGET_SPEED_RATE;	///< 注視点への距離の差分に対する割合

private:
	// メンバ変数 ---------------------------------------------------------------------------
	// 視点の位置
	DirectX::SimpleMath::Vector3 m_eyePos;

	// 注目点の位置
	DirectX::SimpleMath::Vector3 m_targetPos;

	// カメラの位置への距離の差分に対する割合
	float m_eyeMoveRate;

	// 注視点への距離の差分に対する割合
	float m_targetMoveRate;

	// カメラの位置の初期化フラグ
	bool m_initFlag;
};

