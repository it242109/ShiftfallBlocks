//--------------------------------------------------------------------------------------
// File: SoundManager.cpp
// 
// サウンドマネージャー
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "SoundManager.h"

/*
* @brief　コンストラクタ／デストラクタ
*
* @param[in]  なし
*
* @return     なし
*/
SoundManager::SoundManager()
{
    m_audioEngine = std::make_unique<DirectX::AudioEngine>();
}
SoundManager::~SoundManager()
{
    Shutdown();
}

/*
* @brief　初期化処理
* 
* @param[in]  なし
*
* @return     なし
*/
void SoundManager::Initialize()
{
    if (!m_audioEngine->Update())
    {
        if (m_audioEngine->IsCriticalError())
        {
            MessageBox(NULL, L"AudioEngine critical error", L"エラー", MB_OK | MB_ICONHAND);
        }
    }
}

/*
* @brief　更新処理
*
* @param[in]  なし
*
* @return     なし
*/
void SoundManager::Update()
{
    m_audioEngine->Update();
}

/*
* @brief　サウンド読み込み
*
* @param[in]  key      サウンドの識別子
* @param[in]  filePath サウンドのファイルパス
*
* @return     成功した場合はtrue、失敗した場合はfalse
*/
bool SoundManager::LoadSound(const std::wstring& key, const std::wstring& filePath)
{
    try
    {
        auto sound = std::make_unique<DirectX::SoundEffect>(m_audioEngine.get(), filePath.c_str());
        m_soundEffects[key] = std::move(sound);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

/*
* @brief　再生
*
* @param[in]  key      サウンドの識別子
* @param[in]  volume   音量
* @param[in]  pitch    ピッチ
* @param[in]  pan      パン（左右の定位）
*
* @return     なし
*/
void SoundManager::Play(const std::wstring& key, float volume, float pitch, float pan)
{
    // サウンドエフェクトがあるか確認
    auto seIt = m_soundEffects.find(key);
    if (seIt == m_soundEffects.end()) return;

    // 再生中のサウンドがある場合は停止してから再生
    auto it = m_instances.find(key);
    if (it != m_instances.end())
    {
        it->second->Stop();
        m_instances.erase(it);
    }

	// 新しいインスタンスを作成して再生
    auto instance = seIt->second->CreateInstance();
    instance->SetVolume(volume);
    instance->SetPitch(pitch);
    instance->SetPan(pan);
    instance->Play(false);
    m_instances[key] = std::move(instance);

}

/*
* @brief　ループ再生
*
* @param[in]  key      サウンドの識別子
* @param[in]  volume   音量
*
* @return     なし
*/
void SoundManager::PlayLoop(const std::wstring& key,float volume)
{
	// サウンドエフェクトがあるか確認
    auto it = m_soundEffects.find(key);
    if (it == m_soundEffects.end()) return;

	// 再生中のサウンドがある場合は停止してから再生
    auto instance = it->second->CreateInstance();
    instance->SetVolume(volume);
    instance->Play(true); // ループ
    m_instances[key] = std::move(instance);
}

/*
* @brief　停止
*
* @param[in]  key      サウンドの識別子
*
* @return     なし
*/
void SoundManager::Stop(const std::wstring& key)
{
	// 再生中のサウンドがあるか確認して停止
    auto it = m_instances.find(key);
    if (it != m_instances.end())
    {
        if (it->second)
        {
            it->second->Stop(true);
            it->second.reset();
        }
        m_instances.erase(it);
    }
}

/*
* @brief　全てのサウンドを停止
*
* @param[in]  なし
*
* @return     なし
*/
void SoundManager::StopAll()
{
	// 全ての再生中のサウンドを停止
    for (auto& pair : m_instances)
    {
        pair.second->Stop();
    }
    m_instances.clear();
}

/*
* @brief　終了処理
*
* @param[in]  なし
*
* @return     なし
*/
void SoundManager::Shutdown()
{
    // 全てのサウンドを停止
    StopAll();

    // ここでインスタンスをクリア
    m_instances.clear();

    // その後でサウンドエフェクトをクリア
    m_soundEffects.clear();

    if (m_audioEngine)
    {
        m_audioEngine->Suspend();
        m_audioEngine.reset();
    }
}

