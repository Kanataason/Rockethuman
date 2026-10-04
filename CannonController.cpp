#include <algorithm>
#include <cmath>
#include <DirectXMath.h>
#include "VectorUtility.h"

#include "PhysicsSystem.h"
#include "Ragdoll.h"
#include "Player.h"
#include "CannonController.h"
#include "GameObject.h"
#include "RoundManager.h"
#include "InputSystem.h"

void CannonController::Initialize(ServiceLocator& locator)
{
	_input = locator.input;
}
void CannonController::AddForcePlayer(float deltaTime)
{
    auto& transform = GetOwner()->GetTransform();
    const Vector3 direction = Vector::fromF3ToV3(transform.GetUp());   // すでに長さ1
    _ragdoll->Launch(direction, 200.0f);
}

void CannonController::Update(float deltaTime)
{
    RotationCannon(deltaTime);
    if (!_ragdoll)
        return;

    if (_input->GetKeyDown('K')&&_roundManager&&_roundManager->TryUseShot())
    {
        _ragdoll->StartFall();
        AddForcePlayer(deltaTime);
    }

    // 砲身が回ったあとの向きに合わせて、Playerの位置も毎フレーム合わせ直す
    if(!_ragdoll->IsActive())
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

void CannonController::SetInitializePlayer(Player* player)
{
    if (!player)
        return;
    _player = player;
    _ragdoll = _player->GetOwner()->GetComponent<Ragdoll>();
}

void CannonController::SetPlayerPosition()
{
    if (!_player)
        return;

    //Playerの場所の割合
    constexpr float helfSize = 0.28f;

    auto& transform = GetOwner()->GetTransform();

    //砲台の向き、砲台の原点と方向を足して長さと割合をかけてplayerの場所を決める
    const Vector3 upDirection = Vector::fromF3ToV3(transform.GetUp());
    const Vector3 center = Vector::fromF3ToV3(transform.Position)
        + upDirection * (transform.Scale.y * helfSize);

    auto& playerTransform = _player->GetOwner()->GetTransform();
    playerTransform.Position = Vector::fromV3ToF3(center);
    playerTransform.Rotation.z = transform.Rotation.z;
}