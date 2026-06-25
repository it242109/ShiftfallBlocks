//--------------------------------------------------------------------------------------
// File: SceneBase.h
//
// シーンの基底テンプレートクラス
//--------------------------------------------------------------------------------------
#pragma once

#define ESC_QUIT_ENABLE

#ifdef ESC_QUIT_ENABLE
#include "Keyboard.h"
#endif

template <class T>
class SceneManager;

template<class T>
class SceneBase
{
public:
	// ゲッター／セッター -------------------------------------------------------------------
	// シーンマネージャー設定関数
	void SetSceneManager(SceneManager<T>* sceneManager) { m_sceneManager = sceneManager; }
	// シーンマネージャーを取得する関数
	SceneManager<T>* GetSceneManager() const { return m_sceneManager; }

public:
	// 関数 ---------------------------------------------------------------------------------
	// コンストラクタ
	SceneBase() : m_sceneManager(nullptr) {}

	// デストラクタ
	virtual ~SceneBase() = default;

	// 初期化
	virtual void Initialize() = 0;

	// 更新
	virtual void Update(float elapsedTime) = 0;

	// 描画
	virtual void Render() = 0;

	// 終了処理
	virtual void Finalize() = 0;

	// デバイスに依存するリソースを作成する関数
	virtual void CreateDeviceDependentResources() {}

	// ウインドウサイズに依存するリソースを作成する関数
	virtual void CreateWindowSizeDependentResources() {}

	// デバイスロストした時に呼び出される関数
	virtual void OnDeviceLost() {}
public:
	// シーンの切り替え関数
	template <class U>
	void ChangeScene();

	template <class U, class V>
	void ChangeLoadingScene();

public:
	// メンバ変数 ---------------------------------------------------------------------------
	// 設定したリソース取得関数
	T* GetUserResources();
	// シーンマネージャーへのポインタ
	SceneManager<T>* m_sceneManager;

};

// ロード画面の基底クラス
template<class T>
using LoadingScreen = SceneBase<T>;

// --シーンマネージャーのテンプレート関数を読み込む---
#include "SceneManager.h"
// ---------------------------------------------------

// シーンの切り替え関数
template<class T>
template<class U>
void SceneBase<T>::ChangeScene()
{
	m_sceneManager->template SetNextScene<U>();
}

// ロード画面の切替関数
template<class T>
template<class U, class V>
void SceneBase<T>::ChangeLoadingScene()
{
	m_sceneManager->template SetLoadingScene<U, V>();
}

// ユーザーが設定したリソース取得関数
template<class T>
T* SceneBase<T>::GetUserResources()
{
	assert(m_sceneManager);
	return m_sceneManager->GetUserResources();
}

