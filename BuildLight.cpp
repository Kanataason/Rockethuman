#include "ObjectBuilders.h"

#include "GameObject.h"
#include "DirectionalLight.h"

// ライト：平行光源を追加するだけ
void ObjectBuilders::BuildLight(GameObject& obj, const ObjectData& /*data*/, const std::shared_ptr<ModelDatas>& /*model*/)
{
    obj.AddComponent<DirectionalLight>();
}
