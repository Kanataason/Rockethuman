#include "DebugController.h"
#include "InputSystem.h"

void DebugController::Initialize(ServiceLocator& locator)
{
	_input = locator.input;
}
void DebugController::Update(float deltaTime)
{

}