//--------------------------------------------------------------------------------------
// File: BinaryFile.cpp
//
// バイナリファイルを読み込むクラス
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "BinaryFile.h"

#include <fstream>
#include <assert.h>

/*
* @brief ロード
*
* @param[in] fileName ロードするファイル名
*
* @return BinaryFileクラスのインスタンスを返す
*/
std::unique_ptr<BinaryFile> BinaryFile::LoadFile(const wchar_t* fileName)
{
	std::ifstream ifs;

	ifs.open(fileName, std::ios::in | std::ios::binary);
	assert(ifs);

	// ファイル内容の読み取り
	BinaryFile* bin = new BinaryFile();

	// ファイルサイズの取得
	ifs.seekg(0, std::fstream::end);
	std::streamoff eofPos = ifs.tellg();
	ifs.clear();
	ifs.seekg(0, std::fstream::beg);
	std::streamoff begPos = ifs.tellg();
	bin->m_shaderSize = (unsigned int)(eofPos - begPos);

	// 読み込むためのメモリを確保
	bin->m_shaderData.reset(new char[bin->m_shaderSize]);

	// ファイル先頭からバッファへコピー
	ifs.read(bin->m_shaderData.get(), bin->m_shaderSize);

	//	ファイルクローズ
	ifs.close();

	return std::make_unique<BinaryFile>(bin);
}

/*
* @brief コンストラクタ
*
* @param[in] なし
*
* @return なし
*/
BinaryFile::BinaryFile()
{
	m_shaderSize = 0;
}

/*
* @brief ムーブコンストラクタ
*
* @param[in] in ムーブ元のBinaryFileクラスのインスタンス
*
* @return なし
*/
BinaryFile::BinaryFile(BinaryFile* in)
{
	m_shaderData = std::move(in->m_shaderData);
	m_shaderSize = in->m_shaderSize;
}
