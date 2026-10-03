#include "GameScene.h"

#include "CannonController.h"
#include "PlayerController.h"
#include "ObjectStatus.h"

void GameScene::Initialize(ServiceLocator& locator)
{
	BaseScene::Initialize(locator);

	auto* cannon = FindGameObject<CannonController>();
	auto* player = FindGameObject<PlayerController>();

	if (cannon)
		cannon->SetInitializePlayer(player);

	for (const auto& obj : _objects)
	{
		if (auto* component = obj->GetComponent<ObjectStatus>())
			_scoreManager.SubScribeEvent(component);
	}
}

void GameScene::Update(float deltaTime)
{
	BaseScene::Update(deltaTime);
}
void GameScene::DrawUi()
{

}