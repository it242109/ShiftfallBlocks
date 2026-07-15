//--------------------------------------------------------------------------------------
// File: TransitionMask.h
//
// 画面切り替え用のマスク表示クラス
//--------------------------------------------------------------------------------------
#pragma once
#include "SpriteBatch.h"

class TransitionMask
{
public:
	enum class CreateMaskRequest
	{
		NONE,
		COPY,
	};
public:
	// ゲッター／セッター -------------------------------------------------------------------
	// --- 開閉状態  ---
	// オープン（開いている／表示中）状態か確認する関数
	bool IsOpen() const { return m_open; }
	// クローズ（閉まっている／非表示中）状態か確認する関数
	bool IsClose() const { return !m_open; }

	// --- 演出の進行度・時間  ---
	// 内部のオープン具合をそのまま返す関数
	float GetOpenRate() const { return m_rate; }
	// 現在の状態（オープンかクローズか）に応じた実際の割合（0～1）を取得する関数
	float GetRate() const
	{
		if (m_open) return 1.0f - m_rate;
		return m_rate;
	}
	// オープン、クローズにかかる時間を設定する関数
	void SetInterval(float interval) { m_interval = interval; }

	// --- 演出の完了判定  ---
	// 演出が完了しているか確認する関数
	bool IsEnd() const
	{
		// オープンモードで rate が 0.0f（完全に非表示）なら完了
		if (m_open && m_rate == 0.0f) return true;

		// クローズモードで rate が 1.0f（完全に表示）なら完了
		if (!m_open && m_rate == 1.0f) return true;

		return false;
	}

	// --- マスク生成リクエスト ---
	// マスク生成リクエストの情報を取得する関数
	CreateMaskRequest GetCreateMaskRequest() const { return m_request; }
	// マスク生成リクエストの情報を設定する関数
	void SetCreateMaskRequest(CreateMaskRequest request) { m_request = request; }

public:
	// 関数 ---------------------------------------------------------------------------------
	// コンストラクタ
	TransitionMask(
		ID3D11Device* device,
		ID3D11DeviceContext* context,
		float interval
	);
	// 更新処理
	void Update(float elapsedTime);
	// 描画処理
	void Draw(
		ID3D11DeviceContext* context,
		DirectX::CommonStates* states,
		ID3D11ShaderResourceView* texture,
		const RECT& rect
	);
	// オープンする関数
	void Open();
	// クローズする関数
	void Close();

private:
	// メンバ変数 ---------------------------------------------------------------------------
	// 作成リクエスト
	CreateMaskRequest m_request;
	// 割合(0～1)
	float m_rate;
	// オープン又はクローズするまでの時間
	float m_interval;
	// オープンフラグ（trueの場合オープン）
	bool m_open;
	// スプライトバッチ
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
	// ピクセルシェーダー
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PS_Fade;
	// 定数バッファの構造体
	struct ConstantBuffer
	{
		float rate;		///< 演出の進行度
		float pad[3];	///< パティング
	};
	// 定数バッファへのポインタ
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
	// マスク用テクスチャ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_maskTexture;
};
