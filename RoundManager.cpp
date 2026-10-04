#include "RoundManager.h"

void RoundManager::Initialize(ServiceLocator& locator)
{

}

void RoundManager::Update(float deltaTime)
{
	if (_state != RoundState::Result)
		return;

	_resultTimer += deltaTime;
	if (_resultTimer < kResultSeconds)
		return;

	if (_shotCount > 0)
	{
		_state = RoundState::Aiming;
		OnReadyNextShot.Invoke();
	}
	else
	{
		_state = RoundState::Finished;
		OnFinished.Invoke();
	}
}

bool RoundManager::TryUseShot()
{
	// 飛んでいる最中や、結果を見せている間は撃てない
	if (_state != RoundState::Aiming || _shotCount <= 0)
		return false;

	_state = RoundState::Flying;
	return true;
}

void RoundManager::NotifySettled()
{
	if (_state != RoundState::Flying)
		return;

	_state = RoundState::Result;

	--_shotCount;
	_resultTimer = 0.0f;
	OnShotSettled.Invoke();
}
