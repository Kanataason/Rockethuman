#include "CharacterRoster.h"

#include <fstream>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

#include <ThirdParty/nlohmann/json.hpp>

#include "StringUtility.h"

using json = nlohmann::json;

void CharacterRoster::Load(const std::string& jsonPath)
{
	_characters.clear();
	_selectedIndex = 0;

	std::ifstream file(jsonPath);
	if (!file.is_open())
	{
		OutputDebugStringA("CharacterRoster: Characters.json not found\n");
		return;
	}

	// 第3引数 false: 書き方を間違えていても、例外を投げずに、discarded(空)で返す
	const json root = json::parse(file, nullptr, false);
	if (root.is_discarded() || !root.is_object() || !root.contains("Characters") || !root["Characters"].is_array())
	{
		OutputDebugStringA("CharacterRoster: Characters.json is invalid\n");
		return;
	}

	for (const auto& entry : root["Characters"])
	{
		if (!entry.is_object())
			continue;

		CharacterData character;
		character.modelPath = entry.value("path", "");
		if (character.modelPath.empty())
			continue;   // モデルのないキャラクターは、選べても出せないので、飛ばす

		character.name = StringUtility::ToWide(entry.value("name", character.modelPath));
		character.scale = entry.value("scale", character.scale);
		character.colliderScale = entry.value("colliderScale", character.colliderScale);
		character.bounceRestitution = entry.value("bounceRestitution", character.bounceRestitution);
		character.bounceCount = entry.value("bounceCount", character.bounceCount);

		if (entry.contains("animations") && entry["animations"].is_array())
		{
			for (const auto& animation : entry["animations"])
			{
				if (!animation.is_object())
					continue;

				const std::string name = animation.value("name", "");
				const std::string path = animation.value("path", "");
				if (!name.empty() && !path.empty())
					character.animations.push_back({ name, path });
			}
		}

		_characters.push_back(std::move(character));
	}
}

const CharacterData* CharacterRoster::GetSelected() const
{
	if (_characters.empty())
		return nullptr;
	return &_characters[static_cast<size_t>(_selectedIndex)];
}

void CharacterRoster::SelectNext()
{
	if (_characters.empty())
		return;
	_selectedIndex = (_selectedIndex + 1) % GetCount();
}

void CharacterRoster::SelectPrevious()
{
	if (_characters.empty())
		return;
	_selectedIndex = (_selectedIndex + GetCount() - 1) % GetCount();
}

void CharacterRoster::ApplyToPlayer(ObjectData& playerData) const
{
	const CharacterData* character = GetSelected();
	if (!character)
		return;

	playerData.attribute = Attribute::File;
	playerData.modelPath = character->modelPath;
	playerData.transform.Scale = { character->scale, character->scale, character->scale };
	playerData.animationDatas = character->animations;
	playerData.colliderScale = character->colliderScale;
	playerData.bounceRestitution = character->bounceRestitution;
	playerData.bounceCount = character->bounceCount;
}
