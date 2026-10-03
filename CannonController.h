#pragma once
#include "Component.h"

class InputSystem;

class CannonController : public Component
{
public:
	void Initialize(ServiceLocator& locator)override;
	void Update(float deltaTime)override;
private:
	void RotationCannon(float deltaTime);
private:
	InputSystem* _input = nullptr;

	float _swingFrame = 0.0f;//現在の回転値はラジアン
	float _swingSpeed = 0.5f;
	float _swingMinAngle = -120.0f;
	float _swingMaxAngle = 120.0f;
};