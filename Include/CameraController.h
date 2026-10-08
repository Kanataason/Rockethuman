#pragma once
#include <DirectXMath.h>

#include "Component.h"
#include "CollisionResult.h"

class RigidBody;
class InputSystem;

class CameraController :public Component
{
public :
	void Initialize(ServiceLocator& locator)override;
	void Update(float deltaTime)override;

	//falseにすると、WASDでの移動をしなくなる(タイトルのメニュー選択でW/Sを使うため)
	void SetMoveEnabled(bool enabled) { _moveEnabled = enabled; }
private:
	void MoveCamera(const DirectX::XMFLOAT3& direction, float speed);
	void RotationCamera(const POINT& rotation);
private:
	float _moveSpeed = 5.0f;
	float _sensitivity = 0.1f;
	float _minAngle = -89.0f;
	float _maxAngle = 89.0f;
	bool _moveEnabled = true;

	InputSystem* _input = nullptr;
	RigidBody* _rigidBody = nullptr;
};