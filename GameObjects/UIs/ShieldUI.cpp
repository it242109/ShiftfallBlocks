//--------------------------------------------------------------------------------------
// File: ShieldUI.cpp
//
// 盾の残り回数を表示させるUIのクラス
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "ShieldUI.h"

ShieldUI shieldUI;

// 表示位置
const int ShieldUI::SHIELD_UI_X = 1100;
const int ShieldUI::SHIELD_UI_Y = 600;

// 最大数
const int ShieldUI::SHIELD_UI_MAX = 3;

// 表示間隔
const float ShieldUI::SHIELD_UI_RANGE = 90.0f;

/*
* @brief コンストラクタ
*
* @param[in]  なし
* 
* @return     なし
*/
ShieldUI::ShieldUI()
    : m_player{}
    , m_windowHeight(0)
    , m_windowWidth(0)
    , m_pDR(nullptr)
    , m_state(STATE::IDOLE)
{
    m_shield_UI.clear();
}

/*
* @brief デストラクタ
*
* @param[in]  なし
* 
* @return     なし
*/
ShieldUI::~ShieldUI()
{
}

/*
* @brief 初期化処理
*
* @param[in]  pDR　   デバイスリソース
* @param[in]  width　 画面幅
* @param[in]  height  画面の高さ
* 
* @return     なし
*/
void ShieldUI::Initialize(DX::DeviceResources* pDR, int width, int height)
{
    m_pDR = pDR;
    m_windowWidth = width;
    m_windowHeight = height;

    shieldUI.SetPlayer(m_player);

    for (int i = 0; i < SHIELD_UI_MAX; i++)
    {
        Add(L"Resources/Textures/shieldUI.png"
            , DirectX::SimpleMath::Vector2(SHIELD_UI_X - SHIELD_UI_RANGE * i, SHIELD_UI_Y)
            , DirectX::SimpleMath::Vector2(1.0f, 1.0f)
            , ANCHOR::TOP_LEFT);
    }
}

/*
* @brief 更新処理
*
* @param[in]  なし
* 
* @return     なし
*/
void ShieldUI::Update()
{
    if (m_player)
    {
        int currentShieldCount = m_player->GetDefenseCount();
        int UIshield = static_cast<int> (m_shield_UI.size());

        if (currentShieldCount < UIshield)
        {
            m_state = STATE::USED;
        }
        else if (currentShieldCount > UIshield)
        {
            m_state = STATE::INCREASED;
        }
    }

    // 一つ減らしたとき
    if (m_state & STATE::USED)
    {
        Decrease();
    }
    // 一つ増やしたとき
    if (m_state & STATE::INCREASED)
    {
        Increase();
    }
}

/*
* @brief 描画処理
*
* @param[in]  なし
* 
* @return     なし
*/
void ShieldUI::Render()
{
    if (m_shield_UI.empty())
        return;

    if (!(m_state & STATE::USED))
    {
        m_shield_UI[0]->Render();
    }
    for (int i = 1; i < m_shield_UI.size(); i++)
    {
        m_shield_UI[i]->Render();
    }
}

/*
* @brief 追加処理
*
* @param[in]  path      画像のパス
* @param[in]  position  アンカー座標
* @param[in]  scale     元の画像に対するスケール
* @param[in]  anchor    アンカー
* 
* @return     なし
*/
void ShieldUI::Add(const wchar_t* path, DirectX::SimpleMath::Vector2 position,
	DirectX::SimpleMath::Vector2 scale, ANCHOR anchor)
{
    std::unique_ptr<UserInterface> userInterface = std::make_unique<UserInterface>();
    userInterface->Create(m_pDR
        , path
        , position
        , scale
        , anchor);
    userInterface->SetWindowSize(m_windowWidth, m_windowHeight);

    m_shield_UI.push_back(std::move(userInterface));
}

/*
* @brief プレイヤーのセッター
*
* @param[in]  player  プレイヤーのポインタ
* 
* @return     なし
*/
void ShieldUI::SetPlayer(Player* player)
{
    m_player = player;
}

/*
* @brief 残りの数を取得
*
* @param[in]  なし
* 
* @return     最大値を返す
*/
int ShieldUI::GetShieldMax() const
{
    if (m_player)
        return m_player->GetDefenseCount();
    return SHIELD_UI_MAX;
}

/*
* @brief一つ増やす処理
*
* @param[in]  なし
* 
* @return     なし
*/
void ShieldUI::Increase()
{
    if (!m_player)
        return;

    int currentShieldCount = m_player->GetDefenseCount();
    int UIshield = static_cast<int> (m_shield_UI.size());

    if (UIshield >= currentShieldCount)
    {
        m_state ^= STATE::INCREASED;
        return;
    }

    Add(L"Resources/Textures/shieldUI.png"
        , DirectX::SimpleMath::Vector2(SHIELD_UI_X - SHIELD_UI_RANGE * m_shield_UI.size(), SHIELD_UI_Y)
        , DirectX::SimpleMath::Vector2(1.0f, 1.0f)
        , ANCHOR::TOP_LEFT);

    m_state ^= STATE::INCREASED;
}

/*
* @brief 一つ減らす処理
*
* @param[in]  なし
* 
* @return     なし
*/
void ShieldUI::Decrease()
{
    if (m_shield_UI.empty())
    {
        m_state ^= STATE::USED;
        return;
    }

    // 一番端を削除
    m_shield_UI.pop_back();
    m_state ^= STATE::USED;
}