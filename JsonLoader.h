#pragma once
#include <ThirdParty/nlohmann/json.hpp>
#include <string>
#include <vector>
#include <DirectXMath.h>
#include <unordered_map>

using json = nlohmann::json;
#include "ObjectType.h"
#include "Transform.h"

using namespace DirectX;

class JsonLoader
{
public :
	json LoadJsonData(const std::string& assetPath)const;

	ObjectData LoadGameObjectData(const json& root);

	SceneData LoadSceneData(const json& root)const;
private:

	//各Jsonから値を取り出す
	ObjectType LoadObjectType(const json& root)const;
	Attribute LoadObjectAttribute(const json& root)const;
	std::string LoadPath(const json& root)const;
	Transform LoadTransform(const json& root);
	std::string LoadMaterial(const json& root)const;
	void LoadAnimationPath(const json& root, std::vector<AnimationData>& animationPaths);

	//取り出した値を変換する
	ObjectType ToObjectType(const std::string& type)const;
	Attribute ToObjectAttribute(const std::string& attribute)const;
	SceneName ToSceneName(const std::string& name)const;
};