//--------------------------------------------------------------------------------------
// File: TutorialManager.h
//
// チュートリアルの管理クラス
//--------------------------------------------------------------------------------------
#pragma once
class TutorialManager
{
public:
	// 列挙体の定義
	enum class TutorialState
	{
		NONE,
		MOVEMENT,
		MOUSE,
		LIFTANDDROP,
		SWITCH,
		ITEM,
		ATTACK,
		FINISHED
	};
public:
	// コンストラクタ
	TutorialManager();
	
	// 更新処理
	void Update() const;

	// 状態取得
	TutorialState GetState() const { return m_state; }
	bool IsActive() const { return m_active; }

	// セッター
	void SetState(TutorialManager::TutorialState state);

	// プレイヤーがロックされているかどうか
	bool IsPlayerLocked() const;
	
	// ステップ開始
	void Start();

	// 今のステップが終わった時の処理
	void CompleteStep();

	// 次のステップへ進める
	void NextStep();

private:
	// チュートリアルの状態
	TutorialState m_state = TutorialState::NONE;

	// チュートリアルがアクティブかどうか
	bool m_active;
	// 終わったかどうか
	bool m_stepFinished;
};

