//--------------------------------------------------------------------------------------
// File: InputManager.h
//
// キーの共通の入力管理クラス
//--------------------------------------------------------------------------------------
#pragma once
#include "SimpleMath.h"
#include "Keyboard.h"
#include "Mouse.h"
class InputManager
{
public:
	// シングルトンとして使うためのインスタンスを取得
	static InputManager& Get();
	// 入力の状態を更新
	void Update();
	// キーが今押された瞬間か
	bool IsKeyPressed(DirectX::Keyboard::Keys key) const;
	// キーが今離された瞬間か
	bool IsKeyReleased(DirectX::Keyboard::Keys key) const;
	// キーが現在押されているか
	bool IsKeyDown(DirectX::Keyboard::Keys key) const;
	// マウスのボタンが今押された瞬間か
	bool IsMousePressedLeft() const;
	bool IsMousePressedRight() const;
	// マウスのボタンが今離された瞬間か
	bool IsMouseReleasedLeft() const;
	bool IsMouseReleasedRight() const;
	// マウスのボタンが現在押されているか
	bool IsMouseDownLeft() const;
	bool IsMouseDownRight() const;
	// 入力情報をリセット
	void ResetInputState();

private:
	InputManager() = default; // 外部から生成させない
	// キーボードとマウスの状態を保持
	DirectX::Keyboard::State m_currentState{};
	// 前回の状態を保持
	DirectX::Keyboard::State m_prevState{};
	// マウスのポインタ
	std::unique_ptr<DirectX::Mouse> m_mouse;
	// マウスの状態を保持
	DirectX::Mouse::State m_mouseState{};
	// マウスの前回の状態を保持
	DirectX::Mouse::State m_prevMouseState{};
};