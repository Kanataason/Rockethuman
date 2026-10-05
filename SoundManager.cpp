#include "SoundManager.h"

#include <fstream>
#include <exception>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

#include <ThirdParty/nlohmann/json.hpp>

using json = nlohmann::json;

namespace
{
	// JSONのパス(UTF-8)を、SoundEffectが受け取るワイド文字列に変える
	std::wstring ToWide(const std::string& utf8)
	{
		if (utf8.empty())
			return std::wstring();

		const int length = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
		if (length <= 0)
			return std::wstring();

		std::wstring wide(static_cast<size_t>(length), L'\0');
		MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, wide.data(), length);
		wide.resize(static_cast<size_t>(length) - 1);   // 終端の'\0'を除く
		return wide;
	}
}

void SoundManager::Initialize(ServiceLocator& /*locator*/)
{
	DirectX::AUDIO_ENGINE_FLAGS flags = DirectX::AudioEngine_Default;
#ifdef _DEBUG
	flags |= DirectX::AudioEngine_Debug;          // デバッグ時に、詳しいエラーが出る
#endif

	// 音の出力装置が使えない環境でも、ゲームは動くように、失敗したら無音にする
	try
	{
		_engine = std::make_unique<DirectX::AudioEngine>(flags);
	}
	catch (const std::exception&)
	{
		OutputDebugStringA("SoundManager: AudioEngine init failed (no sound)\n");
		_engine.reset();
		return;
	}

	LoadFromJson("Asset/Data/Sound.json");
}

void SoundManager::LoadFromJson(const std::string& jsonPath)
{
	std::ifstream file(jsonPath);
	if (!file.is_open())
	{
		OutputDebugStringA("SoundManager: Sound.json not found\n");
		return;
	}

	// 第3引数 false: 書き方を間違えていても、例外を投げずに、discarded(空)で返す
	const json root = json::parse(file, nullptr, false);
	if (root.is_discarded() || !root.is_object())
	{
		OutputDebugStringA("SoundManager: Sound.json is invalid\n");
		return;
	}

	// { "SE": [ { "name": "shot", "path": "Asset/Sound/shot.wav" }, ... ], "BGM": [ ... ] }
	for (const char* group : { "SE", "BGM" })
	{
		if (!root.contains(group) || !root[group].is_array())
			continue;

		for (const auto& entry : root[group])
		{
			if (!entry.is_object())
				continue;

			const std::string name = entry.value("name", "");
			const std::wstring path = ToWide(entry.value("path", ""));
			if (name.empty() || path.empty())
				continue;

			if (std::string(group) == "SE")
				LoadSe(name, path);
			else
				LoadBgm(name, path);
		}
	}
}

void SoundManager::LoadSe(const std::string& name, const std::wstring& path)
{
	if (!_engine)
		return;

	// ファイルがない・WAV以外のときは、例外が投げられる。ゲームは止めずに、その音だけ、使えないことにする
	try
	{
		_sounds[name] = std::make_unique<DirectX::SoundEffect>(_engine.get(), path.c_str());
	}
	catch (const std::exception&)
	{
		OutputDebugStringW((L"SoundManager: SE load failed: " + path + L"\n").c_str());
	}
}

void SoundManager::LoadBgm(const std::string& name, const std::wstring& path)
{
	if (!_engine)
		return;

	try
	{
		_bgmClips[name] = std::make_unique<DirectX::SoundEffect>(_engine.get(), path.c_str());
	}
	catch (const std::exception&)
	{
		OutputDebugStringW((L"SoundManager: BGM load failed: " + path + L"\n").c_str());
	}
}

void SoundManager::PlaySe(const std::string& name, float volume)
{
	auto it = _sounds.find(name);
	if (it == _sounds.end())
		return;

	it->second->Play(volume, 0.0f, 0.0f);
}

void SoundManager::PlayBgm(const std::string& name, float volume, bool loop)
{
	auto it = _bgmClips.find(name);
	if (it == _bgmClips.end())
		return;

	// 同じ曲が、すでに鳴っていれば、そのまま続ける
	if (_bgm && _currentBgmName == name && _bgm->GetState() == DirectX::PLAYING)
		return;

	if (_bgm)
		_bgm->Stop();

	// 元のSoundEffect(it->second)は、_bgmClipsが持ち続けるので、インスタンスは、安全に使える
	_bgm = it->second->CreateInstance();
	_bgm->SetVolume(volume);
	_bgm->Play(loop);
	_currentBgmName = name;
}

void SoundManager::StopBgm()
{
	if (_bgm)
		_bgm->Stop();
	_currentBgmName.clear();
}
