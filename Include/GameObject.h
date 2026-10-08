#pragma once
#include <iostream>
#include <vector>
#include <type_traits>

#include "Object.h"
#include "Component.h"
#include "CollisionResult.h"

class Collider;
class Renderer;

class GameObject:public Object
{
public :
    //指定した型のComponentを生成してGameObjectに追加する
    //Componentの所有者はGameObjectだけ(unique_ptr)。呼び出し側には、借りるだけの生ポインタを返す
    template<class T>
        requires std::is_base_of_v<Component, T>
    T* AddComponent()
    {
        auto component = std::make_unique<T>();

        //Componentの所有者を自身に設定
        component->SetOwner(this);

        T* result = component.get();
        _components.push_back(std::move(component));

        return result;
    }

    //指定した型のComponentを検索して取得する
    template<typename T>
        requires std::is_base_of_v<Component, T>
    T* GetComponent()
    {
        for (auto& component : _components)
        {
            T* ptr = dynamic_cast<T*>(component.get());

            if (ptr)
                return ptr;
        }

        return nullptr;
    }

    ~GameObject() = default;

    //衝突が開始したときに呼び出される
    void OnCollisionEnter(
        Collider* other,
        const CollisionResult& result);

    //衝突が継続している間、毎フレーム呼び出される
    void OnCollisionStay(
        Collider* other,
        const CollisionResult& result);

    //衝突しなくなったときに呼び出される
    void OnCollisionExit(
        Collider* other,
        const CollisionResult& result);

    //ServiceLocatorから各システムを取得して初期化する
    void Initialize(ServiceLocator& context);

    //所有しているComponentを更新する
    void Update(float deltaTime) override;

    //所有しているComponentを描画する
    void Draw(Renderer& renderer);

    //描画順を取得する
    int GetOrder() const
    {
        return rendererOrder;
    }

    //所有しているComponentを解放する
    void Release();

    //描画順を設定する
    void SetOrder(int order);


private:
    //GameObjectが所有するComponent(所有者はGameObjectだけ)
    std::vector<std::unique_ptr<Component>> _components;

    //Rendererによる描画順
    int rendererOrder = 0;
};