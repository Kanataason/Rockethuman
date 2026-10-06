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
	// 砲身の、水平(+X)からの上向きの角度(度)。この範囲を行ったり来たりする
	float _swingMinAngle = 10.0f;
	float _swingMaxAngle = 80.0f;

	// 大砲の原点から、playerを置く場所までの、砲身に沿った距離(m)
	float _playerOffset = 3.3f;
};