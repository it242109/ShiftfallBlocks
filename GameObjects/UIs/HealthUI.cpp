//--------------------------------------------------------------------------------------
// File: HealthUI.cpp
//
// 体力を表示させるUIのクラス
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "HealthUI.h"

HealthUI healthUI;

// 表示位置
const int HealthUI::HEALTH_UI_X = 50;
const int HealthUI::HEALTH_UI_Y = 600;
// 大きさ
const float HealthUI::HEALTH_UI_SCALE_X = 1.0f;
const float HealthUI::HEALTH_UI_SCALE_Y = 1.0f;
// 最大数
const int HealthUI::HEALTH_UI_MAX = 5;
// 表示間隔
const float HealthUI::HEALTH_UI_RANGE = 90.0f;

/*
* @brief コンストラクタ
*
* @param[in]  なし
* 
* @return     なし
*/
HealthUI::HealthUI()
    : m_player{}
    , m_windowHeight(0)
    , m_windowWidth(0)
    , m_pDR(nullptr)
    , m_state(STATE::IDOLE)
{
    m_health.clear();
}

/*
* @brief デストラクタ
*
* @param[in]  なし
* 
* @return     なし
*/
HealthUI::~HealthUI()
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
void HealthUI::Initialize(DX::DeviceResources* pDR, int width, int height)
{
    m_pDR = pDR;
    m_windowWidth = width;
    m_windowHeight = height;

    // プレイヤーの参照を渡す
    healthUI.SetPlayer(m_player); 

    for (int i = 0; i < HEALTH_UI_MAX; i++)
    {
        Add(L"Resources/Textures/health.png"
            , DirectX::SimpleMath::Vector2(HEALTH_UI_X + HEALTH_UI_RANGE * i, HEALTH_UI_Y)
            , DirectX::SimpleMath::Vector2(HEALTH_UI_SCALE_X, HEALTH_UI_SCALE_Y)
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
void HealthUI::Update()
{
    if (m_player)
    {
        int currentLives = m_player->GetLives();
        int UIlives = static_cast<int>(m_health.size());

        if (currentLives < UIlives)
        {
            // ダメージ処理
            m_state = STATE::DAMAGE;
        }
        else if (currentLives > UIlives)
        {
            // 回復処理
            m_state = STATE::RECOVERY;
        }
    }

    //  ライフ減少時
    if (m_state & STATE::DAMAGE)
    {
        Decrease();
    }
    //  ライフ回復時
    if (m_state & STATE::RECOVERY)
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
void HealthUI::Render()
{
    if (m_health.empty())
        return;

    if (!(m_state & STATE::DAMAGE))
    {
        m_health[0]->Render();
    }
    for (int i = 1; i < m_health.size(); i++)
    {
        m_health[i]->Render();
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
void HealthUI::Add(const wchar_t* path, DirectX::SimpleMath::Vector2 position,
	DirectX::SimpleMath::Vector2 scale, ANCHOR anchor)
{
    std::unique_ptr<UserInterface> userInterface = std::make_unique<UserInterface>();
    userInterface->Create(m_pDR
        , path
        , position
        , scale
        , anchor);
    userInterface->SetWindowSize(m_windowWidth, m_windowHeight);

    m_health.push_back(std::move(userInterface));
}

/*
* @brief プレイヤーのセッター
*
* @param[in]  player  プレイヤーのポインタ
* 
* @return     なし
*/
void HealthUI::SetPlayer(Player* player)
{
    m_player = player;
}

/*
* @brief 残機の取得
*
* @param[in]  なし
* 
* @return     最大値を返す
*/
int HealthUI::GetHealthMax() const
{
    if(m_player)
        return m_player->GetLives();
    return HEALTH_UI_MAX;
}

/*
* @brief　一つ増やす処理
*
* @param[in]  なし
* 
* @return     なし
*/
void HealthUI::Increase()
{
    if (!m_player)
        return;

    int currentLives = m_player->GetLives();
    int UIlives = static_cast<int>(m_health.size());

    if (UIlives >= currentLives)
    {
        m_state ^= STATE::RECOVERY;
        return;
    }

    Add(L"Resources/Textures/health.png"
        , DirectX::SimpleMath::Vector2(HEALTH_UI_X + HEALTH_UI_RANGE * m_health.size(), HEALTH_UI_Y)
        , DirectX::SimpleMath::Vector2(HEALTH_UI_SCALE_X, HEALTH_UI_SCALE_Y)
        , ANCHOR::TOP_LEFT);

	m_state ^= STATE::RECOVERY;
}

/*
* @brief 一つ減らす処理
*
* @param[in]  なし
* 
* @return     なし
*/
void HealthUI::Decrease()
{
    if (m_health.empty())
    {
        m_state ^= STATE::DAMAGE;
        return;
    }

    // 一番端を削除
    m_health.pop_back();
    m_state ^= STATE::DAMAGE;
}