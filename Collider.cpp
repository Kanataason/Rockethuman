#include "Collider.h"
#include "CollisionManager.h"

// 登録済みのCollisionManagerがあれば、自動的に登録を解除する
// (GameObjectがunique_ptrで破棄されるとき、ここで自動的に呼ばれるので、
//  呼び出し側で手動でRemoveを呼び忘れる心配がなくなる)
Collider::~Collider()
{
    if (_registry != nullptr)
    {
        _registry->ReleaseCollider(_id);
    }
}
