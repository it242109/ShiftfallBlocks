//--------------------------------------------------------------------------------------
// File: InputManager.cpp
//
// キーの共通の入力管理クラス
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "InputManager.h"

/*
* @brief シングルトンインスタンスの取得
*
* @param[in] なし
*
* @return なし
*/
InputManager& InputManager::Get()
{
	static InputManager instance;
	return instance;
}

/*
* @brief 更新処理
*
* @param[in] なし
*
* @return なし
*/
void InputManager::Update()
{
	m_prevState = m_currentState;
	m_currentState = DirectX::Keyboard::Get().GetState();
	m_prevMouseState = m_mouseState;
	m_mouseState = DirectX::Mouse::Get().GetState();
}

//////////////////////////////////キーボードの処理/////////////////////////////////
/*
* @brief キーが押されたら
*
* @param[in] key DirectX::Keyboard::Keys列挙体を指定
*
* @return　キーが押されたらtrue、そうでなければfalse
*/
bool InputManager::IsKeyPressed(DirectX::Keyboard::Keys key) const
{
	return m_currentState.IsKeyDown(key) && !m_prevState.IsKeyDown(key);
}
/*
* @brief キーが離されたら
*
* @param[in] key DirectX::Keyboard::Keys列挙体を指定
*
* @return　キーが離されたらtrue、そうでなければfalse
*/
bool InputManager::IsKeyReleased(DirectX::Keyboard::Keys key) const
{
	return !m_currentState.IsKeyDown(key) && m_prevState.IsKeyDown(key);
} 
/*
* @brief キーが押されている間
*
* @param[in] key DirectX::Keyboard::Keys列挙体を指定
*
* @return キーが押されている間はtrue、そうでなければfalse
*/
bool InputManager::IsKeyDown(DirectX::Keyboard::Keys key) const
{
	return m_currentState.IsKeyDown(key);
}

//////////////////////////////////マウスの処理/////////////////////////////////////
/*
* @brief マウスの左ボタンが押されたら
*
* @param[in] なし
*
* @return　ボタンが押されたらtrue、そうでなければfalse
*/
bool InputManager::IsMousePressedLeft() const
{
	return m_mouseState.leftButton && !m_prevMouseState.leftButton;
} 
/*
* @brief マウスの右ボタンが押されたら
*
* @param[in] なし
*
* @return　ボタンが押されたらtrue、そうでなければfalse
*/
bool InputManager::IsMousePressedRight() const
{
	return m_mouseState.rightButton && !m_prevMouseState.rightButton;
}
/*
* @brief マウスの左ボタンが離されたら
*
* @param[in] なし
*
* @return　ボタンが離されたらtrue、そうでなければfalse
*/
bool InputManager::IsMouseReleasedLeft() const
{
	return !m_mouseState.leftButton && m_prevMouseState.leftButton;
}
/*
* @brief マウスの右ボタンが離されたら
*
* @param[in] なし
*
* @return　ボタンが離されたらtrue、そうでなければfalse
*/
bool InputManager::IsMouseReleasedRight() const
{
	return !m_mouseState.rightButton && m_prevMouseState.rightButton;
}
/*
* @brief マウスの左ボタンが押されている間
*
* @param[in] なし
*
* @return　ボタンが押されている間ならtrue、そうでなければfalse
*/
bool InputManager::IsMouseDownLeft() const
{
	return m_mouseState.leftButton;
}
/*
* @brief マウスの右ボタンが押されている間
*
* @param[in] なし
*
* @return　ボタンが押されている間ならtrue、そうでなければfalse
*/bool InputManager::IsMouseDownRight() const
{
	return m_mouseState.rightButton;
}
/*
* @brief 入力情報をリセット
*
* @param[in] なし
*
* @return　なし
*/
void InputManager::ResetInputState()
{
	m_currentState = DirectX::Keyboard::State();
	m_mouseState = DirectX::Mouse::State();

	m_prevState = m_currentState;
	m_prevMouseState = m_mouseState;
}
