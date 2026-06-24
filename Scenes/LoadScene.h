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
	// 関数 ---------------------------------------------------------------------------------
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
	// 定数 ------------------------------------------------------------------------
	static const float GIMMICK_BLOCK_INIT_X;	///< 演出用ブロックの初期X座標
	static const float GIMMICK_BLOCK_INIT_Y;	///< 演出用ブロックの初期Y座標
	static const float GIMMICK_BLOCK_INIT_Z;	///< 演出用ブロックの初期Z座標
	static const float GIMMICK_BLOCK_SCALE;		///< 演出用ブロックの標準の大きさ

	static const float CAMERA_UPDATE_SPEED;		///< カメラ更新速度の倍率

	static const float DEFAULT_ROTATION_ANGLE;	///< デフォルトの回転角度
	static const float DISTAICE_EPSILON_SQUARED;///< カメラとターゲットの重複を判定する微小距離の平方
	static const float CAMERA_Z_PUSH_OFFSET;	///< 位置重複時にカメラを後ろに押し戻すオフセット量
	static const float DEFAULT_SPRITE_ROTATION;	///< 背景スプライトの回転角度
	static const float BASE_SPRITE_SCALE;		///< 背景スプライトの基本拡大率

	static const float FIELD_OF_VIEW_DEGREES;	///< 視野角
	static const float NEAR_PLANE_DISTANCE;		///< カメラの最前面のクリップ距離
	static const float FAR_PLANE_DISTANCE;		///< カメラの最遠面のクリップ距離

private:
	// メンバ変数 ------------------------------------------------------------------
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

