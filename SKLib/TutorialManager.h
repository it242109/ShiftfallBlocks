//--------------------------------------------------------------------------------------
// File: TutorialManager.h
//
// チュートリアルの管理クラス
//--------------------------------------------------------------------------------------
#pragma once
class TutorialManager
{
public:
	// 列挙体の管理 -------------------------------------------------------------------------
	enum class TutorialState
	{
		NONE,		///< デフォルト
		MOVEMENT,	///< 移動操作
		MOUSE,		///< マウス操作
		LIFTANDDROP,///< 持ち上げ／設置操作
		SWITCH,		///< スイッチについての説明
		ITEM,		///< アイテムについての説明
		ATTACK,		///< 攻撃操作
		FINISHED	///< チュートリアル終わり
	};
public:
	// ゲッター／セッター -------------------------------------------------------------------
	// --- チュートリアル状態・進行 ---
	// 現在のチュートリアル状態を取得
	TutorialState GetState() const { return m_state; }
	// チュートリアル状態を設定
	void SetState(TutorialManager::TutorialState state);

	// --- アクティブ状態 ---
	// チュートリアルが現在実行中（アクティブ）かどうか
	bool IsActive() const { return m_active; }

public:
	// 関数 ---------------------------------------------------------------------------------
	// コンストラクタ
	TutorialManager();
	// 更新処理
	void Update() const;
	// プレイヤーがロックされているかどうか
	bool IsPlayerLocked() const;
	// ステップ開始
	void Start();
	// 今のステップが終わった時の処理
	void CompleteStep();
	// 次のステップへ進める
	void NextStep();

private:
	// メンバ変数 ---------------------------------------------------------------------------
	// チュートリアルの状態
	TutorialState m_state = TutorialState::NONE;
	// チュートリアルがアクティブかどうか
	bool m_active;
	// 終わったかどうか
	bool m_stepFinished;
};

