#pragma once
#include <Audio.h>
#include <memory>
#include <string>
#include <unordered_map>

#include "ServiceLoacator.h"

// 効果音(SE)とBGM。DirectXTK Audioを使うので、読み込めるのは WAV だけ
// ・Asset/Data/Sound.json の「名前 → ファイル」を、Initializeで読み込む
// ・鳴らすときは、名前を指定する。名前がない・読み込みに失敗した音は、何もせず無視する(落ちない)
class SoundManager
{
public:
    void Initialize(ServiceLocator& locator);

    // 毎フレーム呼ぶ(呼ばないと、ヘッドホンの抜き差しなどに対応できない)
    void Update() { if (_engine) _engine->Update(); }

    void LoadSe(const std::string& name, const std::wstring& path);
    void LoadBgm(const std::string& name, const std::wstring& path);

    void PlaySe(const std::string& name, float volume = 1.0f);

    // すでに同じBGMが鳴っているときは、何もしない(「もういちど」で、曲が頭に戻らないように)
    void PlayBgm(const std::string& name, float volume = 0.4f, bool loop = true);
    void StopBgm();

private:
    void LoadFromJson(const std::string& jsonPath);

private:
    std::unique_ptr<DirectX::AudioEngine> _engine;

    // 効果音と、BGMの元データ。インスタンスが生きている間は、元のSoundEffectも生きていなければならないので、持ち続ける
    std::unordered_map<std::string, std::unique_ptr<DirectX::SoundEffect>> _sounds;
    std::unordered_map<std::string, std::unique_ptr<DirectX::SoundEffect>> _bgmClips;

    std::unique_ptr<DirectX::SoundEffectInstance> _bgm;   // いま鳴っているBGM
    std::string _currentBgmName;
};
