//--------------------------------------------------------------------------------------
// File: TitleCamera.cpp
//
// タイトルシーン用のカメラクラス
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "TitleCamera.h"

// 定数の定義
const float TitleCamera::TARGET_HEIGHT_OFFSET_TYPEA = 1.0f;	///< プレイヤーの注視点の高さオフセット
const float TitleCamera::CAMERA_OFFSET_HEIGHT_TYPEA = 2.0f;	///< プレイヤーから見たカメラの高さ
const float TitleCamera::CAMERA_OFFSET_DIST_TYPEA= -3.0f;	///< プレイヤーから見たカメラの後方距離

const float TitleCamera::ROTATION_SPEED_DEGRESS = 30.0f;	///< 1秒あたりの回転角度（度数法）
const float TitleCamera::TARGET_HEIGHT_OFFSET_TYPEB = 0.3f; ///< プレイヤーの注視点の高さオフセット（TYPE_B用）
const float TitleCamera::CAMERA_OFFSET_HEIGHT_TYPEB = 2.0f;	///< 旋回カメラの高さ
const float TitleCamera::CAMERA_OFFSET_DIST_TYPEB = 6.0f;	///< 旋回カメラの回転半径（距離）

// コンストラクタ
TitleCamera::TitleCamera()
	: m_type(Type::Type_B),
	m_angle(0.0f),
	m_pPlayerPos{},
	m_pPlayerRotate{}
{
}

// 更新処理
void TitleCamera::Update(float elapsedTime)
{
	switch (m_type)
	{
	case Type::Type_A:	// プレイヤーの後ろから追いかけるカメラ
	{
		DirectX::SimpleMath::Vector3 targetPos =
			*m_pPlayerPos + DirectX::SimpleMath::Vector3(0.0f, TARGET_HEIGHT_OFFSET_TYPEA, 0.0f);
		DirectX::SimpleMath::Vector3 eyePos(0.0f, CAMERA_OFFSET_HEIGHT_TYPEA, CAMERA_OFFSET_DIST_TYPEA);
		eyePos = DirectX::SimpleMath::Vector3::Transform(eyePos, *m_pPlayerRotate);
		SetPositionTarget(targetPos + eyePos, targetPos);
	}
	break;
	case Type::Type_B:	// プレイヤーの周りを回るカメラ
	{
		// 1秒間に３０度回す
		m_angle += DirectX::XMConvertToRadians(ROTATION_SPEED_DEGRESS) * elapsedTime;
		DirectX::SimpleMath::Vector3 targetPos =
			*m_pPlayerPos + DirectX::SimpleMath::Vector3(0.0f, TARGET_HEIGHT_OFFSET_TYPEB, 0.0f);
		DirectX::SimpleMath::Vector3 eyePos(0.0f, CAMERA_OFFSET_HEIGHT_TYPEB, CAMERA_OFFSET_DIST_TYPEB);
		eyePos = DirectX::SimpleMath::Vector3::Transform(
			eyePos, DirectX::SimpleMath::Matrix::CreateRotationY(m_angle));
		SetPositionTarget(targetPos + eyePos, targetPos);
	}
	break;
	default:
		break;
	}
}