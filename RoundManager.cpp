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
		_state = RoundState::AimAngle;
		OnReadyNextShot.Invoke();
	}
	else
	{
		_state = RoundState::Finished;
		OnFinished.Invoke();
	}
}

bool RoundManager::ConfirmAngle()
{
	if (_state != RoundState::AimAngle || _shotCount <= 0)
		return false;

	_state = RoundState::AimPower;
	return true;
}

bool RoundManager::TryUseShot()
{
	// 威力のゲージを動かしている間だけ撃てる(飛んでいる最中や、結果を見せている間は撃てない)
	if (_state != RoundState::AimPower || _shotCount <= 0)
		return false;

	_state = RoundState::Flying;
	OnShotFired.Invoke();
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
