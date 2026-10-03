#include <algorithm>
#include <cmath>
#include <DirectXMath.h>
#include "VectorUtility.h"

#include "PlayerController.h"
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

    // 砲身が回ったあとの向きに合わせて、Playerの位置も毎フレーム合わせ直す
    SetPlayerPosition();
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

void CannonController::SetPlayerPosition()
{
    if (!_player)
        return;

    auto& transform = GetOwner()->GetTransform();

    const Vector3 upDirection = Vector::fromF3ToV3(transform.GetUp());
    const Vector3 center = Vector::fromF3ToV3(transform.Position)
        + upDirection * (transform.Scale.y * 0.5f);

    auto& playerTransform = _player->GetOwner()->GetTransform();
    playerTransform.Position = Vector::fromV3ToF3(center);
    playerTransform.Rotation.z = transform.Rotation.z;
}