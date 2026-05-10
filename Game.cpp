//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include "Scenes/TitleScene.h"
#include "Scenes/TutorialScene.h"


extern void ExitGame() noexcept;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept(false)
    :m_fullscreen{}
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);

    // ƒŒƒ“ƒ_ƒŠƒ“ƒOƒeƒNƒXƒ`ƒƒ‚Ìì¬
    m_transitionTexture = std::make_unique<DX::RenderTexture>(m_deviceResources->GetBackBufferFormat());
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    // 60FPSŒÅ’è‚Éİ’è
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60.0);

    // ƒV[ƒ“ƒ}ƒl[ƒWƒƒ[‚Ìİ’è
    m_sceneManager->SetScene<TitleScene>();

    // BLANKi•‚Å“h‚è‚Â‚Ô‚·j
    auto context = m_deviceResources->GetD3DDeviceContext();
    context->ClearRenderTargetView(m_transitionTexture->GetRenderTargetView(), DirectX::Colors::Black);
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
        {
            Update(m_timer);
        });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());

    // TODO: Add your game logic here.
    elapsedTime;

    // ‰æ–Ê‘JˆÚƒ}ƒXƒN‚ÌXV
    m_transitionMask->Update(elapsedTime);

    // ƒV[ƒ“ƒ}ƒl[ƒWƒƒ[‚ÌXV
    m_sceneManager->Update(elapsedTime);

    // ƒTƒEƒ“ƒhƒ}ƒl[ƒWƒƒ[‚ÌXV
    auto& sound = SoundManager::GetInstance();
    sound.Update();
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();
    context->ClearRenderTargetView(m_transitionTexture->GetRenderTargetView(), DirectX::Colors::Black);

    // ƒV[ƒ“ƒ}ƒl[ƒWƒƒ[‚Ì•`‰æ
    m_sceneManager->Render();

    if (m_transitionMask->GetCreateMaskRequest() == TransitionMask::CreateMaskRequest::COPY)
    {
        auto renderTarget = m_deviceResources->GetRenderTarget();
        auto destResource = m_transitionTexture->GetRenderTarget();

        if (renderTarget && destResource)
        {
            context->CopyResource(destResource, renderTarget);
        }
        m_transitionMask->SetCreateMaskRequest(TransitionMask::CreateMaskRequest::NONE);
    }

    // ƒrƒ…[ƒ|[ƒg‚Ìİ’è
    auto const viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    // ‰æ–Ê‘JˆÚƒ}ƒXƒN‚Ì•`‰æ
    m_transitionMask->Draw(
        context,
        m_states.get(),
        m_transitionTexture->GetShaderResourceView(),
        m_deviceResources->GetOutputSize());

    m_deviceResources->PIXEndEvent();

    // Show the new frame.
    m_deviceResources->Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, DirectX::Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto const viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
}

void Game::OnDeactivated()
{
}

void Game::OnSuspending()
{
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();
}

void Game::OnWindowMoved()
{
    auto const r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);

    // ƒtƒ‹ƒXƒNƒŠ[ƒ“‚©’²‚×‚é
    BOOL fullscreen = FALSE;
    m_deviceResources->GetSwapChain()->GetFullscreenState(&fullscreen, nullptr);
    // ƒtƒ‹ƒXƒNƒŠ[ƒ“‚ª‰ğœ‚³‚ê‚Ä‚µ‚Ü‚Á‚½‚Ìˆ—
    if (m_fullscreen != fullscreen)
    {
        m_fullscreen = fullscreen;
        // ResizeBuffersŠÖ”‚ğŒÄ‚Ño‚·
        m_deviceResources->CreateWindowSizeDependentResources();
    }
}

void Game::OnDisplayChange()
{
    m_deviceResources->UpdateColorSpace();
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

    // TODO: Game window is being resized.
    // 
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 1280;
    height = 720;
}

