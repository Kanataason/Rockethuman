#include "PlayerController.h"
#include "GameObject.h"
#include "Ragdoll.h"
#include "Animator.h"
#include "RigidBody.h"
#include "RagdollData.h"
#include "InputSystem.h"

void PlayerController::Initialize(ServiceLocator& loacator)
{
	_input = loacator.input;

	_animator = GetOwner()->GetComponent<Animator>();
	_rigidBody = GetOwner()->GetComponent<RigidBody>();
	_ragdoll = GetOwner()->GetComponent<Ragdoll>();
}

// 状態は3つ:
// アニメーション再生
// 重力で地面に倒れている
// 体を吊り下げて、浮かせたまま矢印キーで移動
void PlayerController::Update(float deltaTime)
{
	auto speed = 4 * deltaTime;

	switch (_state)
	{
	case PlayerState::Animation:
		UpdateAnimationState(speed);
		break;
	case PlayerState::Fall:
		UpdateFallState();
		break;
	case PlayerState::Hang:
		UpdateHangState(deltaTime, speed);
		break;
	}
}

// 通常:アニメーションを再生したまま、矢印キーで Transform を動かして移動する
void PlayerController::UpdateAnimationState(float speed)
{
	Vector3 moveDirection(0, 0, 0);

	if (_input->GetKey(VK_UP))      moveDirection.z += 1.0f;
	if (_input->GetKey(VK_DOWN))    moveDirection.z -= 1.0f;
	if (_input->GetKey(VK_RIGHT))   moveDirection.x += 1.0f;
	if (_input->GetKey(VK_LEFT))    moveDirection.x -= 1.0f;
	if (_input->GetKey(VK_SHIFT))   moveDirection.y += 1.0f;
	if (_input->GetKey(VK_CONTROL)) moveDirection.y -= 1.0f;

	if (moveDirection.Length() > 0.0f)
	{
		moveDirection.Normalize();
		MovePlayer(moveDirection, speed);
	}

	//重力で地面に倒れる
	if (_input->GetKeyDown('P'))
	{
		_ragdoll->StartFall();
		_state = PlayerState::Fall;
	}
}


void PlayerController::UpdateFallState()
{
	// 倒れている体を吊り下げる(その姿勢のまま持ち上げる)
	if (_input->GetKeyDown(RagdollTuning::kHangingToggleKey))
	{
		_ragdoll->StartHanging();
		_state = PlayerState::Hang;
		return;
	}

	// 通常(アニメーション再生)に戻る。倒れていた場所に、水平だけ移す(高さは変えない)
	if (_input->GetKeyDown('Q'))
	{
		_ragdoll->StopRagdoll(false);
		_state = PlayerState::Animation;
	}
}


void PlayerController::UpdateHangState(float deltaTime, float speed)
{
	//吊り下げをやめて、また重力で落とす
	if (_input->GetKeyDown(RagdollTuning::kHangingToggleKey) || _input->GetKeyDown('P'))
	{
		_ragdoll->ReleaseHanging();
		_state = PlayerState::Fall;
		return;
	}

	// 通常(アニメーション再生)に戻る。吊り下げていた位置に、高さも含めて移す
	if (_input->GetKeyDown('Q'))
	{
		_ragdoll->StopRagdoll(true);
		_state = PlayerState::Animation;
		return;
	}

	Vector3 moveDirection(0, 0, 0);

	if (_input->GetKey(VK_UP))      moveDirection.z += 1.0f;
	if (_input->GetKey(VK_DOWN))    moveDirection.z -= 1.0f;
	if (_input->GetKey(VK_RIGHT))   moveDirection.x += 1.0f;
	if (_input->GetKey(VK_LEFT))    moveDirection.x -= 1.0f;
	if (_input->GetKey(VK_SHIFT))   moveDirection.y += 1.0f;
	if (_input->GetKey(VK_CONTROL)) moveDirection.y -= 1.0f;

	if (moveDirection.Length() > 0.0f)
	{
		moveDirection.Normalize();
		_ragdoll->MoveHangingBone(moveDirection*speed, deltaTime);
	}

	// 飛ばされた勢いを反映して、吊る骨の速度を物理に伝える
	_ragdoll->UpdateHanging(deltaTime);
}

void PlayerController::MovePlayer(const Vector3& direction,float speed)
{
	if (_rigidBody->IsKinematic())
	{
		// キネマティック時はPhysicsSystem::SyncKinematicBodiesがTransform→RigidBodyへ
		// 毎フレーム同期するので、RigidBodyを直接止めても次フレームで上書きされてしまう。
		// Transform自体を動かし、物理側の同期に反映させる。
		auto& transform = GetOwner()->GetTransform();
		transform.Position.x += direction.x * speed;
		transform.Position.y += direction.y * speed;
		transform.Position.z += direction.z * speed;
	}
}

//当たった時の通知
void PlayerController::OnCollisionEnter(std::shared_ptr<Collider>& other, const CollisionResult& result)
{
}
void PlayerController::OnCollisionStay(std::shared_ptr<Collider>& other, const CollisionResult& result)
{

}
void PlayerController::OnCollisionExit(std::shared_ptr<Collider>& other, const CollisionResult& result)
{
}
