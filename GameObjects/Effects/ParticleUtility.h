//--------------------------------------------------------------------------------------
// File: ParticleUtility.h
//
// パーティクルを管理するクラス
//--------------------------------------------------------------------------------------
#pragma once

#include "SKLib/DeviceResources.h"
#include <SimpleMath.h>
#include <vector>

class ParticleUtility
{
public:
	// ゲッター ------------------------------------------------------------
	// --- 物理・移動関連 ---
	// 現在の位置を取得
	const DirectX::SimpleMath::Vector3 GetPosition() const { return m_position; }
	// 現在の速度を取得
	const DirectX::SimpleMath::Vector3 GetVelocity() const { return m_velocity; }
	// 現在の加速度を取得
	const DirectX::SimpleMath::Vector3 GetAccele() const { return m_accele; }

	// --- スケール・大きさ関連 ---
	// 現在のスケールを取得
	const DirectX::SimpleMath::Vector3 GetNowScale() const { return m_nowScale; }
	// 初期（開始時）のスケールを取得
	const DirectX::SimpleMath::Vector3 GetStartScale() const { return m_startScale; }
	// 最終（終了時）のスケールを取得
	const DirectX::SimpleMath::Vector3 GetEndScale() const { return m_endScale; }

	// --- 寿命・生存時間関連 ---
	// 現在の残り寿命を取得
	const float GetLife() const { return m_life; }
	// 初期（開始時）の最大寿命を取得
	const float GetStartLife() const { return m_startLife; }

	// --- カラー・色関連 ---
	// 現在の色を取得
	const DirectX::SimpleMath::Color GetNowColor() const { return m_nowColor; }
	// 初期（開始時）の色を取得
	const DirectX::SimpleMath::Color GetStartColor() const { return m_startColor; }
	// 最終（終了時）の色を取得
	const DirectX::SimpleMath::Color GetEndColor() const { return m_endColor; }

public:
	// 関数 ------------------------------------------------------------
	//	コンストラクタ
	ParticleUtility(
		float life,
		DirectX::SimpleMath::Vector3 pos,
		DirectX::SimpleMath::Vector3 velocity,
		DirectX::SimpleMath::Vector3 accele,
		DirectX::SimpleMath::Vector3 startScale, DirectX::SimpleMath::Vector3 endScale,
		DirectX::SimpleMath::Color startColor, DirectX::SimpleMath::Color endColor);
	//	デストラクタ
	~ParticleUtility();

	//	更新
	bool Update(float elapsedTime);

private:
	// 定数 -----------------------------------------------------
	static const float FULL_PROGRESS;	///< 進捗率の最大値


private:
	// メンバ変数 -----------------------------------------------------
	// 座標
	DirectX::SimpleMath::Vector3 m_position;

	// 速度
	DirectX::SimpleMath::Vector3 m_velocity;
	// 加速度
	DirectX::SimpleMath::Vector3 m_accele;

	// スケール
	DirectX::SimpleMath::Vector3 m_nowScale;
	DirectX::SimpleMath::Vector3 m_startScale;
	DirectX::SimpleMath::Vector3 m_endScale;

	// 生存時間
	float m_life;
	float m_startLife;

	// カラー
	DirectX::SimpleMath::Color m_nowColor;
	DirectX::SimpleMath::Color m_startColor;
	DirectX::SimpleMath::Color m_endColor;


};

