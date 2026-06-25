//--------------------------------------------------------------------------------------
// File: Camera.cpp
//
// メインカメラクラス
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "Camera.h"

// 定数の定義
const float Camera::EYE_SPEED_RATE = 0.05f;		// カメラの位置への距離の差分に対する割合
const float Camera::TARGET_SPEED_RATE = 1.0f;	// 注視点への距離の差分に対する割合

// コンストラクタ
Camera::Camera()
	: m_initFlag(true)
	, m_eyeMoveRate(EYE_SPEED_RATE)
	, m_targetMoveRate(TARGET_SPEED_RATE)

{
}

// カメラの切り替え時の初期化関数
void Camera::InitializeCamera()
{
	m_initFlag = true;
}

// カメラの位置とターゲットの位置を設定する関数
void Camera::SetPositionTarget(const DirectX::SimpleMath::Vector3& eye, const DirectX::SimpleMath::Vector3& target)
{
	if (m_initFlag == true)
	{
		m_initFlag = false;
		m_eyePos = eye;
		m_targetPos = target;
		return;
	}

	// カメラの位置を移動
	m_eyePos += (eye - m_eyePos) * m_eyeMoveRate;

	// カメラのターゲットの位置を移動
	m_targetPos += (target - m_targetPos) * m_targetMoveRate;
}
