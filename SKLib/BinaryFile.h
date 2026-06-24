//--------------------------------------------------------------------------------------
// File: BinaryFile.h
//
// バイナリファイルを読み込むクラス
//--------------------------------------------------------------------------------------
#pragma once

#include <memory>

class BinaryFile
{
private:
	// データ
	std::unique_ptr<char[]> m_shaderData;

	// シェーダーのサイズ
	unsigned int m_shaderSize;

public:
	// ロード
	static std::unique_ptr<BinaryFile> LoadFile(const wchar_t* fileName);

	// コンストラクタ
	BinaryFile();

	// ムーブコンストラクタ
	BinaryFile(BinaryFile* in);

	char* GetData() { return m_shaderData.get(); }
	unsigned int GetSize() { return m_shaderSize; }
};

