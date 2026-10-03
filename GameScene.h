#pragma once
#include "BaseScene.h"
#include "ScoreManager.h"

class GameScene : public BaseScene
{
public:
	void Initialize(ServiceLocator& locator)override;
	void Update(float deltaTime)override;
	void DrawUi()override;
private:
	ScoreManager _scoreManager;
};