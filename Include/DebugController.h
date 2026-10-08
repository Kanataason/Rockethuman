#pragma once
#include <iostream>
#include "Component.h"

class InputSystem;
class Ragdoll;

class DebugController:public Component
{
public:
	~DebugController() noexcept override = default;

	void Initialize(ServiceLocator& locator)override;
	void Update(float deltaTime)override;
private:
	InputSystem* _input;
	Ragdoll* _dragDoll = nullptr;
};