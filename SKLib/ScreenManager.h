//--------------------------------------------------------------------------------------
// File: ScreenManager.h
//
// スクリーンの状態を管理するクラス
//--------------------------------------------------------------------------------------

#pragma once
#include "DeviceResources.h"
class ScreenManager
{
public:
    // 基準解像度
    static constexpr float BaseWidth = 1280.0f;
    static constexpr float BaseHeight = 720.0f;

    // デバイスリソース
    static void SetDeviceResources(DX::DeviceResources* dr) 
    {
        GetDRInternal() = dr;
    }

    // 位置を現在の画面比率に変換
    static DirectX::SimpleMath::Vector2 Pos(float x, float y);

    // スケールを現在の画面比率に変換
    static DirectX::SimpleMath::Vector2 Scale(float sx, float sy);

private:
    // デバイスリソースへの参照を取得
    static DX::DeviceResources*& GetDRInternal();

};


