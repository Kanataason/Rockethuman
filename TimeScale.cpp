#include <algorithm>

#include "TimeScale.h"

void TimeScale::Request(float scale, float seconds)
{
	if (seconds <= 0.0f)
		return;

	//0にすると、時間が完全に止まるので、下限を決める
	const float minScale = 0.02f;
	scale = (std::clamp)(scale, minScale, 1.0f);

	_startScale = (std::min)(_current, scale);
	_current = _startScale;
	_elapsed = 0.0f;
	_duration = seconds;
}

void TimeScale::Update(float realDeltaTime)
{
	if (_duration <= 0.0f)
		return;

	_elapsed += realDeltaTime;

	const float t = (std::min)(_elapsed / _duration, 1.0f);

	//t*t で戻す: 最初は遅いまま、後半で一気に元の速さへ戻る
	_current = _startScale + (1.0f - _startScale) * t * t;

	if (t >= 1.0f)
	{
		_current = 1.0f;
		_duration = 0.0f;
	}
}
