#pragma once
#include <vector>
#include <iostream>
#include <string>

#include "Renderer.h"
#include "BaseSceneLoader.h"
#include "BaseGameObjectFactory.h"

#include "ServiceLoacator.h"

class PhysicsSystem;
class DebugDraw;
class GameObject;
class Camera;
class Light;
class SceneManager;

class BaseScene
{
public:
	BaseScene() = default;
	virtual ~BaseScene() = default;

	virtual void Initialize(ServiceLocator& context);
	virtual void Update(float deltaTime);
	virtual void Draw(Renderer& renderer);
	virtual void DrawUi() {};
	// 所有しているComponentを解放する(unique_ptrの破棄で、RigidBody/Colliderの登録解除も自動的に行われる)
	virtual void Release();

	//シーン上のオブジェクトを追加
	void AddObject(std::unique_ptr<GameObject> objectList) ;

	//シーン上にあるオブジェクトを比べて指定したオブジェクトを返す
	//とても重いのでInitializeとかでしか使用しない
	template<typename T>
	T* FindGameObject()const
	{
		for (const auto& object : _objects)
			if (auto* component = object->GetComponent<T>())
				return component;
		return nullptr;
	}

	const virtual std::vector<std::unique_ptr<GameObject>>& GetObjectList() const { return _objects; };
private:
	//各オブジェクトを初期化
	void SetObjectInfo(ServiceLocator& context);
protected:
    PhysicsSystem* _physicsSystem = nullptr;
	SceneManager* _sceneManager = nullptr;

	Camera* _camera = nullptr;

	std::vector<Light*> _lights;
	std::vector<std::unique_ptr<GameObject>> _objects;

};