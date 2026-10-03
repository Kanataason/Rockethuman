#pragma once
#include "BaseScene.h"

class GameScene : public BaseScene
{
public:
	void Initialize(ServiceLocator& locator)override
	{
		BaseScene::Initialize(locator); 
	};
	void Update(float deltaTime)override
	{
		BaseScene::Update(deltaTime);
	};
};