#pragma once
#include <memory>

#include "ModelInfo.h"
#include "ObjectType.h"
#include "GameObject.h"
#include "ModelManager.h"
#include "MaterialManager.h"
#include "ModelProvider.h"
using namespace DirectX;


// JSONのデータ(ObjectData)から、GameObjectを1つ作る
//  1. モデルの用意        -> ModelProvider
//  2. 役割ごとの組み立て  -> ObjectBuilders(Build〇〇.cpp)
class GameObjectFactory
{
public:
	GameObjectFactory(ModelManager& modelManager,MaterialManager& materialManager):
		_modelProvider(modelManager, materialManager){}

	//GameObjectを生成する
	std::unique_ptr<GameObject> CreateObject(const ObjectData& data);

private:
	ModelProvider _modelProvider;
};
