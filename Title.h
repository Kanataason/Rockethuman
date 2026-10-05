#pragma once
#include "BaseScene.h"
#include "UiRenderer.h"
#include "UiLayout.h"
#include "Event.h"

class InputSystem;

class Title : public BaseScene
{
public:
	void Initialize(ServiceLocator& locator)override;
	void Update(float deltaTime)override;
	void DrawUi()override;
private:

	void ConfirmSelected();
	void DrawMenuItem(MenuEntry menu, int index);
	void DrawPanel(PanelType typeIndex);
private:
	int _selectedIndex = 0;

	PanelType _currentPanel = PanelType::None;

	InputSystem* _input = nullptr;
};