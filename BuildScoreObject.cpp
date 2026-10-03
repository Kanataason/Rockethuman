#include "ObjectBuilders.h"

#include "GameObject.h"
#include "ObjectStatus.h"
#include "MeshRenderer.h"
#include "RigidBody.h"
#include "BoxCollider.h"

// 点数が入る物(的)：状態 + 見た目 + 衝突用のBoxCollider
void ObjectBuilders::BuildScoreObject(GameObject& obj, const ObjectData& data, const std::shared_ptr<ModelDatas>& model)
{
    obj.AddComponent<ObjectStatus>();
    auto meshRenderer = obj.AddComponent<MeshRenderer>();
    meshRenderer->SetModel(model);

    auto coll = obj.AddComponent<BoxCollider>();
    coll->SetSize(data.transform.Scale);
    coll->SetCollisionGroup(CollisionGroup::Target);
    coll->SetPosition(data.transform.Position);

    //obj.AddComponent<RigidBody>();
}
