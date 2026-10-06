#include <algorithm>

#include "CameraController.h"
#include "InputSystem.h"
#include "GameObject.h"
#include "RigidBody.h"
#include "Transform.h"

void CameraController::Initialize(ServiceLocator& locator)
{
    _input = locator.input;
    _rigidBody = GetOwner()->GetComponent<RigidBody>();
    _rigidBody->EnableKinematic();
}
void CameraController::Update(float deltaTime)
{
    if (!_moveEnabled)
        return;

    float speed = _moveSpeed * deltaTime;
    auto forward = GetOwner()->GetTransform().GetForward();
    auto right = GetOwner()->GetTransform().GetRight();

    if (_input->GetKey('W'))
        MoveCamera(
            forward,
            speed);

    if (_input->GetKey('S'))
        MoveCamera(
            { -forward.x,-forward.y,-forward.z },
            speed);

    if (_input->GetKey('A'))
        MoveCamera(
            { -right.x,-right.y,-right.z },
            speed);

    if (_input->GetKey('D'))
        MoveCamera(right,speed);

    if (_input->GetKeyDown('R'))
        _rigidBody->EnableKinematic();

  //RotationCamera(_input->GetMouseDelta());
}
void CameraController::RotationCamera(const POINT& delta)
{
    auto& transform = GetOwner()->GetTransform();

    transform.Rotation.y += delta.x * _sensitivity; //左右
    transform.Rotation.x -= delta.y * _sensitivity; //上下
    transform.Rotation.x = std::clamp(transform.Rotation.x, _minAngle, _maxAngle);
}
void CameraController::MoveCamera(const DirectX::XMFLOAT3& direction, float speed)
{
    if (_rigidBody->IsKinematic())
    {
        //キネマティック時はPhysicsSystem::SyncKinematicBodiesがTransform→RigidBodyへ
        //毎フレーム同期するので、RigidBodyを直接止めても次フレームで上書きされてしまう。
        //Transform自体を動かし、物理側の同期に反映させる。
        auto& transform = GetOwner()->GetTransform();
        transform.Position.x += direction.x * speed;
        transform.Position.y += direction.y * speed;
        transform.Position.z += direction.z * speed;
    }
    else
    {
        //非キネマティック時は速度を与えて物理演算に任せる
        Vector3 velocity = { direction.x * speed, direction.y * speed, direction.z * speed };
        _rigidBody->SetVelocity(velocity);
    }

}