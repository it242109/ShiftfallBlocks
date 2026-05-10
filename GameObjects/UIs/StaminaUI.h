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

#include "GameObjects/Stages/Player.h"

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

private:
	Player* m_player;

	// 定数
	const static int STAMINA_X;
	const static int STAMINA_Y;
	const static int STAMINA_MAX;
	const static float STAMINA_RANGE;
	const float STAMINA_ZERO_THRESHOLD = 0.01f;

	DX::DeviceResources* m_pDR;
	std::vector <std::unique_ptr<UserInterface>> m_stamina;

	// ウィンドウサイズ
	int m_windowWidth, m_windowHeight;

	// 状態管理用のビットフラグ
	byte m_state;

public:
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

	// プレイヤーのセッターを追加
	void SetPlayer(Player* player);
	// 最大値を取得
	int GetStaminaMax() const;

	// 増やす処理
	void Increase();
	// 減らす処理
	void Decrease();
};

