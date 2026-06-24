//--------------------------------------------------------------------------------------
// File: Menu.cpp
//
// メニューの管理クラス
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "Menu.h"
#include "SKLib/UserInterface.h"

#include "SKLib/BinaryFile.h"
#include "SKLib/DeviceResources.h"
#include <SimpleMath.h>
#include <Effects.h>
#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include <WICTextureLoader.h>
#include <CommonStates.h>
#include <vector>

// 定数の定義
const float Menu::RATE_EXPAND = 0.3f;  ///< 拡大する割合

/*
* @brief コンストラクタ
* 
* @param[in] なし
* 
* @return なし
*/
Menu::Menu()
	:
	m_menuIndex(0),
	m_windowHeight(0),
	m_windowWidth(0),
	m_pDR(nullptr),
	m_baseTexturePath(nullptr)
{
	m_userInterface.clear();
}

/*
* @brief デストラクタ
*
* @param[in] なし
*
* @return なし
*/
Menu::~Menu()
{
}

/*
* @brief 初期化処理
*
* @param[in] pDR デバイスリソースへのポインタ
* @param[in] width ウィンドウの幅
* @param[in] height ウィンドウの高さ
*
* @return なし
*/
void Menu::Initialize(DX::DeviceResources* pDR, int width, int height)
{
	m_pDR = pDR;
	m_windowWidth = width;
	m_windowHeight = height;

	m_baseTexturePath = L"Resources/Textures/window.png";
}

/*
* @brief 更新処理
*
* @param[in] なし
*
* @return なし
*/
void Menu::Update()
{
	if (m_pDR)
	{
		auto viewport = m_pDR->GetScreenViewport();
		m_windowWidth = static_cast<int>(viewport.Width);
		m_windowHeight = static_cast<int>(viewport.Height);
	}

	DirectX::Keyboard::State keyState = DirectX::Keyboard::Get().GetState();
	m_tracker.Update(keyState);

	// アイテムが存在しない場合は処理しない
	if (m_userInterface.empty())
	{
		return;
	}

	if (m_tracker.pressed.Down || m_tracker.pressed.S)
	{
		// 移動
		m_menuIndex = (m_menuIndex + 1) % static_cast<int>(m_userInterface.size());

	}
	if (m_tracker.pressed.Up || m_tracker.pressed.W)
	{
		m_menuIndex = (m_menuIndex + static_cast<int>(m_userInterface.size()) - 1)
			% static_cast<int>(m_userInterface.size());
	}

	// 各アイテムに表示する画像の初期サイズを設定
	for (int i = 0; i < m_userInterface.size(); i++)
	{
		m_base[i]->SetScale(m_base[i]->GetBaseScale());
		m_userInterface[i]->SetScale(m_userInterface[i]->GetBaseScale());
	}

	// 選択中の初期サイズ取得
	DirectX::SimpleMath::Vector2 select = m_userInterface[m_menuIndex]->GetBaseScale();

	// 選択状態とするための変化用サイズを算出
	DirectX::SimpleMath::Vector2 selectScale = DirectX::SimpleMath::Vector2::Lerp(
		m_userInterface[m_menuIndex]->GetBaseScale(),
		DirectX::SimpleMath::Vector2::One, 1.0f);

	// 拡大する割合を設定
	select += selectScale * RATE_EXPAND;

	// 算出後のサイズを現在のサイズとして設定
	m_userInterface[m_menuIndex]->SetScale(select);

	// 背景用のウィンドウ画像にも同じ割合の値を設定
	m_base[m_menuIndex]->SetScale(select);

}

/*
* @brief 描画処理
*
* @param[in] なし
*
* @return なし
*/
void Menu::Render()
{
	for (size_t i = 0; i < m_userInterface.size(); i++)
	{
		m_base[i]->Render();
	}

	for (size_t i = 0; i < m_userInterface.size(); ++i)
	{
#ifdef _DEBUG
		// デバッグログ: ボタンの位置とスケールを出力
		auto pos = m_userInterface[i]->GetPosition();
		auto scale = m_userInterface[i]->GetScale();
		OutputDebugString((L"Button " + std::to_wstring(i) + L": Pos(" +
			std::to_wstring(pos.x) + L", " + std::to_wstring(pos.y) +
			L"), Scale(" + std::to_wstring(scale.x) + L", " +
			std::to_wstring(scale.y) + L")\n").c_str());
#endif //_DEBUG

		m_userInterface[i]->Render();
	}
}

/*
* @brief メニューの追加
*
* @param[in] path		画像のパス
* @param[in] position	座標
* @param[in] scale		スケール
* @param[in] anchor		基準点
*
* @return なし
*/
void Menu::Add(const wchar_t* path, DirectX::SimpleMath::Vector2 position, DirectX::SimpleMath::Vector2 scale, ANCHOR anchor)
{
	// メニューとしてアイテムを追加
	std::unique_ptr<UserInterface> userInterface = std::make_unique<UserInterface>();

	// 指定した画像を表示するためのアイテムを作成
	userInterface->Create(m_pDR, path, position, scale, anchor);
	userInterface->SetWindowSize(m_windowWidth, m_windowHeight);

	// アイテムを新しく追加
	m_userInterface.push_back(std::move(userInterface));

	//  背景用のウィンドウ画像も追加する
	std::unique_ptr<UserInterface> base = std::make_unique<UserInterface>();
	base->Create(m_pDR
		, m_baseTexturePath
		, position
		, scale
		, anchor);
	base->SetWindowSize(m_windowWidth, m_windowHeight);

	//  背景用のアイテムも新しく追加する
	m_base.push_back(std::move(base));
}

void Menu::SetPosition(int index, DirectX::SimpleMath::Vector2 position)
{
	// 範囲外アクセス防止
	if (index < 0 || index >= m_userInterface.size())
	{
		MessageBox(NULL, L"範囲外アクセスされました！", L"エラー", MB_ICONHAND);
		return;
	}

	// 座標を更新
	m_userInterface[index]->SetPosition(position);

	// 背景ごと更新
	m_base[index]->SetPosition(position);
}
