//--------------------------------------------------------------------------------------
// File: TitleScene.h
//
// タイトルシーンクラス
//--------------------------------------------------------------------------------------
#pragma once

#include "SKLib/SceneManager.h"
#include "SKLib/InputManager.h"
#include "SKLib/ScreenManager.h"
#include "SKLib/SoundManager.h"
#include "SKLib/TitleCamera.h"
#include "SKLib/UserResources.h"

#include "GameObjects/Stages/Player.h"
#include "GameObjects/UIs/Menu.h"

class TitleScene : public SceneBase<UserResources>
{
public:
	// コンストラクタ／デストラクタ
	TitleScene();
	~TitleScene();

	// 初期化処理
	void Initialize() override;

	// 更新処理
	void Update(float elapsedTime) override;

	// 描画処理
	void Render() override;

	// 終了処理
	void Finalize() override;

	// メニューの設定
	void SetupMenu();

	// デバイスに依存するリソースを作成する関数
	void CreateDeviceDependentResources() override;

	// ウインドウサイズに依存するリソースを作成する関数
	void CreateWindowSizeDependentResources() override;

	// デバイスロストした時に呼び出される関数
	void OnDeviceLost() override;

private:
	std::unique_ptr<DX::DeviceResources>    m_deviceResources;

	// スプライトバッチのポインタ
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

	// キーボード状態を保存
	DirectX::Keyboard::State m_prevKeyboardState = {};

	// ビュー行列
	DirectX::SimpleMath::Matrix m_view;

	// 射影行列
	DirectX::SimpleMath::Matrix m_proj;

	// カメラ
	TitleCamera m_camera;

	// モデル
	std::unique_ptr<DirectX::Model> m_floorModel;
	std::unique_ptr<DirectX::Model> m_wallModel;
	std::unique_ptr<DirectX::Model> m_playerModel;
	std::unique_ptr<DirectX::Model> m_gimmickBlockModel;

	// モデルの位置とスケール
	DirectX::SimpleMath::Vector3 m_floorPosition;
	DirectX::SimpleMath::Vector3 m_floorScale;
	AABB m_floorCollision;

	std::vector<DirectX::SimpleMath::Vector3> m_wallPositions;
	std::vector<DirectX::SimpleMath::Vector3> m_wallScales;
	
	DirectX::SimpleMath::Vector3 m_playerPosition;
	DirectX::SimpleMath::Vector3 m_playerScale;

	DirectX::SimpleMath::Vector3 m_gimmickBlockPosition;
	DirectX::SimpleMath::Vector3 m_gimmickBlockScale;

	// 床の回転度
	DirectX::SimpleMath::Quaternion m_floorRotate;

	// テクスチャ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_titleLogo;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_selectKey;

	std::unique_ptr<Menu> m_titleMenu;

	// フェード関連
	bool m_isClosing = false;

};