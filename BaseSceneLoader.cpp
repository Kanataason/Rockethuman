#include <ThirdParty/nlohmann/json.hpp>

using json = nlohmann::json;
#include "BaseSceneLoader.h"


#include "BaseScene.h"
#include "BaseGameObjectFactory.h"
#include "ObjectType.h"

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
	JsonLoader& loader,GameObjectFactory& factory)
{
	json root = loader.LoadJsonData(path);

	for (const auto& obj : root.at("Objects"))
	{
		auto data = loader.LoadGameObjectData(obj);

		// "count"で並べる指定があれば、その個数ぶん作る(なければ1個)
		for (const auto& one : loader.ExpandRepeat(data))
			scene->AddObject(factory.CreateObject(one));
	}
}