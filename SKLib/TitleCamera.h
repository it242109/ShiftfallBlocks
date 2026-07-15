//--------------------------------------------------------------------------------------
// File: TitleCamera.h
//
// タイトルシーン用のカメラクラス
//--------------------------------------------------------------------------------------

#pragma once
#include "SKLib/Camera.h"
class TitleCamera : public Camera
{
public:
	enum class Type
	{
		Type_A,	//プレイヤーを追尾するカメラ
		Type_B	//プレイヤーの周りをまわるカメラ
	};

public:
	// ゲッター／セッター -----------------------------------------------------------------
	// プレイヤーの位置と回転を設定する関数
	void SetPlayer(
		const DirectX::SimpleMath::Vector3& position,
		const DirectX::SimpleMath::Quaternion& rotate
	)
	{
		m_pPlayerPos = &position;
		m_pPlayerRotate = &rotate;
	}

public:
	// 関数 ------------------------------------------------------------------------
	//コンストラクタ
	TitleCamera();
	// 更新関数
	void Update(float elapsedTime);

private:
	// 定数 ---------------------------------------------------------------------------------
	static const float TARGET_HEIGHT_OFFSET_TYPEA;	///< プレイヤーの注視点の高さオフセット(TYPE_A用）
	static const float CAMERA_OFFSET_HEIGHT_TYPEA;	///< プレイヤーから見たカメラの高さ
	static const float CAMERA_OFFSET_DIST_TYPEA;	///< プレイヤーから見たカメラの後方距離

	static const float ROTATION_SPEED_DEGRESS;		///< 1秒あたりの回転角度（度数法）
	static const float TARGET_HEIGHT_OFFSET_TYPEB;	///< プレイヤーの注視点の高さオフセット（TYPE_B用）
	static const float CAMERA_OFFSET_HEIGHT_TYPEB;	///< 旋回カメラの高さ
	static const float CAMERA_OFFSET_DIST_TYPEB;	///< 旋回カメラの回転半径（距離）
private:
	// メンバ変数 ------------------------------------------------------------------
	//現在のカメラタイプ
	Type m_type;
	//回転角（ラジアン）
	float m_angle;
	// プレイヤーの位置
	const DirectX::SimpleMath::Vector3* m_pPlayerPos;
	// プレイヤーの回転
	const DirectX::SimpleMath::Quaternion* m_pPlayerRotate;
	// ビュー行列
	DirectX::SimpleMath::Matrix m_view;
};

