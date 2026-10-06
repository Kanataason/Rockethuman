#include <algorithm>
#include <cmath>

#include "CameraFollow.h"
#include "GameObject.h"
#include "Ragdoll.h"

void CameraFollow::Initialize(ServiceLocator& locator)
{
	_startPosition = GetOwner()->GetTransform().Position;
}

void CameraFollow::StartFollow()
{
	if (!_target)
		return;

	_startRootY = _target->GetRootPosition().y;
	_hasBeenFast = false;
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

		//今のラグドールの速さ
		const float speed = _target->GetRootVelocity().Length();
		const float speedRatio = std::clamp(speed / _pullBackFullSpeed, 0.0f, 1.0f);

		//速く飛んだあとで、遅くなってきたら寄る。最初の、まだ速くなっていない間は、寄らない
		_hasBeenFast = _hasBeenFast || speed > _zoomInArmSpeed;
		const float minScale = _hasBeenFast ? _zoomInScale : 1.0f;

		//奥行きは速さによって拡大縮小(速い: 引く ～ 遅い: 寄る)。横は腰の少し先
		//縦は、速いときは動いた分の一部だけ。寄るほど、プレイヤーが画面の中心に来るように、全部追う
		const float yRatio = _followYRatio + (1.0f - _followYRatio) * (1.0f - speedRatio);
		goal.x = root.x + _lockX;
		goal.y = _startPosition.y + (root.y - _startRootY) * yRatio;

		const float scale = minScale + (_pullBackScale - minScale) * speedRatio;
		goal.z = _startPosition.z * scale;

		//寄りすぎて、プレイヤーにめり込まないように、距離に下限をつける
		goal.z = (std::min)(goal.z, root.z - _minDistanceToTarget);

		sharpness = _followSharpness;
	}

	//フレームレートが変わっても、同じ速さで近づく
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
