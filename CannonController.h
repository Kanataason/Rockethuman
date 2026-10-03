#pragma once
#include "Component.h"

class InputSystem;
class Player;
class Ragdoll;
class RoundManager;

class CannonController : public Component
{
public:
	void Initialize(ServiceLocator& locator)override;
	void Update(float deltaTime)override;

	void AddForcePlayer(float deltaTime);
	void SetInitializePlayer(Player* player);
	void SetRoundManager(RoundManager* roundManager) { _roundManager = roundManager; }

private:
	void SetPlayerPosition();
	void RotationCannon(float deltaTime);
private:
	InputSystem* _input = nullptr;
	Player* _player = nullptr;
	Ragdoll* _ragdoll = nullptr;
	RoundManager* _roundManager = nullptr;

	float _swingFrame = 0.0f;//現在の回転値はラジアン
	float _swingSpeed = 1.0f;
	float _swingMinAngle = -80.0f;
	float _swingMaxAngle = 80.0f;
};