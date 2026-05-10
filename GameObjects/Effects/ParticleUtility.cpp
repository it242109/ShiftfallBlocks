//--------------------------------------------------------------------------------------
// File: ParticleUtility.cpp
//
// パーティクルを管理するクラス
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "ParticleUtility.h"

#include "StepTimer.h"
#include "SKLib/DeviceResources.h"
#include <Effects.h>
#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include <WICTextureLoader.h>
#include <CommonStates.h>
#include <vector>

/*
* @brief コンストラクタ
*
* @param[in]  life　　　　生存時間
* @param[in]  pos　　　　 座標
* @param[in]  velocity　　速度
* @param[in]  startScale　初期サイズ
* @param[in]  endScale　　最終サイズ
* @param[in]  startColor　初期色
* @param[in]  endColor　　最終色
* 
* @return     なし
*/
ParticleUtility::ParticleUtility(float life, DirectX::SimpleMath::Vector3 pos, DirectX::SimpleMath::Vector3 velocity,
	DirectX::SimpleMath::Vector3 accele, DirectX::SimpleMath::Vector3 startScale, DirectX::SimpleMath::Vector3 endScale, 
	DirectX::SimpleMath::Color startColor, DirectX::SimpleMath::Color endColor)
{
	//	与えられた生存時間を取得
	m_startLife =
		m_life = life;

	//	初期位置を取得
	m_position = pos;
	//	移動ベクトルを取得
	m_velocity = velocity;
	//	加速度ベクトルを取得
	m_accele = accele;

	//	初めのサイズを取得
	m_startScale =
		m_nowScale = startScale;
	//	終わりのサイズを取得
	m_endScale = endScale;

	//	初めの色を取得
	m_startColor =
		m_nowColor = startColor;
	//	終了の色を取得
	m_endColor = endColor;
}
/*
* @brief　デストラクタ
*
* @param[in]  なし
* 
* @return     なし
*/
ParticleUtility::~ParticleUtility()
{
}

/*
* @brief　更新処理
*
* @param[in]  timer　Game等からStepTimerを受け取る
* 
* @return     生存時間がある間はtrue
*/
bool ParticleUtility::Update(float elapsedTime)
{
	//	スケール
	m_nowScale = DirectX::SimpleMath::Vector3::Lerp(m_startScale, m_endScale, 1.0f - m_life / m_startLife);
	//	色
	m_nowColor = DirectX::SimpleMath::Color::Lerp(m_startColor, m_endColor, 1.0f - m_life / m_startLife);
	//	速度の計算
	m_velocity += m_accele * elapsedTime;
	//	座標の計算
	m_position += m_velocity * elapsedTime;
	//	ライフを減らしていく
	m_life -= elapsedTime;
	//	ライフが0未満なら消す
	if (m_life < 0.0f)
	{
		return false;
	}

	return true;
}