#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto context = m_deviceResources->GetD3DDeviceContext();

    m_states = std::make_unique <DirectX::CommonStates>(device);
    m_debugFont = std::make_unique<DebugFont>(device, context, L"Resources/Font/SegoeUI_18.spritefont");

    m_userResources = std::make_unique<UserResources>();
    m_sceneManager = std::make_unique<SceneManager<UserResources>>
        (m_userResources.get());

    m_transitionMask = std::make_unique<TransitionMask>(
        m_deviceResources->GetD3DDevice(),
        m_deviceResources->GetD3DDeviceContext(),
        1.0f
    );

    // ƒŒƒ“ƒ_ƒŠƒ“ƒOƒeƒNƒXƒ`ƒƒ‚ÉƒfƒoƒCƒX‚ğİ’è‚·‚é
    m_transitionTexture->SetDevice(device);

    // ƒ†[ƒU[ƒŠƒ\[ƒX‚Ì“o˜^
    m_userResources->SetCommonStates(m_states.get());
    m_userResources->SetDebugFont(m_debugFont.get());
    m_userResources->SetDeviceResources(m_deviceResources.get());
    m_userResources->SetKeyboardTracker(&m_keyboardTracker);
    m_userResources->SetMouseStateTracker(&m_mouseTracker);
    m_userResources->SetStepTimerStates(&m_timer);
    m_userResources->SetTransitionMask(m_transitionMask.get());

    // ƒfƒoƒCƒXˆË‘¶ƒŠƒ\[ƒX‚Ìì¬
    m_sceneManager->CreateDeviceDependentResources();

    // ƒTƒEƒ“ƒhƒ}ƒl[ƒWƒƒ[‚Ì‰Šú‰»
    auto& sound = SoundManager::GetInstance();
    sound.Initialize();

    // ƒTƒEƒ“ƒh‚Ìƒ[ƒh
    sound.LoadSound(L"TITLEBGM", L"Resources/Sounds/titleBGM.wav");
    sound.LoadSound(L"GAMEPLAYBGM", L"Resources/Sounds/gameplayBGM.wav");
    sound.LoadSound(L"TUTORIALBGM", L"Resources/Sounds/tutorialBGM.wav");

    sound.LoadSound(L"SELECT", L"Resources/Sounds/select.wav");
    sound.LoadSound(L"DECISION", L"Resources/Sounds/decision.wav");
    sound.LoadSound(L"JUMP", L"Resources/Sounds/jump.wav");
    sound.LoadSound(L"ATTACK", L"Resources/Sounds/attack.wav");
    sound.LoadSound(L"DAMAGE", L"Resources/Sounds/damage.wav");
    sound.LoadSound(L"FALL", L"Resources/Sounds/falling.wav");
    sound.LoadSound(L"DEFENSE", L"Resources/Sounds/shield.wav");
    sound.LoadSound(L"OPEN", L"Resources/Sounds/door_open.wav");
    sound.LoadSound(L"SWITCH", L"Resources/Sounds/switch.wav");
    sound.LoadSound(L"TELEPORT", L"Resources/Sounds/teleport.wav");
    sound.LoadSound(L"LIFT", L"Resources/Sounds/lift.wav");
    sound.LoadSound(L"PUT", L"Resources/Sounds/put.wav");
    sound.LoadSound(L"CLEAR", L"Resources/Sounds/result.wav");
    sound.LoadSound(L"GAMEOVER", L"Resources/Sounds/gameover.wav");

}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.
    m_sceneManager->CreateWindowSizeDependentResources();

    auto size = m_deviceResources->GetOutputSize();

    // ƒTƒCƒY‚ª‚O‚È‚ç‰½‚à‚µ‚È‚¢
    if (size.right - size.left <= 0 || size.bottom - size.top <= 0) return;

    // ƒeƒNƒXƒ`ƒƒ‚ğV‚µ‚¢ƒTƒCƒY‚Åì‚è’¼‚·
    m_transitionTexture->SetWindow(size);

    // “h‚è‚Â‚Ô‚µˆ—
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto rtv = m_transitionTexture->GetRenderTargetView();
    if (rtv)
    {
        context->ClearRenderTargetView(rtv, DirectX::Colors::Black);
    }
}

void Game::SetFullscreenState(BOOL value)
{
    if (m_fullscreen == value) return; // ó‘Ô‚ª•Ï‚í‚ç‚È‚¢‚È‚ç‰½‚à‚µ‚È‚¢

    m_fullscreen = value;
    m_deviceResources->GetSwapChain()->SetFullscreenState(m_fullscreen, nullptr);

    // ƒŠƒ\[ƒXÄ¶¬
    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.
    m_sceneManager->CreateDeviceDependentResources();

}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion