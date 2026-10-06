#pragma once
#include <memory>

#include "ServiceLocator.h"
#include "CollisionResult.h"

class Collider;
class GameObject;
class Renderer;

//GameObjectに持たせる前提の設計
class Component
{
public:
    virtual ~Component() {}

    virtual void Initialize(ServiceLocator& locator) {}
    virtual void Update(float deltaTime) {}
    virtual void Draw(Renderer& renderer) {}
    virtual void Release() {}

    //当たり判定処理(otherは借りるだけ。Colliderの所有者は、相手のGameObject)
    virtual void OnCollisionEnter(Collider* other, const CollisionResult& result) {}
    virtual void OnCollisionStay(Collider* other, const CollisionResult& result) {}
    virtual void OnCollisionExit(Collider* other, const CollisionResult& result) {}

    GameObject* GetOwner() const
    {
        return _owner;
    }
    void SetOwner(GameObject* owner)
    {
        _owner = owner;
    }

protected:
    GameObject* _owner = nullptr;
};