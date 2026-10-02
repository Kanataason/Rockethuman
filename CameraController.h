#pragma once
#include <DirectXMath.h>

#include "Component.h"
#include "CollisionResult.h"

class RigidBody;
class InputSystem;

class CameraController :public Component
{
public :
	void Initialize(ServiceLocator& context)override;
	void Update(float deltaTime)override;
private:
	void MoveCamera(const DirectX::XMFLOAT3& direction, float speed);
	void RotationCamera(const POINT& rotation);
private:
	float _moveSpeed = 5.0f;
	float _sensitivity = 0.1f;
	float _minAngle = -89.0f;
	float _maxAngle = 89.0f;

	InputSystem* _input = nullptr;
	RigidBody* _rigidBody = nullptr;
};