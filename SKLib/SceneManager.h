//--------------------------------------------------------------------------------------
// File: SceneManager.h
//
// シーンを管理するテンプレートクラス
//--------------------------------------------------------------------------------------
#pragma once

#define ESC_QUIT_ENABLE

#ifdef ESC_QUIT_ENABLE
#include "Keyboard.h"
#endif

#include "SceneBase.h"

template <class T>
class SceneManager;

// シーンマネージャークラス
template<class T>
class SceneManager
{
private:
	// 共通でアクセスしたいオブジェクトへのポインタ
	T* m_userResources;

	// 現在のシーン
	std::unique_ptr<SceneBase<T>> m_currentScene;

	// 次のシーン
	std::unique_ptr<SceneBase<T>> m_nextScene;

	// ロード画面
	std::unique_ptr<LoadingScreen<T>> m_loadingScreen;

	// スレッドと共有数
	std::thread m_loadingThread;
	std::mutex m_loadingMutex;
	bool m_isLoading;

	// シーン削除
	void DeleteScene();
	
public:
	// コンストラクタ
	SceneManager(T* userResources = nullptr)
		: m_userResources(userResources)
		, m_currentScene(nullptr)
		, m_nextScene(nullptr)
		, m_loadingScreen()
		, m_isLoading()
	{
	};
	// デストラクタ
	virtual ~SceneManager() 
	{
		DeleteScene(); 

		if (m_loadingThread.joinable())
		{
			m_loadingThread.join();
		}
	};

	// 更新
	void Update(float elapsedTime);

	// 描画
	void Render();

	// デバイスに依存するリソースを作成する関数
	void CreateDeviceDependentResources();

	// ウインドウサイズに依存するリソースを作成する関数
	void CreateWindowSizeDependentResources();

	// デバイスロストした時に呼び出される関数
	virtual void OnDeviceLost();

	// シーンの設定関数
	template <class U>
	void SetScene();

	// 次のシーンの設定関数
	template <class U>
	bool SetNextScene();

	// ロード画面を伴うシーンの設定関数
	template<class U, class V>
	bool SetLoadingScene();

	// ユーザーリソース設定関数
	void SetUserResources(T* userResources) { m_userResources = userResources; }
	
	// ユーザーリソース取得関数
	T* GetUserResources() { return m_userResources; }

	// 次のシーンへの準備
	void PrepareNextScene(std::function<std::unique_ptr<SceneBase<T>>()> sceneFactory);

	// ロード中かどうか
	bool IsLoading();

	// 現在のシーンの型を取得する関数
	const std::type_info& GetCurrentSceneType() const;

};

// シーンの設定関数
template <class T>
template <class U>
void SceneManager<T>::SetScene()
{
	assert(m_currentScene == nullptr);

	// シーンを生成
	SetNextScene<U>();
}

// シーンの設定関数
template <class T>
template <class U>
bool SceneManager<T>::SetNextScene()
{
	if (!m_nextScene)
	{
		// シーンを生成
		PrepareNextScene([]() {return std::make_unique<U>(); });
		return true;
	}

	return false;
}
template <class T>
template <class U, class V>
bool SceneManager<T>::SetLoadingScene()
{
	if (SetNextScene<U>())
	{
		// ロード画面を作成
		m_loadingScreen = std::make_unique<V>();
		m_loadingScreen->SetSceneManager(this);
		m_loadingScreen->Initialize();
	}
	return false;
}

// 更新関数
template <class T>
void SceneManager<T>::Update(float elapsedTime)
{
#ifdef ESC_QUIT_ENABLE
	// ESCキーで終了
	auto kb = DirectX::Keyboard::Get().GetState();
	if (kb.Escape) PostQuitMessage(0);
#endif

	if (!IsLoading() && m_nextScene)
	{
		DeleteScene();
		assert(m_currentScene == nullptr);

		m_currentScene = std::move(m_nextScene);

		if (m_loadingScreen)
		{
			m_loadingScreen->Finalize();
			m_loadingScreen.reset();
		}
	}
	if (m_loadingScreen)
	{
		m_loadingScreen->Update(elapsedTime);
		return;
	}

	if (m_currentScene) m_currentScene->Update(elapsedTime);
}

// 描画関数
template <class T>
void SceneManager<T>::Render()
{
	// シーンの描画
	if (m_loadingScreen)
	{
		m_loadingScreen->Render();
		return;
	}
	if (m_currentScene)
	{
		m_currentScene->Render();
	}
}

// デバイスに依存するリソースを作成する関数
template <class T>
void SceneManager<T>::CreateDeviceDependentResources()
{
	if (m_currentScene) m_currentScene->CreateDeviceDependentResources();
}

// ウインドウサイズに依存するリソースを作成する関数
template <class T>
void SceneManager<T>::CreateWindowSizeDependentResources()
{
	if (m_currentScene) m_currentScene->CreateWindowSizeDependentResources();
}

// デバイスロストした時に呼び出される関数
template <class T>
void SceneManager<T>::OnDeviceLost()
{
	if (m_currentScene) m_currentScene->OnDeviceLost();
}

template<class T>
inline void SceneManager<T>::PrepareNextScene(std::function<std::unique_ptr<SceneBase<T>>()> sceneFactory)
{
	if (m_loadingThread.joinable())
	{
		m_loadingThread.join();
	}
	m_isLoading = true;

	m_loadingThread = std::thread
	{
		[=]()
		{
			m_nextScene = sceneFactory();
			m_nextScene->SetSceneManager(this);
			m_nextScene->Initialize();

			m_loadingMutex.lock();
			m_isLoading = false;
			m_loadingMutex.unlock();
		}
	};
}

// ロード中かどうか
template<class T>
inline bool SceneManager<T>::IsLoading()
{
	m_loadingMutex.lock();
	bool isLoading = m_isLoading;
	m_loadingMutex.unlock();

	return isLoading;
}

// 現在のシーンの型を取得する関数
template<class T>
inline const std::type_info& SceneManager<T>::GetCurrentSceneType() const
{
	if (m_currentScene)
	{
		return typeid(*m_currentScene);
	}
	else
	{
		return typeid(void);
	}
}

// シーンの削除関数
template <class T>
void SceneManager<T>::DeleteScene()
{
	if (m_currentScene)
	{
		m_currentScene->Finalize();

		m_currentScene.reset();
	}
}
