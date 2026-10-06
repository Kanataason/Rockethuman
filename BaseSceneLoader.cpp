#include <ThirdParty/nlohmann/json.hpp>

using json = nlohmann::json;
#include "BaseSceneLoader.h"


#include "BaseScene.h"
#include "BaseGameObjectFactory.h"
#include "ObjectType.h"
#include "CharacterRoster.h"

void SceneLoader::LoadObjects(BaseScene& scene,std::string& loadPath,GameObjectFactory& factory)
{
	//渡したJsonPathのデータを取得
    json root = _jsonLoader.LoadJsonData(loadPath);

	//Jsonから特定のrootをすべて取得
	for (const auto& object : root["objects"])
	{
		auto data = _jsonLoader.LoadGameObjectData(object);

		for (const auto& one : _jsonLoader.ExpandRepeat(data))
			scene.AddObject(factory.CreateObject(one));
	}

}

void SceneLoader::LoadSceneGameObject(BaseScene* scene,const std::string& path,
	JsonLoader& loader,GameObjectFactory& factory,const CharacterRoster* characters)
{
	json root = loader.LoadJsonData(path);

	for (const auto& obj : root.at("Objects"))
	{
		auto data = loader.LoadGameObjectData(obj);

		// プレイヤーは、タイトルで選んだキャラクターに差し替える(位置は、ステージのJSONのまま)
		if (characters && data.typeName == ObjectType::Player)
			characters->ApplyToPlayer(data);

		// "count"で並べる指定があれば、その個数ぶん作る(なければ1個)
		for (const auto& one : loader.ExpandRepeat(data))
			scene->AddObject(factory.CreateObject(one));
	}
	// "Stage" は { ... } のオブジェクト。省略したら、StageDataの初期値のまま
	if (root.contains("Stage") && root["Stage"].is_object())
		scene->SetStageInfo(LoadSceneStageInfo(root["Stage"]));
}

StageData SceneLoader::LoadSceneStageInfo(const json& root)
{
	StageData data{};

	// 書かれていない項目は、StageDataの初期値のまま使う
	data.shotCount = root.value("shotCount", data.shotCount);

	if (root.contains("starThresholds") && root["starThresholds"].is_array()
		&& root["starThresholds"].size() >= 3)
	{
		for (size_t i = 0; i < data.starThresholds.size(); ++i)
			data.starThresholds[i] = root["starThresholds"][i].get<int>();
	}

	return data;
}