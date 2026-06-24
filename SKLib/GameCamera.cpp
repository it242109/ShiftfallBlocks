//--------------------------------------------------------------------------------------
// File: GameCamera.cpp
//
// ゲームカメラ／カメラ衝突提供インターフェースクラスの定義
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "GameCamera.h"
#include "Mouse.h"
#include <DirectXCollision.h>
#include "InputManager.h"

/*
* @brief コンストラクタ
* 
* @param windowWidth ウインドウサイズ（幅）
* @param windowHeight ウインドウサイズ（高さ）
* 
* @return なし
*/
GameCamera::GameCamera(int windowWidth, int windowHeight)
	:m_cameraDistance(6.0f),
	m_yAngle(0.0f),
	m_yTmp(0.0f),
	m_xAngle(0.0f),
	m_xTmp(0.0f),
	m_x(0),
	m_y(0),
	m_scrollWheelValue(0),
	m_screenW(windowWidth),
	m_screenH(windowHeight),
	m_cameraHorizontalAngle(0.0f),
	m_lastWheelValue(0)
	//m_isGimmickView(false)
{
	SetWindowSize(windowWidth, windowHeight);

	// マウスのホイール値をリセット
	DirectX::Mouse::Get().ResetScrollWheelValue();
}

/*
* @brief 更新
*
* @param playerPos			プレイヤーの位置
* @param collisionProvider	衝突判定プロバイダ
*
* @return なし
*/
void GameCamera::Update(const DirectX::SimpleMath::Vector3& playerPos, ICameraCollisionProvider* collisionProvider)
{
	// マウスの状態を取得／プレイヤー追従の処理
	auto& mouse = DirectX::Mouse::Get();
	auto state = mouse.GetState();

	m_followMode = true;

	// マウス入力・回転計算
	HWND hWnd = GetActiveWindow();
	if (hWnd && GetForegroundWindow() == hWnd && state.positionMode != DirectX::Mouse::MODE_RELATIVE)
	{
		float centerX = m_screenW / 2.0f;
		float centerY = m_screenH / 2.0f;

		// 回転角度の更新
		m_yAngle += ((float)state.x - centerX) * MOUSE_SENSITIVITY;
		m_xAngle += ((float)state.y - centerY) * MOUSE_SENSITIVITY;

		// 縦回転制限
		m_xAngle = std::max(-DirectX::XM_PIDIV2 * 0.01f, std::min(DirectX::XM_PIDIV2 * 0.01f, m_xAngle));

		// 角度正規化
		m_yAngle = fmodf(m_yAngle, DirectX::XM_2PI);
		if (m_yAngle < 0.0f) m_yAngle += DirectX::XM_2PI;

		// マウス中央固定
		POINT pt = { (long)centerX, (long)centerY };
		ClientToScreen(hWnd, &pt);
		SetCursorPos(pt.x, pt.y);
		while (ShowCursor(FALSE) >= 0);
	}

	// マウスホイールで攻撃範囲を変更する
	int currentWheel = state.scrollWheelValue;
	int wheelDelta = currentWheel - m_lastWheelValue;
	m_lastWheelValue = currentWheel;
	if (wheelDelta != 0) 
	{
		m_scrollData += (float)wheelDelta * 0.1f;
		m_scrollData = std::max(1.0f, std::min(20.0f, m_scrollData));
	}

	// 座標計算
	DirectX::SimpleMath::Vector3 up = { 0, 1, 0 };
	// プレイヤーからカメラまでの距離を固定
	const float fixedCameraDist = 5.0f;

	if (m_followMode)
	{
		// プレイヤーからの距離の設定
		const float cameraDistance = 5.0f;
		// プレイヤーからの高さの設定
		const float cameraHeight = 2.0;

		m_target = playerPos + DirectX::SimpleMath::Vector3(0, 1.2f, 0);
		// レイの開始点
		DirectX::SimpleMath::Vector3 rayOrigin = playerPos + DirectX::SimpleMath::Vector3(0, 1.5f, 0);

		// ベクトルの作成
		DirectX::SimpleMath::Vector3 baseOffset(0, cameraHeight, cameraDistance);

		// 回転を適用
		DirectX::SimpleMath::Matrix rot = DirectX::SimpleMath::Matrix::CreateRotationY(m_yAngle) * DirectX::SimpleMath::Matrix::CreateRotationX(m_xAngle);
		DirectX::SimpleMath::Vector3 rotatedOffset = DirectX::SimpleMath::Vector3::Transform(baseOffset, rot);
		
		DirectX::SimpleMath::Vector3 rayDir = rotatedOffset;
		float maxDist = rayDir.Length();
		rayDir.Normalize();

		// 衝突判定
		float hitDist = maxDist;
		if (collisionProvider)
		{
			hitDist = collisionProvider->GetClosestHitDistance(rayOrigin, rayDir, maxDist);
		}


		float finalDist = std::max(1.0f, hitDist - 0.3f);
		m_eye = rayOrigin + (rayDir * finalDist);
	}
	else
	{
		DirectX::SimpleMath::Matrix rot = DirectX::SimpleMath::Matrix::CreateRotationY(m_yAngle) * DirectX::SimpleMath::Matrix::CreateRotationX(m_xAngle);
		DirectX::SimpleMath::Vector3 forward = DirectX::SimpleMath::Vector3::Transform({ 0, 0, -1 }, rot);
		m_eye = m_target - (forward * fixedCameraDist);
	}

	m_view = DirectX::SimpleMath::Matrix::CreateLookAt(m_eye, m_target, up);
}

