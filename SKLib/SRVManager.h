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
private:
	// リソースリスト
	std::unordered_map<std::wstring, std::vector<uint8_t>> m_list;

	ID3D11Device* m_device = nullptr;

public:
	// データのロード
	bool LoadData(const wchar_t* name);

	// データの削除
	bool ReleaseData(const wchar_t* name);

	// データの取得
	std::vector<uint8_t> GetData(const wchar_t* name);
};



