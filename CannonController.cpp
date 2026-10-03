#include <algorithm>
#include <cmath>
#include <DirectXMath.h>

#include "CannonController.h"
#include "GameObject.h"
#include "InputSystem.h"

void CannonController::Initialize(ServiceLocator& locator)
{
	_input = locator.input;
}

void CannonController::Update(float deltaTime)
{
    RotationCannon(deltaTime);
}
void CannonController::RotationCannon(float deltaTime)
{
    auto& transform = GetOwner()->GetTransform();

    _swingFrame += _swingSpeed * deltaTime;

    constexpr float maxAngle = DirectX::XM_2PI;
    _swingFrame = std::fmod(_swingFrame, maxAngle);

    float centerDeg = (_swingMinAngle + _swingMaxAngle) * 0.5f;
    float amplitudeDeg = (_swingMaxAngle - _swingMinAngle) * 0.5f;

    float resultDeg = centerDeg + amplitudeDeg * std::sin(_swingFrame);

    transform.Rotation.z = resultDeg;
}