/*
* @brief 行列の生成
* 
* @param x マウスのX座標
* @param y マウスのY座標
*
* @return なし
*/
void GameCamera::Motion(int x, int y)
{
	// マウスポインタの位置のドラッグ開始位置からの変位（相対値）
	float dx = (x - m_x) * m_sx;
	float dy = (y - m_y) * m_sy;

	if (dx != 0.0f || dy != 0.0f)
	{
		// X軸の回転
		float xAngle = dy * DirectX::XM_PI;
		// Y軸の回転
		float yAngle = dx * DirectX::XM_PI;

		m_xTmp = m_xAngle + xAngle;
		m_yTmp = m_yAngle + yAngle;

		// 縦回転（X軸回転）のみ制限をかける
		m_xTmp = std::max(-DirectX::XM_PIDIV2 * 0.9f, std::min(DirectX::XM_PIDIV2 * 0.9f, m_xTmp));

	}
}

/*
* @brief カメラのビュー行列の取得関数
* 
* @param なし
* 
* @return ビュー行列
*/
DirectX::SimpleMath::Matrix GameCamera::GetCameraMatrix() const
{
	return m_view;
}

/*
* @brief カメラの位置の取得関数
* 
* @param なし
* 
* @return 視点の位置
*/
DirectX::SimpleMath::Vector3 GameCamera::GetEyePosition() const
{
	return m_eye;
}

/*
* @brief カメラの注視点の取得関数
* 
* @param なし
* 
* @return 注視点の位置
*/
DirectX::SimpleMath::Vector3 GameCamera::GetTargetPosition() const
{
	return m_target;
}

/*
* @brief 追従カメラの注視点の取得関数
* 
* @param なし
*
* @return 注視点の位置
*/
DirectX::SimpleMath::Vector3 GameCamera::GetFollowTargetPosition() const
{
	return m_followTarget;
}

/*
* @brief 画面サイズの設定関数
* 
* @param windowWidth ウインドウサイズ（幅）
* @param windowHeight ウインドウサイズ（高さ）
* 
* @return なし
*/
void GameCamera::SetWindowSize(int windowWidth, int windowHeight)
{
	// 画面サイズに対する相対的なスケールに調整
	m_sx = 1.0f / float(windowWidth);
	m_sy = 1.0f / float(windowHeight);
}

/*
* @brief 画面サイズの取得関数
* 
* @param windowWidth ウインドウサイズ（幅）への参照
* @param windowHeight ウインドウサイズ（高さ）への参照
* 
* @return なし
*/
void GameCamera::GetWindowSize(int& windowWidth, int& windowHeight) const
{
	windowWidth = m_screenW;
	windowHeight = m_screenH;
}

/*
* @brief カメラの角度を設定する関数
* 
* @param xAngle X軸の回転角度（縦回転）
* @param yAngle Y軸の回転角度（横回転）
* 
* @return なし
*/
void GameCamera::SetAngle(float xAngle, float yAngle)
{
	m_xAngle = xAngle;
	m_yAngle = yAngle;
	m_xTmp = xAngle;
	m_yTmp = yAngle;
}

/*
* @brief 追従時のカメラの距離を設定する関数
*
* @param distance カメラとターゲットの距離
*
* @return なし
*/
void GameCamera::SetDistance(float distance)
{
	m_cameraDistance = distance;
}

/*
* @brief カメラの追従を設定する関数
*
* @param eye 追従カメラの視点位置
* @param target 追従カメラの注視点位置
*
* @return なし
*/
void GameCamera::SetFollowTarget(const DirectX::SimpleMath::Vector3& eye, const DirectX::SimpleMath::Vector3& target)
{
	m_followEye = eye;
	m_followTarget = target;
}

/*
* @brief 追従／フリーモードの切替関数
*
* @param enable trueなら追従モード、falseならフリーモード
*
* @return なし
*/
void GameCamera::SetFollowMode(bool enable)
{
	m_followMode = enable;
}

/*
* @brief フリーモード時のカメラの距離を設定する関数
*
* @param target カメラの注視点位置
*
* @return なし
*/
void GameCamera::SetTargetPosition(const DirectX::SimpleMath::Vector3& target)
{
	m_target = target;
}
