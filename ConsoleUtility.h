#pragma once
#include <DirectXMath.h>
#include <string>
#include <iostream>
#include <d3dcompiler.h>
#include <d3d11.h>
#include <sstream>
#include <Windows.h>

using namespace DirectX;

class DebugConsole
{
public:
	///<summary>
	///コンソールに受け取った行列を出す関数
	///</summary>
	///<param name="main"></param>
	///<param name="name"></param>
	static void ConsoleFLOAT4X4(const XMFLOAT4X4 main, const std::string type, const std::string& name)
	{
		char buffer[512];

		sprintf_s(buffer,
			"Name %s : %s\n"
			"[%f,%f,%f,%f]\n"
			"[%f,%f,%f,%f]\n"
			"[%f,%f,%f,%f]\n"
			"[%f,%f,%f,%f]\n\n",
			name.c_str(),
			type.c_str(),
			main._11, main._12, main._13, main._14,
			main._21, main._22, main._23, main._24,
			main._31, main._32, main._33, main._34,
			main._41, main._42, main._43, main._44);
		
		OutputDebugStringA(buffer);
	}
	template<typename T>
	static void ConsoleDebug(const T& value, const std::string& name, const std::string& typeName)
	{
		std::ostringstream oss;

		oss << "typeName: " << typeName << "\n";
		oss << "Name:" << name << "\n";
		oss << "Value: " << value << "\n\n";

		OutputDebugStringA(oss.str().c_str());
	}
	static void ConsoleFLOAT3(const XMFLOAT3& value, const std::string& name, const std::string& typeName)
	{
		char buffer[256];
		sprintf_s(buffer, "name : %s\n"
			"typeName : %s \n"
			"[%f,%f,%f]\n\n",
			name.c_str(), typeName.c_str(), value.x, value.y, value.z);
		OutputDebugStringA(buffer);
	}
	static void ConsoleFLOAT3to60FPS(const XMFLOAT3& value,int frame,const std::string& name = "", const std::string& typeName = "")
	{
		if (frame % 60 == 0)
		{
			char buffer[256];
			sprintf_s(buffer,
				"name :%s \n"
				"typeName : %s \n"
				"[%f,%f,%f]\n\n",
				name.c_str(), typeName.c_str(), value.x, value.y, value.z);
			OutputDebugStringA(buffer);
		}
	}
	static void ConsoleFLOAT4(const XMFLOAT4& value, const std::string& name)
	{
		char buffer[256];
		sprintf_s(buffer, "name : %s\n"
			"[%f,%f,%f,%f]\n\n",
			name.c_str(), value.x, value.y, value.z,value.w);
		OutputDebugStringA(buffer);
	}
};