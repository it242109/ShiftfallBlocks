//--------------------------------------------------------------------------------------
// File: TutorialManager.cpp
//
// チュートリアルの管理クラス
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "TutorialManager.h"

/*
* @brief コンストラクタ
*
* @param[in] なし
*
* @return なし
*/
TutorialManager::TutorialManager()
	: 
	m_active(false),
	m_stepFinished(false)

{
}

/*
* @brief 更新処理
*
* @param[in] なし
*
* @return なし
*/
void TutorialManager::Update() const
{
	if (!m_active) return;
}

/*
* @brief セッター
*
* @param[in] state チュートリアルの状態
*
* @return なし
*/
void TutorialManager::SetState(TutorialManager::TutorialState state)
{
	m_active = true;
	m_state = state;
	m_stepFinished = false;
}

/*
* @brief プレイヤーがロックされているかどうか
*
* @param[in] なし
*
* @return プレイヤーがロックされている場合はtrue、そうでない場合はfalse
*/
bool TutorialManager::IsPlayerLocked() const
{
	if (!m_active) return false;

	// プレイヤーをロック
	switch (m_state)
	{
	case TutorialState::MOVEMENT:	// 移動操作
	case TutorialState::MOUSE:		// マウス操作
	case TutorialState::LIFTANDDROP:// 持ち上げ／設置操作
	case TutorialState::SWITCH:		// スイッチについての説明
	case TutorialState::ITEM:		// アイテムについての説明
	case TutorialState::ATTACK:		// 攻撃操作
		return true;
	default:
		return false;
	}
}

/*
* @brief ステップ開始
*
* @param[in] なし
*
* @return なし
*/
void TutorialManager::Start()
{
	m_active = true;
	m_state = TutorialState::MOVEMENT;
	m_stepFinished = false;
}

/*
* @brief 今のステップが終わった時の処理
*
* @param[in] なし
*
* @return なし
*/
void TutorialManager::CompleteStep()
{
	if (m_state == TutorialState::LIFTANDDROP || 
		m_state == TutorialState::SWITCH)
	{
		// チュートリアル終了
		m_active = false;      
		// 状態を終了へ
		m_state = TutorialState::FINISHED; 
	}
	else
	{
		// 次のステップへ進む
		m_stepFinished = true;
	}
}

/*
* @brief 次のステップへ進める
*
* @param[in] なし
*
* @return なし
*/
void TutorialManager::NextStep()
{
	if (!m_stepFinished)
	{
		// ステップが完了していない場合は次のステップに進まない
		switch (m_state)
		{
		case TutorialState::MOVEMENT:
			m_state = TutorialState::MOUSE;
			break;
		case TutorialState::MOUSE:
			m_state = TutorialState::FINISHED;
			m_active = false;
			break;
		case TutorialState::ITEM:
			m_state = TutorialState::ATTACK;
			break;
		case TutorialState::ATTACK:
			m_state = TutorialState::FINISHED;
			m_active = false;
			break;
		}

		// ステップが終わっていない場合は次のステップに進む
		m_stepFinished = false;
	}
}
