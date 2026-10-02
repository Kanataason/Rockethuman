#pragma once
#include <iostream>
#include "Component.h"

class InputSystem;
class Ragdoll;

class DebugController:public Component
{
public:
	~DebugController() noexcept override = default;

	void Initialize(ServiceLocator& loacator)override;
	void Update(float deltaTime)override;
private:
	InputSystem* _input;
	std::shared_ptr<Ragdoll> _dragDoll;
};