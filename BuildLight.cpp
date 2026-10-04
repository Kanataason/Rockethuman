#include "ObjectBuilders.h"

#include "GameObject.h"
#include "DirectionalLight.h"

// ライト：平行光源を追加するだけ
void ObjectBuilders::BuildLight(GameObject& obj, const ObjectData& /*data*/, const std::shared_ptr<ModelDatas>& /*model*/)
{
   auto* light = obj.AddComponent<DirectionalLight>();
   light->SetDirection({ 0.0f, -0.7f, 0.6f });
}
