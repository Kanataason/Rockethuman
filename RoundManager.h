#pragma once
#include "ServiceLoacator.h"

class RoundManager
{
public:
	void Initialize(ServiceLocator& locator);

	bool TryUseShot();
	int GetShotCount()const { return _shotCount; }
private:
	int _shotCount = 3;
};