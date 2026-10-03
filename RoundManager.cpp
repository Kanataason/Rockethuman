#include "RoundManager.h"

void RoundManager::Initialize(ServiceLocator& locator)
{

}

bool RoundManager::TryUseShot()
{
	if (_shotCount <= 0) return false;

	 --_shotCount;
	return true;
}