#include <ThirdParty/nlohmann/json.hpp>

#include "BaseSceneLoader.h"


#include "BaseScene.h"
#include "BaseGameObjectFactory.h"
#include "ObjectType.h"
#include "CharacterRoster.h"

using json = nlohmann::json;

void SceneLoader::LoadSceneGameObject(BaseScene* scene,const std::string& path,
	JsonLoader& loader,GameObjectFactory& factory,const CharacterRoster* characters)
{
	json root = loader.LoadJsonData(path);

	for (const auto& obj : root.at("Objects"))
	{
		auto data = loader.LoadGameObjectData(obj);

		//プレイヤーは、タイトルで選んだキャラクターに差し替える
		if (characters && data.typeName == ObjectType::Player)
			characters->ApplyToPlayer(data);

		//並べる指定があれば、その個数ぶん作る(なければ1個)
		for (const auto& one : loader.ExpandRepeat(data))
			scene->AddObject(factory.CreateObject(one));
	}

	if (root.contains("Stage") && root["Stage"].is_object())
		scene->SetStageInfo(LoadSceneStageInfo(root["Stage"]));
}

StageData SceneLoader::LoadSceneStageInfo(const json& root)
{
	StageData data{};

	//書かれていない項目は、StageDataの初期値のまま使う
	data.shotCount = root.value("shotCount", data.shotCount);

	if (root.contains("starThresholds") && root["starThresholds"].is_array()
		&& root["starThresholds"].size() >= 3)
	{
		for (size_t i = 0; i < data.starThresholds.size(); ++i)
			data.starThresholds[i] = root["starThresholds"][i].get<int>();
	}

	return data;
}