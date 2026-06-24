//--------------------------------------------------------------------------------------
// File: StaminaUI.h
//
// スタミナUIのクラス
//--------------------------------------------------------------------------------------
#pragma once

#include "SKLib/UserInterface.h"
#include "SKLib/DeviceResources.h"

#include <SimpleMath.h>
#include <Effects.h>
#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include <WICTextureLoader.h>
#include <CommonStates.h>
#include <vector>
#include "Keyboard.h"

#include "GameObjects/StageObjects/Player.h"

// 前方宣言
class Player;

class StaminaUI
{
public:
	// 状態
	enum STATE
	{
		IDOLE = 0,
		DECREASE = 1 << 0,
		INCREASE = 1 << 1,
	};

public:
	// ゲッター／セッター -------------------------------------------------------------------
	// --- システム・グラフィックス  ---
	// プレイヤーのセッター
	void SetPlayer(Player* player);
	// 最大値を取得
	int GetStaminaMax() const;

public:
	// 関数 ---------------------------------------------------------------------------------
	// コンストラクタ／デストラクタ
	StaminaUI();
	~StaminaUI();

	// 初期化処理
	void Initialize(DX::DeviceResources* pDR, int width, int height);
	// 更新処理
	void Update();
	// 描画処理
	void Render();

	// 追加処理
	void Add(const wchar_t* path,
		DirectX::SimpleMath::Vector2 position,
		DirectX::SimpleMath::Vector2 scale,
		ANCHOR anchor);

	// 増やす処理
	void Increase();
	// 減らす処理
	void Decrease();

private:
	// 定数 ------------------------------------------------------------------------
	static const int STAMINA_UI_X;					///< Xの位置
	static const int STAMINA_UI_Y;					///< Yの位置
	static const float STAMINA_UI_SCALE_X;			///< Xの大きさ
	static const float STAMINA_UI_SCALE_Y;			///< Yの大きさ
	static const int STAMINA_UI_MAX;				///< 最大数
	static const float STAMINA_UI_RANGE;			///< 表示間隔
	static const float STAMINA_UI_ZERO_THRESHOLD;	///<スタミナが０の場合のしきい値
	static const float STAMINA_UI_PER_VALUE;		///< UIアイコン1つあたりに対応するスタミナ量

private:
	// メンバ変数 ------------------------------------------------------------------
	// プレイヤーへのポインタ
	Player* m_player;

	DX::DeviceResources* m_pDR;
	std::vector <std::unique_ptr<UserInterface>> m_stamina;

	// ウィンドウサイズ
	int m_windowWidth, m_windowHeight;

	// 状態管理用のビットフラグ
	byte m_state;
};

