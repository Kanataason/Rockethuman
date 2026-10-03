#include "Collider.h"
#include "PhysicsSystem.h"

// 登録済みのPhysicsSystemがあれば、自動的に登録を解除する
// (GameObjectがunique_ptrで破棄されるとき、ここで自動的に呼ばれるので、
//  呼び出し側で手動でRemoveを呼び忘れる心配がなくなる)
Collider::~Collider()
{
    if (_registry != nullptr)
    {
        _registry->RemoveCollider(this);
    }
}
