//--------------------------------------------------------------------------------------
// File: UserResources.h
//
// シーンへ渡すユーザーリソースクラス
//--------------------------------------------------------------------------------------
#pragma once

#include "StepTimer.h"
#include "DeviceResources.h"
#include "SKLib/DebugFont.h"
#include "TransitionMask.h"

class UserResources
{
public:
	// ゲッター／セッター -------------------------------------------------------------------
	// ステップタイマーを設定する関数
	void SetStepTimerStates(DX::StepTimer* timer) { m_timer = timer; }
	// ステップタイマーを取得する関数
	DX::StepTimer* GetStepTimer() { return m_timer; }

	// デバイスリソースを設定する関数
	void SetDeviceResources(DX::DeviceResources* deviceResources) { m_deviceResources = deviceResources; }
	// デバイスリソースを取得する関数
	DX::DeviceResources* GetDeviceResources() { return m_deviceResources; }

	// キーボードステートトラッカーを設定する関数
	void SetKeyboardTracker(DirectX::Keyboard::KeyboardStateTracker* tracker) { m_keyboardTracker = tracker; }
	// キーボードステートトラッカーを取得する関数
	DirectX::Keyboard::KeyboardStateTracker* GetKeyboardTracker() { return m_keyboardTracker; }

	// マウスステートトラッカーを設定する関数
	void SetMouseStateTracker(DirectX::Mouse::ButtonStateTracker* tracker) { m_mouseTracker = tracker; }
	// マウスステートトラッカーを取得する関数
	DirectX::Mouse::ButtonStateTracker* GetMouseTracker() { return m_mouseTracker; }

	// 共通ステートを設定する関数
	void SetCommonStates(DirectX::CommonStates* commonStates) { m_commonStates = commonStates; }
	// 共通ステートを取得する関数
	DirectX::CommonStates* GetCommonStates() { return m_commonStates; }

	// デバッグフォントを設定する関数
	void SetDebugFont(DebugFont* debugFont) { m_debugFont = debugFont; }
	// デバッグフォントを取得する関数
	DebugFont* GetDebugFont() { return m_debugFont; }

	// 画面遷移マスクを設定する関数
	void SetTransitionMask(TransitionMask* transitionMask) { m_transitionMask = transitionMask; }
	// 画面遷移マスクを取得する関数
	TransitionMask* GetTransitionMask() { return m_transitionMask; }

public:
	// 関数 ---------------------------------------------------------------------------------
	// コンストラクタ
	UserResources()
		: m_timer(nullptr)
		, m_deviceResources(nullptr)
		, m_keyboardTracker(nullptr)
		, m_mouseTracker(nullptr)
		, m_debugFont(nullptr)
		, m_commonStates(nullptr)
		, m_transitionMask(nullptr)
	{
	}
private:
	// メンバ変数 ---------------------------------------------------------------------------
	// ステップタイマー
	DX::StepTimer* m_timer;

	// デバイスリソース
	DX::DeviceResources* m_deviceResources;

	// キーボードステートトラッカー
	DirectX::Keyboard::KeyboardStateTracker* m_keyboardTracker;

	// マウスステートトラッカー
	DirectX::Mouse::ButtonStateTracker* m_mouseTracker;

	//デバッグ用文字列表示へのポインタ
	DebugFont* m_debugFont;

	// 共通ステート
	DirectX::CommonStates* m_commonStates;

	// 画面遷移マスク
	TransitionMask* m_transitionMask;

	// タイマー値
	float m_clearTime = 0.0f;


};