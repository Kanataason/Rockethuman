#include "ObjectBuilders.h"

#include "GameObject.h"
#include "MeshRenderer.h"
#include "EffectParticle.h"

//エフェクトの粒：見た目だけ(物理も、当たり判定も持たない)
void ObjectBuilders::BuildEffect(GameObject& obj, const ObjectData& data, const std::shared_ptr<ModelDatas>& model)
{
    auto meshRenderer = obj.AddComponent<MeshRenderer>();
    meshRenderer->SetModel(model);

    obj.AddComponent<EffectParticle>();
}
