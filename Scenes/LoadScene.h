//--------------------------------------------------------------------------------------
// File: LoadScene.h
//
// ロードシーンクラス
//--------------------------------------------------------------------------------------
#pragma once

#include <vector>
#include "SKLib/SceneManager.h"
#include "SKLib/ScreenManager.h"
#include "SKLib/TitleCamera.h"
#include "SKLib/UserResources.h"

class LoadScene : public LoadingScreen<UserResources>
{
public:
	// コンストラクタ／デストラクタ
	LoadScene();
	~LoadScene();

	// 初期化処理
	void Initialize() override;

	// 更新処理
	void Update(float elapsedTime) override;

	// 描画処理
	void Render() override;

	// 終了処理
	void Finalize() override;

	// デバイスに依存するリソースを作成する関数
	void CreateDeviceDependentResources() override;

	// ウインドウサイズに依存するリソースを作成する関数
	void CreateWindowSizeDependentResources() override;

	// デバイスロストした時に呼び出される関数
	void OnDeviceLost() override;

private:
	// デバイスリソースのポインタ
	std::unique_ptr<DX::DeviceResources>    m_deviceResources;

	// スプライトバッチのポインタ
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

	// ビュー行列
	DirectX::SimpleMath::Matrix m_view;

	// 射影行列
	DirectX::SimpleMath::Matrix m_proj;

	// カメラ
	TitleCamera m_camera;

	// モデル
	std::unique_ptr<DirectX::Model> m_platformBlockModel;

	// モデルの位置とスケール
	DirectX::SimpleMath::Vector3 m_gimmickBlockPosition;
	DirectX::SimpleMath::Vector3 m_gimmickBlockScale;

	// 床の回転度
	DirectX::SimpleMath::Quaternion m_gimmickBlockRotate;

	// テクスチャ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_loadingSRV;
};

