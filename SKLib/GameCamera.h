//--------------------------------------------------------------------------------------
// File: GameCamera.h
//
// ゲームカメラ／カメラ衝突提供インターフェースクラスの定義
//--------------------------------------------------------------------------------------

#pragma once
#include "SimpleMath.h"
#include <algorithm>

// カメラ衝突提供インターフェース
class ICameraCollisionProvider
{
public:
	virtual ~ICameraCollisionProvider() = default;

	// レイを飛ばし最も近い衝突物体までの距離を返す
	virtual float GetClosestHitDistance(const DirectX::SimpleMath::Vector3& origin,
		const DirectX::SimpleMath::Vector3& direction,
		float maxDistance) const = 0;
};

// ゲームカメラクラス
class GameCamera
{
public:
	// ゲッター／セッター -------------------------------------------------------------------
	// --- カメラの基本行列・座標 ---
	// カメラの位置を取得する関数
	DirectX::SimpleMath::Vector3 GetEyePosition() const;
	// カメラの注視点を取得する関数
	DirectX::SimpleMath::Vector3 GetTargetPosition() const;
	// フリーモード時のカメラの注視点を設定する関数
	void SetTargetPosition(const DirectX::SimpleMath::Vector3& target);
	// カメラのビュー行列を取得する関数
	DirectX::SimpleMath::Matrix GetCameraMatrix() const;

	// --- カメラの角度・回転 ---
	// 現在の水平角度を取得する関数
	float GetHorizontalAngle() const { return m_cameraHorizontalAngle; }
	// 現在の垂直（Y軸）角度を取得する関数
	float GetYAngle() const { return m_yAngle; }
	// カメラの角度を設定する関数
	void SetAngle(float xAngle, float yAngle);

	// --- カメラの距離・入力 ---
	// 追従時のカメラの距離を設定する関数
	void SetDistance(float distance);
	// マウスのスクロールホイールの値を取得する関数
	float GetScrollDelta() const { return m_scrollData; }

	// --- 追従・フリーモード制御  ---
	// 追従／フリーモードの切替関数
	void SetFollowMode(bool enable);
	// 追従カメラの注視点を取得する関数
	DirectX::SimpleMath::Vector3 GetFollowTargetPosition() const;
	// カメラの追従（位置とターゲット）を一括設定する関数
	void SetFollowTarget(const DirectX::SimpleMath::Vector3& eye, const DirectX::SimpleMath::Vector3& target);

	// --- ウィンドウ・システム設定 ---
	// 画面サイズを取得する関数
	void GetWindowSize(int& windowWidth, int& windowHeight) const;
	// 画面サイズを設定する関数
	void SetWindowSize(int windowWidth, int windowHeight);

public:
	// 関数 ---------------------------------------------------------------------------------
	// コンストラクタ
	GameCamera(int windowWidth, int windowHeight);

	// 更新
	void Update(const DirectX::SimpleMath::Vector3& playerPos,
		ICameraCollisionProvider* collisionProvider);
private:
	void Motion(int x, int y);

private:
	// 定数 ---------------------------------------------------------------------------------
	static const float HALF_DIVIDER;		///< 画面中心を求めるための除算値
	static const float MOUSE_SENSITIVITY;	///< マウス感度
	static const float MOUSE_ROTATION_LIMIT;///< // 縦回転（ピッチ）を制限するための係数
	static const float SCROLL_SENSITIVITY;	///< マウスホイールのスクロール感度
	static const float MIN_SCROLL_RANGE;	///< 攻撃範囲（スクロール値）の最小値
	static const float MAX_SCROLL_RANGE;	///< 攻撃範囲（スクロール値）の最大値

	static const float FIXED_CAMERA_DISTANCE;	///< 非追従モード時のプレイヤーからの固定距離
	static const float FOLLOW_CAMERA_DISTANCE;	///< 追従モード時の基本カメラ距離
	static const float FOLLOW_CAMERA_HEIGHT;	///< 追従モード時の基本カメラの高さ
	static const float MIN_CAMERA_RAY_DISTANCE;	///< 壁に作られた際の最小カメラ距離

	static const DirectX::SimpleMath::Vector3 CAMERA_FOWARD;///< カメラを前方に向ける

	static const float TARGET_HEIGHT_OFFSET;	///< カメラが注視するプレイヤーの高さ
	static const float RAY_ORIGIN_HEIGHT_OFFSET;///< 壁判定用レイを発射するプレイヤーの高さ基準
	static const float WALL_SAFETY_BUFFER;		///< カメラが壁にめり込まないように手前に戻すバッファ距離

	static const float CAMERA_PITCH_LIMIT_RATIO;///< カメラの最大見上げ・見下ろし角を限界の手前に抑えるための制限比率

private:
	// メンバ変数 ---------------------------------------------------------------------------
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

	// マウストラッカー
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

};