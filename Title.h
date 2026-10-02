#pragma once
#include "BaseScene.h"


class InputSystem;
class UIRenderer;

class Title : public BaseScene
{
public:
	void Initialize(ServiceLocator& locator)override;
	void Update(float deltaTime)override;
	void DrawUi()override;
private:
	void UpdateStartButton(const POINT& mousePos);
	void DrawStartButton();
private:
	bool _isStartButtonHovered = false;

	InputSystem* _input = nullptr;
	UIRenderer* _renderer;
};