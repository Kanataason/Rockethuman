#pragma once
#include <cctype>
#include <string>
#include <string_view>

namespace BoneNameUtility
{
	//ミクサモのボーン名の、番号つきの接頭辞を、番号なしにそろえる
	//"mixamorig9:Hips" → "mixamorig:Hips"
	//ミクサモの複数のキャラクターを、Blenderなどで1つにまとめて出力すると、
	//接頭辞が mixamorig1: mixamorig9: のように、番号つきになる。
	//ラグドールとIKは、"mixamorig:Hips" の名前で探すので、読み込むときに、そろえておく
	inline std::string NormalizeMixamoPrefix(std::string name)
	{
		constexpr std::string_view prefix = "mixamorig";
		if (name.compare(0, prefix.size(), prefix) != 0)
			return name;

		const size_t digitsBegin = prefix.size();
		size_t digitsEnd = digitsBegin;
		while (digitsEnd < name.size() && std::isdigit(static_cast<unsigned char>(name[digitsEnd])))
			++digitsEnd;

		//数字のあとが ':' のときだけ、数字を消す(mixamorig:… や、別の名前は、そのまま)
		if (digitsEnd > digitsBegin && digitsEnd < name.size() && name[digitsEnd] == ':')
			name.erase(digitsBegin, digitsEnd - digitsBegin);

		return name;
	}
}
