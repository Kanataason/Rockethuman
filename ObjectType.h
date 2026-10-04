#pragma once
#include <string>
#include <DirectXMath.h>
#include <vector>
#include <functional>
#include "Transform.h"

//役割(どのBuild関数で組み立てるか)。形(メッシュ)はPrimitiveShapeで別に指定する
enum class ObjectType //Jsonでstring->typeに変換する
{
	Player,
	Enemy,
	Ground,
	Camera,
	Light,
	Prop,//見た目と当たり判定だけの置き物
	Cannon,
	ScoreObject,
	None
};

//Attribute::Primitiveのときの形(Jsonの"shape"。省略するとBox)
enum class PrimitiveShape
{
	Box,
	BoxBottomPivot,//下端の中央が原点の箱(底を支点に回したい物に使う)
	Quad,
	Triangle,
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
	PrimitiveShape shape = PrimitiveShape::Box;
	std::string modelPath = "";
	std::string materialName = "";
	Transform transform = {};
	DirectX::XMFLOAT3 direction = { 0,0,0 };
	float score = 100.0f;   // 的の基本点(Jsonの"score"。省略時は100)

	std::vector<AnimationData> animationDatas;
};