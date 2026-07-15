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

#include "GameObjects/StageObjects/Player.h"
#include "GameObjects/UIs/Menu.h"

class TitleScene : public SceneBase<UserResources>
{
public:
	// 関数 ---------------------------------------------------------------------------------
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
	// デバイスに依存するリソースを作成する関数
	void CreateDeviceDependentResources() override;
	// ウインドウサイズに依存するリソースを作成する関数
	void CreateWindowSizeDependentResources() override;
	// デバイスロストした時に呼び出される関数
	void OnDeviceLost() override;

private:
	// 定数 ---------------------------------------------------------------------------------
	static const DirectX::SimpleMath::Vector3 TITLE_PLAYER_POSITION;	///< タイトルシーンのプレイヤーの位置
	static const DirectX::SimpleMath::Vector3 TITLE_PLAYER_SCALE;		///< タイトルシーンのプレイヤーの大きさ
	static const DirectX::SimpleMath::Vector3 GIMMICKBLOCK_POSITION;	///< 仕掛けブロックの位置
	static const DirectX::SimpleMath::Vector3 GIMMICKBLOCK_SCALE;		///< 仕掛けブロックの大きさ
	static const DirectX::SimpleMath::Vector3 FLOOR_POSITION;			///< 床の位置
	static const DirectX::SimpleMath::Vector3 FLOOR_SCALE;				///< 床の大きさ
	static const float WALL_OFFSET_DIST;	///<　壁オブジェクトの中心からの距離
	static const float WALL_THICKNESS;		///< 壁の厚み
	static const float WALL_HEIGHT;			///< 壁の高さ
	static const float WALL_LENGTH;			///< 壁の長さ

	static const float MENU_DEFAULT_POSITION_X;	///<メニューのデフォルトの位置X
	static const float MENU_DEFAULT_SCALE_X;	///< メニューのデフォルトの大きさX
	static const float MENU_DEFAULT_SCALE_Y;	///< メニューのデフォルトの大きさY

	static const float FIELD_OF_VIEW_DEGREES;	///< 視野角
	static const float NEAR_PLANE_DISTANCE;		///< カメラの最前面のクリップ距離
	static const float FAR_PLANE_DISTANCE;		///< カメラの最遠面のクリップ距離

private:
	// メンバ変数 ---------------------------------------------------------------------------
	// デバイスリソースのポインタ
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
	// 床モデル
	std::unique_ptr<DirectX::Model> m_floorModel;
	// 壁モデル
	std::unique_ptr<DirectX::Model> m_wallModel;
	// プレイヤーモデル
	std::unique_ptr<DirectX::Model> m_playerModel;
	// 仕掛けブロックモデル
	std::unique_ptr<DirectX::Model> m_gimmickBlockModel;
	// 床モデルの位置
	DirectX::SimpleMath::Vector3 m_floorPosition;
	// 床モデルの大きさ
	DirectX::SimpleMath::Vector3 m_floorScale;
	// 壁モデルの位置
	std::vector<DirectX::SimpleMath::Vector3> m_wallPositions;
	// 壁モデルの大きさ
	std::vector<DirectX::SimpleMath::Vector3> m_wallScales;
	// プレイヤーモデルの位置
	DirectX::SimpleMath::Vector3 m_playerPosition;
	// プレイヤーモデルの大きさ
	DirectX::SimpleMath::Vector3 m_playerScale;
	// 仕掛けブロックの位置
	DirectX::SimpleMath::Vector3 m_gimmickBlockPosition;
	// 仕掛けブロックの大きさ
	DirectX::SimpleMath::Vector3 m_gimmickBlockScale;
	// 床の回転度
	DirectX::SimpleMath::Quaternion m_floorRotate;
	// タイトルロゴのテクスチャ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_titleLogo;
	// 選択キーのテクスチャ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_selectKey;
	// メニュー
	std::unique_ptr<Menu> m_titleMenu;

	// フェード関連
	bool m_isClosing = false;

};