//--------------------------------------------------------------------------------------
// File: GameCamera.h
//
// ゲームカメラクラス
//--------------------------------------------------------------------------------------

#pragma once
#include "SimpleMath.h"
#include <algorithm>

// カメラ衝突提供インターフェース
class ICameraCollisionProvider
{
public:
	virtual ~ICameraCollisionProvider() = default;

	// レイを飛ばし、最も近い衝突物体までの距離を返す
	virtual float GetClosestHitDistance(const DirectX::SimpleMath::Vector3& origin,
		const DirectX::SimpleMath::Vector3& direction,
		float maxDistance) const = 0;

};

// ゲームカメラクラス
class GameCamera
{
private:
	static constexpr float MOUSE_SENSITIVITY = 0.00095f;

private:
	// カメラの距離
	float m_cameraDistance;

	// 横回転
	float m_yAngle, m_yTmp;

	// 縦回転
	float m_xAngle, m_xTmp;

	// ドラッグされた座標
	int m_x, m_y;
	float m_sx, m_sy;

	// 生成されたビュー行列
	DirectX::SimpleMath::Matrix m_view;

	// スクロールホイール値
	int m_scrollWheelValue;

	//マウストラッカー
	DirectX::Mouse::ButtonStateTracker m_tracker;

	// スクリーンサイズ
	int m_screenW, m_screenH;

	// 追従モード関連変数
	bool m_followMode = false;
	DirectX::SimpleMath::Vector3 m_followEye;
	DirectX::SimpleMath::Vector3 m_followTarget;

	// 追従モードの水平角度
	float m_cameraHorizontalAngle;

	// スクロールホイールのデータ
	float m_scrollData = 5.0f;
	// 前回のスクロールホイールの値
	int m_lastWheelValue;

public:
	// 視点
	DirectX::SimpleMath::Vector3 m_eye;

	// 注視点
	DirectX::SimpleMath::Vector3 m_target;

private:
	void Motion(int x, int y);

public:
	// コンストラクタ
	GameCamera(int windowWidth, int windowHeight);

	//void Update();

	// 更新
	void Update(const DirectX::SimpleMath::Vector3& playerPos,
		ICameraCollisionProvider* collisionProvider);
	float GetHorizontalAngle() const { return m_cameraHorizontalAngle; }

	// カメラのビュー行列の取得関数
	DirectX::SimpleMath::Matrix GetCameraMatrix() const;

	// カメラの位置の取得関数
	DirectX::SimpleMath::Vector3 GetEyePosition() const;

	// カメラの注視点の取得関数
	DirectX::SimpleMath::Vector3 GetTargetPosition() const;

	// 追従カメラの注視点の取得関数
	DirectX::SimpleMath::Vector3 GetFollowTargetPosition() const;

	// 画面サイズの取得関数
	void GetWindowSize(int& windowWidth, int& windowHeight) const;

	// 現在の水平角度を取得する関数
	float GetYAngle() const { return m_yAngle; }

	float GetScrollDelta() const { return m_scrollData; }
	
	// 画面サイズの設定関数
	void SetWindowSize(int windowWidth, int windowHeight);

	// カメラの角度を設定する関数
	void SetAngle(float xAngle, float yAngle);

	// 追従時のカメラの距離を設定する関数
	void SetDistance(float distance);

	// カメラの追従を設定する関数
	void SetFollowTarget(const DirectX::SimpleMath::Vector3& eye, const DirectX::SimpleMath::Vector3& target);

	// 追従／フリーモードの切替関数
	void SetFollowMode(bool enable);

	// フリーモード時のカメラの距離を設定する関数
	void SetTargetPosition(const DirectX::SimpleMath::Vector3& target);
};

