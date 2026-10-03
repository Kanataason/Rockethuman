#include "GameScene.h"

#include "CannonController.h"
#include "PlayerController.h"

void GameScene::Initialize(ServiceLocator& locator)
{
	BaseScene::Initialize(locator);

	auto* cannon = FindGameObject<CannonController>();
	auto* player = FindGameObject<PlayerController>();

	if (cannon)
		cannon->SetPlayer(player);
}

void GameScene::Update(float deltaTime)
{
	BaseScene::Update(deltaTime);
}