//--------------------------------------------------------------------------------------
// File: PortalParticle.h
//
// ポータルパーティクルクラス
//--------------------------------------------------------------------------------------
#pragma once

#include "SKLib/DeviceResources.h"
#include <SimpleMath.h>
#include <vector>
#include <list>

#include "GameObjects/Effects/ParticleUtility.h"

// ポータル情報
struct PortalEmitterInfo
{
	DirectX::SimpleMath::Vector3 position;
	DirectX::SimpleMath::Vector3 scale;
};


class PortalParticle
{
	//	データ受け渡し用コンスタントバッファ(送信側)
	struct ConstBuffer
	{
		DirectX::SimpleMath::Matrix matWorld;
		DirectX::SimpleMath::Matrix matView;
		DirectX::SimpleMath::Matrix matProj;
		DirectX::SimpleMath::Vector4 Diffuse;
		
	};
public:
	// 関数 ------------------------------------------------------------
	// インプットレイアウト
	static const std::vector<D3D11_INPUT_ELEMENT_DESC> INPUT_LAYOUT;

	// コンストラクタ／デストラクタ
	PortalParticle();
	~PortalParticle();

	// テクスチャリソース読み込み関数
	void LoadTexture(const wchar_t* path);
	// 生成関数
	void Create(DX::DeviceResources* pDR);
	// 更新処理
	void Update(float elapsedTime);
	// 描画処理
	void Render(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj);
	// ビルボード作成関数
	void Billboard(
		DirectX::SimpleMath::Vector3 target,
		DirectX::SimpleMath::Vector3 eye,
		DirectX::SimpleMath::Vector3 up);

	// リストを渡す
	void SetEmitters(const std::vector<PortalEmitterInfo>& emitters) { m_emitters = emitters; }
private:
	// シェーダー作成部分だけ分離した関数
	void CreateShader();

	// ランダムな浮動小数点数を生成する関数
	float RandomFloat(float min, float max);

private:
	// 定数 -----------------------------------------------------------
	static const float EMITTER_RANGE_SCALE;			///< エミッタ―の範囲をどれくらいにするか
	static const int PARTICLE_COUNT ;				///< パーティクルの数

	static const float MIN_SPARN_RADIUS;			///< パーティクルの最小半径
	static const float MAX_SPARN_RADIUS;			///< パーティクルの最大半径

	static const float VELOCITY_HORIZONTAL_WEIGHT;	///< 横方向の速度の勢い
	static const float VELOCITY_UPWARD_BASE;		///< 上方向の基本速度
	static const float VELOCITY_SPEED_MIN;			///< 最小初速倍率
	static const float VELOCITY_SPEED_MAX;			///< 最大初速倍率

	static const float PARTICLE_LIFETIME;			///< 生存時間（秒）
private:
	// メンバ変数 -----------------------------------------------------
	// デバイスリソースへのポインタ
	DX::DeviceResources* m_pDR;	

	//	経過時間記録用
	float m_timer;

	// バッファ
	Microsoft::WRL::ComPtr<ID3D11Buffer>	m_CBuffer;

	//	入力レイアウト
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

	//	プリミティブバッチ
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColorTexture>> m_batch;
	//	コモンステート
	std::unique_ptr<DirectX::CommonStates> m_states;
	//	テクスチャハンドル
	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_texture;
	//	！New！
	//	頂点情報のリスト
	std::vector<DirectX::VertexPositionColorTexture> m_vertices;
	//	パーティクルに使う情報を保存するためのクラスオブジェクト用リスト
	std::list<ParticleUtility> m_particleUtility;
	//	カメラに関する情報
	DirectX::SimpleMath::Vector3 m_cameraPosition;
	DirectX::SimpleMath::Vector3 m_cameraTarget;

	//	頂点シェーダ
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	//	ピクセルシェーダ
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
	//	ジオメトリシェーダ
	Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_geometryShader;

	// ワールド行列
	DirectX::SimpleMath::Matrix m_world;
	// ビュー行列
	DirectX::SimpleMath::Matrix m_view;
	// 射影行列
	DirectX::SimpleMath::Matrix m_proj;

	// オブジェクトの位置
	DirectX::SimpleMath::Vector3 m_position;
	// パーティクルの中心位置
	std::vector<DirectX::SimpleMath::Vector3> m_centerPosition;

	DirectX::SimpleMath::Matrix m_billboard;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_customDepthStencilState;

	// 座標リスト
	std::vector<PortalEmitterInfo> m_emitters;

};
