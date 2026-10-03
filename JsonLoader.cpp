#include <fstream>

#include "JsonLoader.h"

namespace
{
	// JSON上の文字列とObjectTypeの対応表
	const std::unordered_map<std::string, ObjectType> ObjectTypeTable =
	{
		{"Prop",     ObjectType::Prop},
		{"Player",   ObjectType::Player},
		{"Light",    ObjectType::Light},
		{"Camera",   ObjectType::Camera},
		{"Cannon",   ObjectType::Cannon},
		{"ScoreObject", ObjectType::ScoreObject}
	};

	// JSON上の文字列とPrimitiveShapeの対応表
	const std::unordered_map<std::string, PrimitiveShape> PrimitiveShapeTable =
	{
		{"Box",            PrimitiveShape::Box},
		{"BoxBottomPivot", PrimitiveShape::BoxBottomPivot},
		{"Quad",           PrimitiveShape::Quad},
		{"Triangle",       PrimitiveShape::Triangle}
	};

	// JSON上の文字列とAttributeの対応表
	const std::unordered_map<std::string, Attribute> ObjectAttributeTable =
	{
		{"File",      Attribute::File},
		{"Primitive", Attribute::Primitive},
		{"Other",     Attribute::Other}
	};
	const std::unordered_map<std::string, SceneName> SceneNameTable =
	{
		{"Title", SceneName::Title},
		{"GameScene",SceneName::GameScene},
		{"Result",SceneName::Result}
	};
}

json JsonLoader::LoadJsonData(const std::string& assetPath)const
{
	// JSONファイルを開いて内容を読み込む
	std::ifstream file(assetPath);

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open json file.");
	}

	json j;
	file >> j;

	return j;
}
ObjectType JsonLoader::LoadObjectType(const json& root)const
{
	if (!root.contains("type") || !root["type"].is_string())
	{
		return ObjectType::None;
	}

	return ToObjectType(root["type"].get<std::string>());
}
PrimitiveShape JsonLoader::LoadPrimitiveShape(const json& root)const
{
	// 省略したときはBox
	if (!root.contains("shape") || !root["shape"].is_string())
	{
		return PrimitiveShape::Box;
	}

	return ToPrimitiveShape(root["shape"].get<std::string>());
}
Attribute JsonLoader::LoadObjectAttribute(const json& root)const
{
    if (!root.contains("attribute") || !root["attribute"].is_string())
    {
        return Attribute::None;
    }

    return ToObjectAttribute(root["attribute"].get<std::string>());
}
Transform JsonLoader::LoadTransform(const json& root)
{
	Transform transform{};

	// Position [x, y, z] を読み込む
	if (root.contains("position") &&
		root["position"].is_array() &&
		root["position"].size() >= 3)
	{
		transform.Position.x = root["position"][0].get<float>();
		transform.Position.y = root["position"][1].get<float>();
		transform.Position.z = root["position"][2].get<float>();
	}

	// Scale [x, y, z] を読み込む
	if (root.contains("scale") &&
		root["scale"].is_array() &&
		root["scale"].size() >= 3)
	{
		transform.Scale.x = root["scale"][0].get<float>();
		transform.Scale.y = root["scale"][1].get<float>();
		transform.Scale.z = root["scale"][2].get<float>();
	}

	return transform;
}
std::string JsonLoader::LoadPath(const json& root)const
{
	return root.value("path", "");
}
std::string JsonLoader::LoadMaterial(const json& root) const
{
	return root.value("material", "");
}
void JsonLoader::LoadAnimationPath(const json& root, std::vector<AnimationData>& animationPath)
{
	if (!root.contains("animations") ||!root["animations"].is_array())
	{
		return;
	}

	for (const auto& animation : root["animations"])
	{
		if (!animation.is_object())
		{
			continue;
		}

		const std::string name =animation.value("name", "");

		const std::string path =animation.value("path", "");

		// 名前またはパスが無効なアニメーションは登録しない
		if (name.empty() || path.empty())
		{
			continue;
		}

		animationPath.push_back({
			name,
			path
			});
	}
}
SceneData JsonLoader::LoadSceneData(const json& root)const
{
	SceneData data{};

	if (root.contains("SceneName"))
		data.sceneName = ToSceneName(root.at("SceneName").get<std::string>());

	if (root.contains("ScenePath"))
		data.path = root.at("ScenePath").get<std::string>();

	return data;
}

ObjectData JsonLoader::LoadGameObjectData(const json& root)
{
	ObjectData data = {};

	data.typeName = LoadObjectType(root);
	data.shape = LoadPrimitiveShape(root);
	data.attribute = LoadObjectAttribute(root);
	data.modelPath = LoadPath(root);
	data.transform = LoadTransform(root);
	data.materialName = LoadMaterial(root);
	
	LoadAnimationPath(root,data.animationDatas);

	return data;
}

//分ける
Attribute JsonLoader::ToObjectAttribute(const std::string& attribute)const
{
	auto it = ObjectAttributeTable.find(attribute);

	if (it != ObjectAttributeTable.end())
	{
		return it->second;
	}

	throw std::runtime_error(
		"Unknown ObjectAttribute: " + attribute
	);
}


ObjectType JsonLoader::ToObjectType(const std::string& type)const
{
	auto it = ObjectTypeTable.find(type);

	if (it != ObjectTypeTable.end())
	{
		return it->second;
	}

	throw std::runtime_error(
		"Unknown ObjectType: " + type
	);
}
PrimitiveShape JsonLoader::ToPrimitiveShape(const std::string& shape)const
{
	auto it = PrimitiveShapeTable.find(shape);

	if (it != PrimitiveShapeTable.end())
	{
		return it->second;
	}

	throw std::runtime_error(
		"Unknown PrimitiveShape: " + shape
	);
}
SceneName JsonLoader::ToSceneName(const std::string& name)const
{
	auto it = SceneNameTable.find(name);

	if (it != SceneNameTable.end())
	{
		return it->second;
	}

	throw std::runtime_error(
		"Unknown SceneName: " + name
	);
}