#include "BaseGameObjectFactory.h"

#include "BaseScene.h"
#include "MeshRenderer.h"
#include "BaseCamera.h"
#include "DirectionalLight.h"
#include "CameraController.h"
#include "Animator.h"

#include "Mesh.h"
#include "SphereCollider.h"
#include "BoxCollider.h"
#include "RigidBody.h"
#include "PlayerController.h"
#include "CapsuleCollider.h"
#include "Ragdoll.h"

// ObjectTypeごとのコンポーネント構築処理をまとめた関数テーブル
// 新しいObjectTypeを追加する場合はBuild関数を実装し、ここに登録するだけでよい
using ObjectBuilder = void(*)(GameObject&, const ObjectData&, const std::shared_ptr<ModelDatas>&);

static const std::unordered_map<ObjectType, ObjectBuilder> _objectBuilders =
{
    { ObjectType::Box,    &GameObjectFactory::BuildBox },
    { ObjectType::Player, &GameObjectFactory::BuildPlayer },
    { ObjectType::Camera, &GameObjectFactory::BuildCamera },
    { ObjectType::Light,  &GameObjectFactory::BuildLight },
    { ObjectType::Cannon, &GameObjectFactory::BuildCannon}
};

// モデルを取得し、各メッシュにマテリアル/テクスチャを割り当てる
std::shared_ptr<ModelDatas> GameObjectFactory::PrepareModel(const ObjectData& data)
{
    auto model = _modelManager.CheckModel(data);

    // モデル読み込み失敗（未対応のPrimitiveやFBX読み込みエラー）
    if (model == nullptr)
        return nullptr;

    for (auto& mesh : model->meshes)
    {
        if (mesh.material != nullptr)
        {
            // マテリアルにテクスチャパスが指定されていれば読み込んで設定
            auto& path = mesh.material->GetTexturePath();

            if (!path.empty())
            {
                auto texture = _materialManager.FbxCheckTexture(path);

                if (texture != nullptr)
                {
                    mesh.material->SetTexture(texture);
                }
                else
                {
                    OutputDebugStringA("None Texture");
                }
            }
        }
        else
        {
            // マテリアル未設定のメッシュには指定名のマテリアルを割り当てる
            mesh.material = _materialManager.CheckMaterial(data.materialName);
        }
    }

    return model;
}

// ObjectDataからGameObjectを1つ生成する
std::unique_ptr<GameObject> GameObjectFactory::CreateObject(const ObjectData& data)
{
    auto obj = std::make_unique<GameObject>();
    std::shared_ptr<ModelDatas> model;

    // モデルを必要とする属性の場合のみ読み込む
    if (data.attribute != Attribute::Other)
    {
        model = PrepareModel(data);

        // モデルが無いまま組み立てると各Build関数でクラッシュするため、ここで中断
        if (model == nullptr)
        {
            OutputDebugStringA("CreateObject failed: model load error\n");
            return nullptr;
        }
    }

    // typeNameに対応する構築関数をテーブルから探して実行
    auto found = _objectBuilders.find(data.typeName);
    if (found != _objectBuilders.end())
    {
        found->second(*obj, data, model);
    }

    obj->GetTransform() = data.transform;
    return obj;
}

// 静的な箱オブジェクト：見た目 + 衝突用のBoxCollider
void GameObjectFactory::BuildBox(GameObject& obj, const ObjectData& data, const std::shared_ptr<ModelDatas>& model)
{
    auto meshRenderer = obj.AddComponent<MeshRenderer>();
    meshRenderer->SetModel(model);

    auto coll = obj.AddComponent<BoxCollider>();
    coll->SetSize(data.transform.Scale);
    coll->SetCollisionGroup(3);
    coll->SetPosition(data.transform.Position);
}

// プレイヤー：操作・物理・ラグドール・見た目・アニメーションをまとめて構築
void GameObjectFactory::BuildPlayer(GameObject& obj, const ObjectData& data, const std::shared_ptr<ModelDatas>& model)
{
    obj.AddComponent<PlayerController>();

    // 通常時はキネマティックとして扱い、物理演算に押されないようにする
    auto rigi = obj.AddComponent<RigidBody>();
    rigi->EnableKinematic();
    rigi->SetPosition(Vector::fromF3ToV3(data.transform.Position));

    // ラグドール切り替え用にSkeletonと所有者を紐付け
    auto doll = obj.AddComponent<Ragdoll>();
    doll->SetSkeleton(model->skeleton);
    doll->SetParent(obj);

    auto meshRenderer = obj.AddComponent<MeshRenderer>();
    meshRenderer->SetModel(model);

    // アニメーション再生に必要な情報一式を渡す
    auto anim = obj.AddComponent<Animator>();
    anim->SetAnimationInfo(model->animation, model->skeleton, model->animations);
}

// カメラ：視点制御 + 当たり判定用のSphereCollider（物理挙動あり）
void GameObjectFactory::BuildCamera(GameObject& obj, const ObjectData& data, const std::shared_ptr<ModelDatas>& /*model*/)
{
    obj.AddComponent<Camera>();
 //  obj.AddComponent<CameraController>();

    //auto coll = obj.AddComponent<SphereCollider>();
    //coll->SetCollisionGroup(2);
    //coll->SetPosition(data.transform.Position);

    //// カメラは物理挙動で動かすためキネマティックにはしない
    //auto rigi = obj.AddComponent<RigidBody>();
    //rigi->DisableKinematic();
    //rigi->SetPosition(Vector::fromF3ToV3(data.transform.Position));
}

// ライト：平行光源を追加するだけ
void GameObjectFactory::BuildLight(GameObject& obj, const ObjectData& /*data*/, const std::shared_ptr<ModelDatas>& /*model*/)
{
    obj.AddComponent<DirectionalLight>();
}

void GameObjectFactory::BuildCannon(GameObject& obj, const ObjectData& data, const std::shared_ptr<ModelDatas>& model)
{
    auto meshRenderer = obj.AddComponent<MeshRenderer>();
    meshRenderer->SetModel(model);
}