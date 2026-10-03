#pragma once
#include <string>
#include <DirectXMath.h>
#include <vector>
#include <functional>
#include "Transform.h"

enum class ObjectType //Jsonでstring->typeに変換する
{
	Player,
	Enemy,
	Ground,
	Camera,
	Light,
	Box,
	Triangle,
	Quad,
	Cannon,
	ScoreObject,
	None
};
enum class Attribute //図形のタイプ
{
	File,
	Primitive,
	Other,
	None
};

enum class SceneName {
	Title,
	GameScene,
	Result,
	None,
};

struct AnimationData
{
	std::string AnimationName;
	std::string AnimationPath;
};

class BaseScene;

struct SceneData
{
	std::string path = "";
	SceneName sceneName = SceneName::None;
	std::function<std::unique_ptr<BaseScene>()> scene;
};

//Jsonからのデータを入れる構造体
struct ObjectData
{
	Attribute attribute = Attribute::None;

	ObjectType typeName = ObjectType::None;
	std::string modelPath = "";
	std::string materialName = "";
	Transform transform = {};
	DirectX::XMFLOAT3 direction = { 0,0,0 };

	std::vector<AnimationData> animationDatas;
};