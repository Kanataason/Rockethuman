#include "GameObject.h"
#include "Renderer.h"
#include "ServiceLoacator.h"

//指定した型のComponentを検索してコンポーネントに通知を送る処理


void GameObject::SetOrder(int order)
{

}

void GameObject::OnCollisionEnter(Collider* other,const CollisionResult& result)
{
	for (auto& component : _components)
	{
		component->OnCollisionEnter(other, result);
	}
}
void GameObject::OnCollisionStay(Collider* other,const CollisionResult& result)
{
	for (auto& component : _components)
		component->OnCollisionStay(other, result);
}
void GameObject::OnCollisionExit(Collider* other,const CollisionResult& result)
{
	for (auto& component : _components)
		component->OnCollisionExit(other, result);
}


void GameObject::Update(float deltaTime)
{
	for (auto& component : _components)
	{
		component->Update(deltaTime);
	}

}
void GameObject::Draw(Renderer& renderer)
{
	for (auto& component : _components)
	{
		component->Draw(renderer);
	}
}
void GameObject::Initialize(ServiceLocator& locator)
{
	for (auto& component : _components)
	{
		component->Initialize(locator);
	}

}

void GameObject::Release()
{
	for (auto& component : _components)
	{
		component->Release();
	}
}
