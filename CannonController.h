#pragma once
#include "Component.h"

class InputSystem;
class PlayerController;
class Ragdoll;

class CannonController : public Component
{
public:
	void Initialize(ServiceLocator& locator)override;
	void Update(float deltaTime)override;

	void AddForcePlayer(float deltaTime);
	void SetInitializePlayer(PlayerController* controller);
private:
	void SetPlayerPosition();
	void RotationCannon(float deltaTime);
private:
	InputSystem* _input = nullptr;
	PlayerController* _player = nullptr;
	Ragdoll* _ragdoll = nullptr;

	float _swingFrame = 0.0f;//現在の回転値はラジアン
	float _swingSpeed = 1.0f;
	float _swingMinAngle = -80.0f;
	float _swingMaxAngle = 80.0f;
};