#pragma once
#include <string>
#include <vector>

#include "JsonLoader.h"

class BaseScene;
class GameObjectFactory;

class SceneLoader
{
public :
	void LoadObjects(BaseScene& scene, std::string& loadPath, GameObjectFactory& factory);

	void LoadSceneGameObject(BaseScene* scene,const std::string& loadPath,
		JsonLoader& loader,GameObjectFactory& factory);
private:
	StageData LoadSceneStageInfo(const json& root);
private :
	JsonLoader _jsonLoader;
};