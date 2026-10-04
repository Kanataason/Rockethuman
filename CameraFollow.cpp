#include <algorithm>
#include <cmath>

#include "CameraFollow.h"
#include "GameObject.h"
#include "Ragdoll.h"

void CameraFollow::Initialize(ServiceLocator& context)
{
	_startPosition = GetOwner()->GetTransform().Position;
}

void CameraFollow::StartFollow()
{
	if (!_target)
		return;

	_startRootY = _target->GetRootPosition().y;
	_mode = Mode::Follow;
}

void CameraFollow::ReturnToStart()
{
	_mode = Mode::Return;
}

void CameraFollow::Update(float deltaTime)
{
	if (_mode == Mode::Idle)
		return;

	auto& position = GetOwner()->GetTransform().Position;

	DirectX::XMFLOAT3 goal = _startPosition;
	float sharpness = _returnSharpness;

	if (_mode == Mode::Follow)
	{
		if (!_target)
			return;

		const auto root = _target->GetRootPosition();

		// 奥行きは速さによって拡大縮小。横は腰の少し先、縦は動いた分の一部だけ追う
		goal.x = root.x + _lockX;
		goal.y = _startPosition.y + (root.y - _startRootY) *_followYRatio;

		// 今のラグドールの速さに応じて、なめらかに引く
		const float speed = _target->GetRootVelocity().Length();
		const float pullBackRatio = std::clamp(speed / _pullBackFullSpeed, 0.0f, 1.0f);
		goal.z = _startPosition.z * (1.0f + (_pullBackScale - 1.0f) * pullBackRatio);

		sharpness = _followSharpness;
	}

	// フレームレートが変わっても、同じ速さで近づく
	const float t = 1.0f - std::exp(-sharpness * deltaTime);
	position.x += (goal.x - position.x) * t;
	position.y += (goal.y - position.y) * t;
	position.z += (goal.z - position.z) * t;

	if (_mode == Mode::Return)
	{
		const float dx = goal.x - position.x;
		const float dy = goal.y - position.y;
		if (dx * dx + dy * dy < _returnFinishDistance * _returnFinishDistance)
		{
			position = _startPosition;
			_mode = Mode::Idle;
		}
	}
}
