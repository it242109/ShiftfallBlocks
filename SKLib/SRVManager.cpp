//--------------------------------------------------------------------------------------
// File: SRVManager.cpp
//
// リソースを管理するクラス
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "SRVManager.h"
#include "SKLib/ReadData.h"

// データのロード
bool SRVManager::LoadData(const wchar_t* name)
{
    if (m_list.count(name)) return false;

    m_list.emplace(name, DX::ReadData(name));

    return true;
}

// データの削除
bool SRVManager::ReleaseData(const wchar_t* name)
{
    if (!m_list.count(name)) return false;

    m_list.erase(name);

    return true;
}

// データの取得
std::vector<uint8_t> SRVManager::GetData(const wchar_t* name)
{
    return m_list.at(name);
}
