//--------------------------------------------------------------------------------------
// File: SoundManager.h
// 
// サウンドマネージャー
//--------------------------------------------------------------------------------------
#pragma once

#include <Audio.h>
#include <memory>
#include <unordered_map>
#include <vector>

class SoundManager
{
public:
	// ゲッター／セッター -------------------------------------------------------------------
	// オーディオエンジンを取得
	DirectX::AudioEngine* GetEngine() { return m_audioEngine.get(); }

public:
	// 関数 ---------------------------------------------------------------------------------
	// シングルトンを取得する
	static SoundManager& GetInstance()
	{
		static SoundManager instance;
		return instance;
	}

	// 初期化処理
	void Initialize();

	// サウンド読み込み
	bool LoadSound(const std::wstring& key, const std::wstring& filePath);

	// 再生
	void Play(const std::wstring& key, float volume = 1.0f,float pitch = 0.0f, float pan = 0.0f);

	// ループ再生
	void PlayLoop(const std::wstring& key,float volume);

	// 停止
	void Stop(const std::wstring& key);

	// 全てのサウンドを停止
	void StopAll();

	// 更新
	void Update();

	// 終了処理
	void Shutdown();

private:
	// メンバ変数 ---------------------------------------------------------------------------
	// デフォルトコンストラクタ／デフォルトデストラクタ
	SoundManager();
	~SoundManager();

	// コンストラクタ／デストラクタ
	SoundManager(const SoundManager&) = delete;
	SoundManager& operator=(const SoundManager&) = delete;

	std::unique_ptr<DirectX::AudioEngine> m_audioEngine;

	// サウンドデータ管理
	std::unordered_map<std::wstring, std::unique_ptr<DirectX::SoundEffect>> m_soundEffects;

	// インスタンス管理
	std::unordered_map<std::wstring, std::unique_ptr<DirectX::SoundEffectInstance>> m_instances;
};

