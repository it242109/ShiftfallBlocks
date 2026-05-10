//--------------------------------------------------------------------------------------
// File: TitleCamera.cpp
//
// タイトルシーン用のカメラクラス
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "TitleCamera.h"

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
			*m_pPlayerPos + DirectX::SimpleMath::Vector3(0.0f, 1.0f, 0.0f);
		DirectX::SimpleMath::Vector3 eyePos(0.0f, 2.0f, -3.0f);
		eyePos = DirectX::SimpleMath::Vector3::Transform(eyePos, *m_pPlayerRotate);
		SetPositionTarget(targetPos + eyePos, targetPos);
	}
	break;
	case Type::Type_B:	// プレイヤーの周りを回るカメラ
	{
		// 1秒間に３０度回す
		m_angle += DirectX::XMConvertToRadians(30.0f) * elapsedTime;
		DirectX::SimpleMath::Vector3 targetPos =
			*m_pPlayerPos + DirectX::SimpleMath::Vector3(0.0f, 0.3f, 0.0f);
		DirectX::SimpleMath::Vector3 eyePos(0.0f, 2.0f, 6.0f);
		eyePos = DirectX::SimpleMath::Vector3::Transform(
			eyePos, DirectX::SimpleMath::Matrix::CreateRotationY(m_angle));
		SetPositionTarget(targetPos + eyePos, targetPos);
	}
	break;
	default:
		break;
	}
}