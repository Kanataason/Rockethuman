#pragma once
#include "Component.h"

class InputSystem;
class PlayerController;

class CannonController : public Component
{
public:
	void Initialize(ServiceLocator& locator)override;
	void Update(float deltaTime)override;

	void SetPlayer(PlayerController* controller) { _player = controller; SetPlayerPosition(); }
private:
	void SetPlayerPosition();
	void RotationCannon(float deltaTime);
private:
	InputSystem* _input = nullptr;
	PlayerController* _player = nullptr;

	float _swingFrame = 0.0f;//現在の回転値はラジアン
	float _swingSpeed = 1.5f;
	float _swingMinAngle = -120.0f;
	float _swingMaxAngle = 120.0f;
};