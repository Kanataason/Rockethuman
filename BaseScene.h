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
	virtual void Release();
	virtual void Reset();

	//シーン上のオブジェクトを追加
	void AddObject(std::unique_ptr<GameObject> objectList) ;

	const virtual std::vector<std::unique_ptr<GameObject>>& GetObjectList() const { return _objects; };
private:
	//各オブジェクトを初期化
	void SetObjectInfo(ServiceLocator& context);
protected:
    PhysicsSystem* _physicsSystem = nullptr;
	SceneManager* _sceneManager = nullptr;

	std::shared_ptr<Camera>	_camera = nullptr;

	std::vector<std::shared_ptr<Light>> _lights;
	std::vector<std::unique_ptr<GameObject>> _objects;

};