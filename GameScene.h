#pragma once
#include "BaseScene.h"
#include "ScoreManager.h"
#include "RoundManager.h"
#include "UIRenderer.h"

class GameScene : public BaseScene
{
public:
	void Initialize(ServiceLocator& locator)override;
	void Update(float deltaTime)override;
	void DrawUi()override;
private:
	void DrawPanelBase(const PanelData& panel);
	void UpdateScorePanel(const PanelData& panel);
	void DrawShotCountPanel();
private:
	RoundManager _roundManager;
	ScoreManager _scoreManager;
};