#include "pch.h"
#include "Number.h"

/*
* @brief コンストラクタ
*
* @param[in] pSpriteBatch スプライトバッチへのポインタ
* @param[in] pNumberSRV 数字のテクスチャSRVへのポインタ
* @param[in] digit      表示する桁数
* @param[in] scale      大きさ
*
* @return なし
*/
Number::Number(std::unique_ptr<DirectX::SpriteBatch>* pSpriteBatch,
	ID3D11ShaderResourceView** pNumberSRV,
	uint32_t digit,
	float scale)
: m_pSpriteBatch(pSpriteBatch)
, m_pNumberSRV(pNumberSRV)
, m_digit(digit)
, m_scale(scale)
, m_frame{}
{
	m_sprites = std::make_unique<SpriteSheet>();
	m_sprites->Load(*m_pNumberSRV, L"Resources/Textures/number.txt");

	for (size_t i = 0; i < 10; ++i)
	{
		std::wostringstream oss;
		oss << "number_b_" << i;

		m_frame[i] = m_sprites->Find(oss.str().c_str());
		assert(m_frame[i] != 0);
	}

	// 小数点を読み込む
	m_dotFrame = m_sprites->Find(L"number_b_dot");

    // コロンを読み込む
    m_colonFrame = m_sprites->Find(L"number_b_colon");
}

/*
* @brief デストラクタ
*
* @param[in] なし
*
* @return なし
*/
Number::~Number()
{
}

/*
* @brief 更新処理
*
* @param[in] elapsedTime 経過時間
*
* @return 常にtrueを返す
*/
bool Number::Update(float elapsedTime)
{
	UNREFERENCED_PARAMETER(elapsedTime);
	return true;
}

/*
* @brief 描画処理
*
* @param[in] なし
*
* @return なし
*/
void Number::Render()
{
    if (m_number.empty()) return;

    // 1280x720でのサイズで最終的な位置を計算
    DirectX::SimpleMath::Vector2 virtualPos = m_position;
    float w = m_frame[0]->size.x;

    // 元のスケールのまま計算
    virtualPos.x += m_frame[0]->origin.x * m_scale;
	virtualPos.y += m_frame[0]->origin.y * m_scale;

    // 右揃えの計算
    virtualPos.x += static_cast<float>(w) * m_scale * (m_digit - m_number.length());

    for (wchar_t c : m_number)
    {
        const SpriteSheet::SpriteFrame* frame = nullptr;

        if (c >= L'0' && c <= L'9')
            frame = m_frame[c - L'0'];
        else if (c == L'.')
            frame = m_dotFrame;
        else if (c == L':')
            frame = m_colonFrame;

        if (frame)
        {
            // 位置を変換
            DirectX::SimpleMath::Vector2 screenPos = ScreenManager::Pos(virtualPos.x, virtualPos.y);
            // スケールを変換
            DirectX::SimpleMath::Vector2 screenScale = ScreenManager::Scale(m_scale, m_scale);

            // 描画
            m_sprites->Draw((*m_pSpriteBatch).get(), *frame, screenPos, DirectX::Colors::White, 0.0f, screenScale.x);
        
            // 次の文字への送り量
            virtualPos.x += frame->size.x * m_scale;
        }
    }
}

/*
* @brief 整数を設定
*
* @param[in] number 表示する整数
*
* @return なし
*/
void Number::SetNumber(int number)
{
    m_number = std::to_wstring(number);
}

/*
* @brief 小数を設定
*
* @param[in] nunber 表示する浮動小数点数
* @param[in] precision 小数点以下の桁数
*
* @return なし
*/
void Number::SetNumberDecimal(float number, int precision)
{
    std::wostringstream oss;
    oss << std::fixed << std::setprecision(precision) << number;
    m_number = oss.str();
}

/*
* @brief タイマーを設定
*
* @param[in] totalSeconds　表示する時間（秒）
*
* @return なし
*/
void Number::SetTimer(int totalSeconds)
{
    if (totalSeconds < 0)
        totalSeconds = 0;

    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;

    std::wostringstream oss;
    oss << std::setw(2) << std::setfill(L'0') << minutes;
    oss << L":";
    oss << std::setw(2) << std::setfill(L'0') << seconds;

    m_number = oss.str();
}
