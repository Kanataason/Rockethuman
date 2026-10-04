#pragma once
#include "BaseScene.h"
#include "ScoreManager.h"
#include "RoundManager.h"
#include "UIRenderer.h"
#include "UiTween.h"

class Ragdoll;
class CameraFollow;

class GameScene : public BaseScene
{
public:
	void Initialize(ServiceLocator& locator)override;
	void Update(float deltaTime)override;
	void DrawUi()override;
private:
	void SubscribeEvents();

	void DrawPanelBase(const PanelData& panel);
	void UpdateScorePanel(const PanelData& panel);
	void DrawShotCountPanel();
private:
	RoundManager _roundManager;
	ScoreManager _scoreManager;

	Ragdoll* _ragdoll = nullptr;
	CameraFollow* _cameraFollow = nullptr;

	// 残りの玉数が、ポンと弾むアニメーション(ラグドールが止まったとき)
	Tween _shotCountPop;
};
