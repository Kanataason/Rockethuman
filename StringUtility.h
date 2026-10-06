#pragma once
#include <string>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

namespace StringUtility
{
	//UTF-8の文字列(JSONから読んだもの)を、ワイド文字列(UIの文字や、ファイルを開く関数用)に変える
	inline std::wstring ToWide(const std::string& utf8)
	{
		if (utf8.empty())
			return std::wstring();

		const int length = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
		if (length <= 0)
			return std::wstring();

		std::wstring wide(static_cast<size_t>(length), L'\0');
		MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, wide.data(), length);
		wide.resize(static_cast<size_t>(length) - 1); //終端の'\0'を除く
		return wide;
	}
}
