//--------------------------------------------------------------------------------------
// File: StaminaUI.cpp
//
// スタミナUIのクラス
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "StaminaUI.h"

// インスタンス
StaminaUI staminaUI;

// 表示位置
const int StaminaUI::STAMINA_UI_X = 50;
const int StaminaUI::STAMINA_UI_Y = 500;
// 大きさ
const float StaminaUI::STAMINA_UI_SCALE_X = 1.0f;
const float StaminaUI::STAMINA_UI_SCALE_Y = 1.0f;
// 最大数
const int StaminaUI::STAMINA_UI_MAX = 5;
// 表示間隔
const float StaminaUI::STAMINA_UI_RANGE = 70.0f;
// スタミナが０の場合のしきい値
const float StaminaUI::STAMINA_UI_ZERO_THRESHOLD = 0.01f;
// UIアイコン1つあたりに対応するスタミナ量
const float StaminaUI::STAMINA_UI_PER_VALUE = 10.0f;

/*
* @brief コンストラクタ
*
* @param[in]  なし
* 
* @return     なし
*/
StaminaUI::StaminaUI()
    : m_player{}
    , m_windowHeight(0)
    , m_windowWidth(0)
    , m_pDR(nullptr)
    , m_state(STATE::IDOLE)
{
    m_stamina.clear();
}

/*
* @brief デストラクタ
*
* @param[in]  なし
* 
* @return     なし
*/
StaminaUI::~StaminaUI()
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
void StaminaUI::Initialize(DX::DeviceResources* pDR, int width, int height)
{
    m_pDR = pDR;
    m_windowWidth = width;
    m_windowHeight = height;

    // プレイヤーの参照を渡す
    staminaUI.SetPlayer(m_player); 

    for (int i = 0; i < STAMINA_UI_MAX; i++)
    {
        Add(L"Resources/Textures/staminaUI.png"
            , DirectX::SimpleMath::Vector2(STAMINA_UI_X + STAMINA_UI_RANGE * i, STAMINA_UI_Y)
            , DirectX::SimpleMath::Vector2(STAMINA_UI_SCALE_X, STAMINA_UI_SCALE_Y)
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
void StaminaUI::Update()
{
    if (m_player)
    {
        // プレイヤーの現在のスタミナを取得
        float currentStamina = m_player->GetStamina();

        int targetUILives = 0;

        if (currentStamina > STAMINA_UI_ZERO_THRESHOLD)
        {
            targetUILives = static_cast<int>(ceilf(currentStamina / STAMINA_UI_PER_VALUE));
        }

        // UIのアイコン数
        int UILives = static_cast<int>(m_stamina.size());

        if (targetUILives < UILives)
        {
            m_state = STATE::DECREASE;
        }
        else if (targetUILives > UILives)
        {
            m_state = STATE::INCREASE;
        }
    }

    //  減少時
    if (m_state & STATE::DECREASE)
    {
        Decrease();
    }
    //  ライフ回復時
    if (m_state & STATE::INCREASE)
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
void StaminaUI::Render()
{
    if (m_stamina.empty())
        return;

    if (!(m_state & STATE::DECREASE))
    {
        m_stamina[0]->Render();
    }
    for (int i = 1; i < m_stamina.size(); i++)
    {
        m_stamina[i]->Render();
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
void StaminaUI::Add(const wchar_t* path, DirectX::SimpleMath::Vector2 position,
	DirectX::SimpleMath::Vector2 scale, ANCHOR anchor)
{
    std::unique_ptr<UserInterface> userInterface = std::make_unique<UserInterface>();
    userInterface->Create(m_pDR
        , path
        , position
        , scale
        , anchor);
    userInterface->SetWindowSize(m_windowWidth, m_windowHeight);

    m_stamina.push_back(std::move(userInterface));
}

/*
* @brief プレイヤーのセッター
*
* @param[in]  player  プレイヤーのポインタ
* 
* @return     なし
*/
void StaminaUI::SetPlayer(Player* player)
{
    m_player = player;
}

/*
* @brief スタミナの取得
*
* @param[in]  なし
* 
* @return     最大値を返す
*/
int StaminaUI::GetStaminaMax() const
{
    if (m_player)
        return m_player->GetLives();
    return STAMINA_UI_MAX;
}

/*
* @brief　一つ増やす処理
*
* @param[in]  なし
* 
* @return     なし
*/
void StaminaUI::Increase()
{
    if (!m_player)
        return;

    // ターゲットとなるUIの数を再計算
    float currentStamina = m_player->GetStamina();
    int targetUILives = 0;
    if (currentStamina > STAMINA_UI_ZERO_THRESHOLD)
    {
        targetUILives = static_cast<int>(ceilf(currentStamina / STAMINA_UI_PER_VALUE));
    }
    int UIlives = static_cast<int>(m_stamina.size());

    if (UIlives >= targetUILives)
    {
        m_state ^= STATE::INCREASE;
        return;
    }

    Add(L"Resources/Textures/staminaUI.png"
        , DirectX::SimpleMath::Vector2(STAMINA_UI_X + STAMINA_UI_RANGE * m_stamina.size(), STAMINA_UI_Y)
        , DirectX::SimpleMath::Vector2(STAMINA_UI_SCALE_X, STAMINA_UI_SCALE_Y)
        , ANCHOR::TOP_LEFT);

	m_state ^= STATE::INCREASE;
}

/*
* @brief 一つ減らす処理
*
* @param[in]  なし
* 
* @return     なし
*/
void StaminaUI::Decrease()
{
    if (m_stamina.empty())
    {
        m_state ^= STATE::DECREASE;
        return;
    }

    // 一番端を削除
    m_stamina.pop_back();
    m_state ^= STATE::DECREASE;
}