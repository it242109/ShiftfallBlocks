// --------------------------------------------------------------------------------------
// File: SwordUI.cpp
//
// 剣の残り回数を表示させるUIのクラス
//---------------------------------------------------------------------------------------
#include "pch.h"
#include "SwordUI.h"

SwordUI swordUI;

// 表示位置
const int SwordUI::SWORD_UI_X = 1100;
const int SwordUI::SWORD_UI_Y = 500;

// 最大数
const int SwordUI::SWORD_UI_MAX = 3;

// 表示間隔
const float SwordUI::SWORD_UI_RANGE = 90.0f;

/*
* @brief コンストラクタ
*
* @param[in]  なし
* 
* @return     なし
*/
SwordUI::SwordUI()
    : m_player{}
    , m_windowHeight(0)
    , m_windowWidth(0)
    , m_pDR(nullptr)
    , m_state(STATE::IDOLE)
{
    m_sword_UI.clear();
}

/*
* @brief デストラクタ
*
* @param[in]  なし
* 
* @return     なし
*/
SwordUI::~SwordUI()
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
void SwordUI::Initialize(DX::DeviceResources* pDR, int width, int height)
{
    m_pDR = pDR;
    m_windowWidth = width;
    m_windowHeight = height;

    swordUI.SetPlayer(m_player);

    for (int i = 0; i < SWORD_UI_MAX; i++)
    {
        Add(L"Resources/Textures/swordUI.png"
            , DirectX::SimpleMath::Vector2(SWORD_UI_X - SWORD_UI_RANGE * i, SWORD_UI_Y)
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
void SwordUI::Update()
{
    if (m_player)
    {
        int currentSwordCount = m_player->GetAttackCount();
        int UIsword = static_cast<int>(m_sword_UI.size());

        if (currentSwordCount < UIsword)
        {
            m_state = STATE::USED;
        }
        else if (currentSwordCount > UIsword)
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
void SwordUI::Render()
{
    if (m_sword_UI.empty())
        return;

    if (!(m_state & STATE::USED))
    {
        m_sword_UI[0]->Render();
    }
    for (int i = 1; i < m_sword_UI.size(); i++)
    {
        m_sword_UI[i]->Render();
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
void SwordUI::Add(const wchar_t* path, DirectX::SimpleMath::Vector2 position,
DirectX::SimpleMath::Vector2 scale, ANCHOR anchor)
{
    std::unique_ptr<UserInterface> userInterface = std::make_unique<UserInterface>();
    userInterface->Create(m_pDR
        , path
        , position
        , scale
        , anchor);
    userInterface->SetWindowSize(m_windowWidth, m_windowHeight);

    m_sword_UI.push_back(std::move(userInterface));
}

/*
* @brief プレイヤーのセッター
*
* @param[in]  player  プレイヤーのポインタ
* 
* @return     なし
*/
void SwordUI::SetPlayer(Player* player)
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
int SwordUI::GetSwordMax() const
{
    if (m_player)
        return m_player->GetAttackCount();
	return SWORD_UI_MAX;
}

/*
* @brief一つ増やす処理
*
* @param[in]  なし
* 
* @return     なし
*/
void SwordUI::Increase()
{
    if (!m_player)
        return;

    int currentSwordCount = m_player->GetAttackCount();
    int UIsword = static_cast<int>(m_sword_UI.size());

    if (UIsword >= currentSwordCount)
    {
        m_state ^= STATE::INCREASED;
        return;
    }

    Add(L"Resources/Textures/swordUI.png"
        , DirectX::SimpleMath::Vector2(SWORD_UI_X - SWORD_UI_RANGE * m_sword_UI.size(), SWORD_UI_Y)
        , DirectX::SimpleMath::Vector2(1.0f, 1.0f)
        , ANCHOR::TOP_LEFT);

	m_state ^= STATE::INCREASED;
}

/*
* @brief一つ減らす処理
*
* @param[in]  なし
* 
* @return     なし
*/
void SwordUI::Decrease()
{
    if (m_sword_UI.empty())
    {
        m_state ^= STATE::USED;
        return;
    }

    // 一番端を削除
    m_sword_UI.pop_back();
    m_state ^= STATE::USED;
}

