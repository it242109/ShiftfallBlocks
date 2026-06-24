//--------------------------------------------------------------------------------------
// File: ScreenManager.cpp
//
// スクリーンの状態を管理するクラス
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "ScreenManager.h"

/*
* @brief　位置を現在の画面比率に変換
*
* @param[in]  x スクリーンのX座標
* @param[in]  y スクリーンのY座標
* 
* @return       スクリーンの位置情報
*/
DirectX::SimpleMath::Vector2 ScreenManager::Pos(float x, float y)
{
    auto dr = GetDRInternal();
    if (!dr) return { x, y };

    auto size = dr->GetOutputSize();
    float curW = (float)(size.right - size.left);

    // 横の倍率を基準に統一する
    float sw = curW / BaseWidth;
    return { x * sw, y * sw };
}

/*
* @brief　スケールを現在の画面比率に変換
*
* @param[in]  sx スクリーンのX座標
* @param[in]  sy スクリーンのY座標
* 
* @return       スクリーンの位置情報
*/
DirectX::SimpleMath::Vector2 ScreenManager::Scale(float sx, float sy)
{
    auto dr = GetDRInternal();
    if (!dr) return { sx, sy };

    auto size = dr->GetOutputSize();
    float sw = (float)(size.right - size.left) / BaseWidth;

    // スケールも横の倍率に合わせる
    return { sx * sw, sy * sw };
}

/*
* @brief　デバイスリソースへの参照を取得
*
* @param[in]  なし
* 
* @return     デバイスリソースへの参照
*/
DX::DeviceResources*& ScreenManager::GetDRInternal()
{
    static DX::DeviceResources* s_dr = nullptr;
    return s_dr;
}
