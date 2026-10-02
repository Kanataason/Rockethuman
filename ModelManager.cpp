#include "ModelManager.h"

void ModelManager::SetDevice(ID3D11Device* device)
{
	_device = device;
}

std::shared_ptr<ModelDatas> ModelManager::CheckModel(const ObjectData& data)
{
    // Primitive（三角・四角など）はtypeNameでキャッシュ
    if (data.attribute == Attribute::Primitive)
    {
        auto itr = primitives.find(data.typeName);

        if (itr != primitives.end())
        {
            //すでにロードされているものはインデックスとメッシュデータしか使わない
            //マテリアルは個別で使うためnullptrにする
            auto copy = std::make_shared<ModelDatas>();
            for (auto& meshData : itr->second->meshes)
            {
                copy->meshes.push_back({ meshData.mesh, nullptr });
            }
            return copy;
        }

        auto model = _modelLoader.PrimitiveLoadModel(data.typeName);

        // 未対応のObjectTypeだった場合は失敗として返す
        if (!model)
        {
            OutputDebugStringA("PrimitiveLoadModel failed\n");
            return nullptr;
        }

        for (auto& meshData : model->meshes)
        {
            meshData.mesh->CreateMeshInfo(_device);
        }

        primitives[data.typeName] = model;

        return model;
    }

    // FBXモデルはmodelPathでキャッシュ
    auto itr = models.find(data.modelPath);

    if (itr != models.end())
    {
        return itr->second;
    }

    auto model = _modelLoader.FBXLoadModel(data.modelPath);

    // 読み込み失敗（ファイルが無い・壊れているなど）の場合は失敗として返す
    if (!model)
    {
        OutputDebugStringA("FBXLoadModel failed\n");
        return nullptr;
    }

    for (const auto& animationData : data.animationDatas)
    {
        auto animation = _animationLoader.Load(animationData, *model->skeleton);
        model->animations.push_back(animation);
    }

    for (auto& meshData : model->meshes)
    {
        meshData.mesh->CreateMeshInfo(_device);
    }

    models[data.modelPath] = model;

    return model;
}