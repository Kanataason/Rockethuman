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
#include "PowerZone.h"

void CannonController::Initialize(ServiceLocator& locator)
{
	_input = locator.input;
}
void CannonController::AddForcePlayer(float deltaTime)
{
    auto& transform = GetOwner()->GetTransform();
    // 砲身は、大砲の+X方向(横)に伸びている。Rotation.zが、水平からの上向きの角度になる
    const Vector3 direction = Vector::fromF3ToV3(transform.GetRight());   // すでに長さ1

    // 威力ゲージの量(0～1)から、速さを決める。緑は弱く、黄色は中くらい、赤は強い(PowerZone.hの表)
    const float power = _roundManager ? _roundManager->GetPowerAmount() : 1.0f;
    const float launchSpeed = PowerZone::SpeedFromPower(power);
    _ragdoll->Launch(direction, launchSpeed);
}

void CannonController::Update(float deltaTime)
{
    if (!_ragdoll || !_roundManager)
        return;

    // スペースキーで、2段階に止める: 角度(揺れている大砲)-> 威力(ゲージ)で発射
    const bool pressed = _input->GetKeyDown(VK_SPACE);

    switch (_roundManager->GetState())
    {
    case RoundState::AimAngle:
        RotationCannon(deltaTime);
        if (pressed)
            _roundManager->ConfirmAngle();
        break;

    case RoundState::AimPower:
        // 角度は止まったまま。威力のゲージを止めたら、発射する
        if (pressed && _roundManager->TryUseShot())
        {
            _ragdoll->StartFall();
            AddForcePlayer(deltaTime);
        }
        break;

    default:
        break;
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

    auto& transform = GetOwner()->GetTransform();

    //砲台の原点から、砲身の向きに _playerOffset だけ進んだ所に、playerを置く
    //(以前は Scale.y を使っていたが、FBXは Scale が 0.01 などなので、メートルで直接指定する)
    const Vector3 barrelDirection = Vector::fromF3ToV3(transform.GetRight());
    const Vector3 barrelPoint = Vector::fromF3ToV3(transform.Position) + barrelDirection * _playerOffset;
    const float hipsHeight = _ragdoll->GetHipHeight();
    const Vector3 origin = barrelPoint - barrelDirection * hipsHeight;

    auto& playerTransform = _player->GetOwner()->GetTransform();
    playerTransform.Position = Vector::fromV3ToF3(origin);

    //playerは立った姿(頭が+Y)なので、砲身(+X)に頭を向けるために、-90度ぶん寝かせる
    playerTransform.Rotation.z = transform.Rotation.z - 90.0f;
}