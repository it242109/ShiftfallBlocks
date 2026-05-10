//--------------------------------------------------------------------------------------
// File: Menu.h
//
// メニューの管理クラス
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

class Menu
{
public:
	unsigned int m_menuIndex;
	
private:
	// デバイスリソースへのポインタ
	DX::DeviceResources* m_pDR; 

	// メニューのUIオブジェクトのリスト
	std::vector<std::unique_ptr<UserInterface>> m_userInterface;
	std::vector<std::unique_ptr<UserInterface>> m_base;

	// メニューのテクスチャのパス
	const wchar_t* m_baseTexturePath;

	// メニューのベースウィンドウ
	std::unique_ptr<UserInterface> m_baseWindow;

	// ウィンドウサイズ
	int m_windowWidth, m_windowHeight;

	// キーボードの状態トラッカー
	DirectX::Keyboard::KeyboardStateTracker m_tracker;

public:
	// コンストラクタ／デストラクタ
	Menu();
	~Menu();

	// 初期化処理
	void Initialize(DX::DeviceResources* pDR, int width, int height);
	// 更新処理
	void Update();
	// 描画処理
	void Render();

	// メニューの追加
	void Add(const wchar_t* path,
		DirectX::SimpleMath::Vector2 position,
		DirectX::SimpleMath::Vector2 scale,
		ANCHOR anchor
	);

	// 位置の設定
	void SetPosition(int index, DirectX::SimpleMath::Vector2 position);
	// サイズの取得
	size_t GetSize() const { return m_userInterface.size(); }
};

