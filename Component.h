#pragma once
#include <memory>

#include "ServiceLoacator.h"
#include "CollisionResult.h"

class Collider;
class GameObject;
class Renderer;

//GameObjectに持たせる前提の設計
class Component
{
public:
    virtual ~Component() {}

    virtual void Initialize(ServiceLocator& context) {}
    virtual void Update(float deltaTime) {}
    virtual void Draw(Renderer& renderer) {}
    virtual void Release() {}

    //当たり判定処理
    virtual void OnCollisionEnter(std::shared_ptr<Collider>& other, const CollisionResult& result) {}
    virtual void OnCollisionStay(std::shared_ptr<Collider>& other, const CollisionResult& result) {}
    virtual void OnCollisionExit(std::shared_ptr<Collider>& other, const CollisionResult& result) {}

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