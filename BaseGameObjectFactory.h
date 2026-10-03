#pragma once
#include <unordered_map>
#include <functional>
#include <iostream>

#include "ModelInfo.h"
#include "ObjectType.h"
#include "GameObject.h"
#include "ModelManager.h"
#include "MaterialManager.h"
using namespace DirectX;


class GameObjectFactory
{
public:
	GameObjectFactory(ModelManager& modelManager,MaterialManager& materialManager):
		_modelManager(modelManager),_materialManager(materialManager){}

	//GameObjectを生成する
	std::unique_ptr<GameObject> CreateObject(const ObjectData& data);

	//モデルを読み取る
	std::shared_ptr<ModelDatas> PrepareModel(const ObjectData& data);


	//関数テーブル用の関数
	static void BuildBox(GameObject& obj, const ObjectData& data, const std::shared_ptr<ModelDatas>& model);
	static void BuildPlayer(GameObject& obj, const ObjectData& data, const std::shared_ptr<ModelDatas>& model);
	static void BuildCamera(GameObject& obj, const ObjectData& data, const std::shared_ptr<ModelDatas>& model);
	static void BuildLight(GameObject& obj, const ObjectData& data, const std::shared_ptr<ModelDatas>& model);
	static void BuildCannon(GameObject& obj, const ObjectData& data, const std::shared_ptr<ModelDatas>& model);
private:
	ModelManager& _modelManager;
	MaterialManager& _materialManager;
};