//--------------------------------------------------------------------------------------
// File: UserInterface.h
//
// ユーザーインターフェースクラス
//--------------------------------------------------------------------------------------

#pragma once

#include "StepTimer.h"
#include "SKLib/DeviceResources.h"

#include <SimpleMath.h>
#include <Effects.h>
#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include <WICTextureLoader.h>
#include <CommonStates.h>
#include <vector>

//	UIのアンカーポイントの列挙数
enum ANCHOR
{
	TOP_LEFT = 0,
	TOP_CENTER,
	TOP_RIGHT,

	MIDDLE_LEFT,
	MIDDLE_CENTER,
	MIDDLE_RIGHT,

	BOTTOM_LEFT,
	BOTTOM_CENTER,
	BOTTOM_RIGHT
};

class UserInterface
{
public:
	//	データ受け渡し用コンスタントバッファ(送信側)
	struct ConstBuffer
	{
		DirectX::SimpleMath::Vector4	windowSize;
	};

public:
	// ゲッター／セッター -------------------------------------------------------------------
	// --- 座標  ---
	// 座標を取得する関数
	DirectX::SimpleMath::Vector2 GetPosition() const { return m_position; }
	// 座標を設定する関数
	void SetPosition(DirectX::SimpleMath::Vector2 position){ m_position = position; }

	// --- スケール・大きさ ---
	// 現在のスケールを取得する関数
	DirectX::SimpleMath::Vector2 GetScale() const { return m_scale; }
	// 基準となるベーススケールを取得する関数
	DirectX::SimpleMath::Vector2 GetBaseScale() const { return m_baseScale; }
	// スケールを設定する関数
	void SetScale(DirectX::SimpleMath::Vector2 scale) { m_scale = scale; }

	// --- アンカー・基準点 ---
	// アンカー（基準点）を取得する関数
	ANCHOR GetAnchor() const { return m_anchor; }

	// --- ウィンドウ・システム設定 ---
	// 画面サイズを設定する関数
	void SetWindowSize(const int& width, const int& height);

public:
	// 関数 ---------------------------------------------------------------------------------
	// コンストラクタ／デストラクタ
	UserInterface();
	~UserInterface();
	// テクスチャリソース読み込み関数
	void LoadTexture(const wchar_t* path);
	// 生成関数
	void Create(DX::DeviceResources* pDR
		, const wchar_t* path
		, DirectX::SimpleMath::Vector2 position
		, DirectX::SimpleMath::Vector2 scale
		, ANCHOR anchor);

	// 描画処理
	void Render();
private:
	// Shader作成部分だけ分離した関数
	void CreateShader();
	
private:
	// 定数----------------------------------------------------------------------------------
	// インプットレイアウト
	static const std::vector<D3D11_INPUT_ELEMENT_DESC> INPUT_LAYOUT;

private:
	// メンバ変数 ---------------------------------------------------------------------------
	// デバイスリソースのポインタ
	DX::DeviceResources* m_pDR;
	// バッファ
	Microsoft::WRL::ComPtr<ID3D11Buffer>	m_CBuffer;
	// ステップタイマー
	DX::StepTimer                           m_timer;
	//	入力レイアウト
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
	//	プリミティブバッチ
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColorTexture>> m_batch;
	//	コモンステート
	std::unique_ptr<DirectX::CommonStates> m_states;
	//	テクスチャハンドル
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
	// リソース
	Microsoft::WRL::ComPtr<ID3D11Resource> m_res;
	//	頂点シェーダ
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	//	ピクセルシェーダ
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
	//	ジオメトリシェーダ
	Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_geometryShader;
	// ウィンドウのサイズ
	int m_windowWidth, m_windowHeight;
	// テクスチャのサイズ
	int m_textureWidth, m_textureHeight;
	// スケール
	DirectX::SimpleMath::Vector2 m_scale;
	// 基準のスケール
	DirectX::SimpleMath::Vector2 m_baseScale;
	// 位置
	DirectX::SimpleMath::Vector2 m_position;
	// 基準点(アンカー)
	ANCHOR m_anchor;
};
