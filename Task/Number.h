//--------------------------------------------------------------------------------------
// File: Number.h
//
// 数字を管理するクラス
//
//--------------------------------------------------------------------------------------

#pragma once

#include "SKLib/TaskManager.h"
#include "SKLib/ScreenManager.h"
#include "Task/SpriteSheet.h"
class Number : public Task
{
public:
	// コンストラクタ
	Number(
		std::unique_ptr<DirectX::SpriteBatch>* pSpriteBatch,
		ID3D11ShaderResourceView** pNumberSRV,
		uint32_t digit = 8,
		float scale = 1.0f
	);
	
	// デストラクタ
	~Number();

	// 更新処理
	bool Update(float elapsedTime) override; 

	void Render() override;

private:
	std::unique_ptr<SpriteSheet> m_spriteSheet;				// スプライトシート
	std::unique_ptr<DirectX::SpriteBatch>* m_pSpriteBatch;	// スプライトバッチ
	ID3D11ShaderResourceView** m_pNumberSRV;				// テクスチャ

	// 表示位置
	DirectX::SimpleMath::Vector2 m_position;

	// 表示桁数
	uint32_t m_digit;

	// 表示する数字
	std::wstring m_number;
	//uint32_t m_number;

	// スケール
	float m_scale;

	// スプライトフレーム
	const SpriteSheet::SpriteFrame* m_frame[10];
	const SpriteSheet::SpriteFrame* m_dotFrame;
	const SpriteSheet::SpriteFrame* m_colonFrame;

	std::unique_ptr<SpriteSheet> m_sprites;

public:
	void SetPosition(DirectX::SimpleMath::Vector2 pos) { m_position = pos; }
	void SetDigit(uint32_t digit) { m_digit = digit; }
	void SetScale(float scale) { m_scale = scale; }

	// 整数を設定
	void SetNumber(int number);

	// 小数を設定
	void SetNumberDecimal(float number, int precision = 1);

	// タイマーを設定
	void SetTimer(int totalSeconds);

};

