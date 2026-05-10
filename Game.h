//
// Game.h
//

#pragma once

#include "SKLib/DeviceResources.h"
#include "StepTimer.h"

#include "SKLib/DebugFont.h"
#include "SKLib/SceneManager.h"
#include "SKLib/InputManager.h"
#include "SKLib/SoundManager.h"
#include "SKLib/RenderTexture.h"
#include "SKLib/TransitionMask.h"

#include "SKLib/UserResources.h"


// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game final : public DX::IDeviceNotify
{
public:

    Game() noexcept(false);
    ~Game() = default;

    Game(Game&&) = default;
    Game& operator= (Game&&) = default;

    Game(Game const&) = delete;
    Game& operator= (Game const&) = delete;

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // IDeviceNotify
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnDisplayChange();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize( int& width, int& height ) const noexcept;

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;
    
    //　フルスクリーン対応にする関数
    BOOL m_fullscreen;

    // デバッグフォント
    std::unique_ptr<DebugFont>              m_debugFont;
	std::unique_ptr<DirectX::CommonStates>  m_states;

    // サウンドエフェクト
    std::unique_ptr<DirectX::SoundEffect> m_soundEffect;

    // キーボードステートトラッカー
    DirectX::Keyboard::KeyboardStateTracker m_keyboardTracker;

    // マウスステートトラッカー
    DirectX::Mouse::ButtonStateTracker m_mouseTracker;

    // ユーザーリソース
    std::unique_ptr<UserResources> m_userResources;

	// シーンマネージャー
    std::unique_ptr<SceneManager<UserResources>> m_sceneManager;

    // トランジションマスクのポインタ
    std::unique_ptr<TransitionMask> m_transitionMask;

    // 画面遷移テクスチャのポインタ
    std::unique_ptr<DX::RenderTexture> m_transitionTexture;

public:
    void SetFullscreenState(BOOL value);
};
