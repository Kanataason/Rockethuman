#include "BaseScene.h"

#include "ObjectType.h"
#include "RigidBody.h"

#include "DebagUtility.h"
#include "PhysicsSystem.h"
#include "GameObject.h"
#include "BaseLight.h"
#include "BaseCamera.h"

using namespace DirectX;


void BaseScene::Draw(Renderer& renderer)
{
	//描写処理にカメラの情報を渡す
	if(_camera)
    renderer.SetCamera(*_camera);

	//オブジェクトごとに描画をする
	for (const auto& obj : _objects)
	{
		if (auto light = obj->GetComponent<Light>())
		{
			renderer.SetLight(*light);
		}
		obj->Draw(renderer);
	}
	//デバッグの線描写
	//debug->Render(*_camera);
}

void BaseScene::Initialize(ServiceLocator& context)//シーンの情報初期化
{
	_physicsSystem = context.physicsSystem;
	_sceneManager = context.sceneManager;
	SetObjectInfo(context);
}
void BaseScene::SetObjectInfo(ServiceLocator& context)
{
	for (const auto& obj : _objects)
	{
		obj->Initialize(context);

		auto rigid = obj->GetComponent<RigidBody>();
		auto collider = obj->GetComponent<Collider>();

		//物理システムのほうに当たり判定と物理の処理をセットで送る
		if (rigid || collider)
		{
			PhysicsBody body;

			body.rigidBody = rigid;
			body.collider = collider;

			_physicsSystem->AddPhysicsBody(body);
		}
		
		if (auto light = obj->GetComponent<Light>())
			_lights.push_back(light);

		if (auto camera = obj->GetComponent<Camera>())
			_camera = camera;
	}
}

void BaseScene::Update(float deltaTime)
{
	for (const auto& obj : _objects)
	{
		obj->Update(deltaTime);
	}
}
void BaseScene::Release()
{
	for (const auto& obj : _objects)
	{
		obj->Release();
	}
}
void BaseScene::Reset()
{
	for (const auto& obj : _objects)
	{
		auto rigid = obj->GetComponent<RigidBody>();
		auto collider = obj->GetComponent<Collider>();
		_physicsSystem->ReleasePhysicsBody(rigid, collider);
	}
}

void BaseScene::AddObject(std::unique_ptr<GameObject> object)
{
	_objects.push_back(std::move(object));
}