//--------------------------------------------------------------------------------------
// File: SRVManager.h
//
// リソースを管理するクラス
//--------------------------------------------------------------------------------------
#pragma once

#include <unordered_map>
#include <vector>
#include <string>

class SRVManager
{
public:
	// ゲッター／セッター -------------------------------------------------------------------
	// データの取得
	std::vector<uint8_t> GetData(const wchar_t* name);

public:
	// 関数 ---------------------------------------------------------------------------------
	// データのロード
	bool LoadData(const wchar_t* name);
	// データの削除
	bool ReleaseData(const wchar_t* name);

private:
	// メンバ変数 ---------------------------------------------------------------------------
	// リソースリスト
	std::unordered_map<std::wstring, std::vector<uint8_t>> m_list;
	// デバイスリソース
	ID3D11Device* m_device = nullptr;
};



