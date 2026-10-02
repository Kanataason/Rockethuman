#pragma once
#include <DirectXMath.h>
#include <iostream>
#include "Component.h"

class InputSystem;
class RigidBody;
class Animator;
class Ragdoll;

// プレイヤーの状態
enum class PlayerState
{
	Animation,	// 通常:アニメーションを再生して、矢印キーで移動する
	Fall,		// 倒れる:重力で地面に倒れている(ラグドール)
	Hang,		// ぶらぶら:体を吊り下げて、浮かせたまま、矢印キーで移動する
};

class PlayerController : public Component
{
public:
	void Initialize(ServiceLocator& loacator)override;

	void Update(float deltaTime)override;
	
private:
	// 状態ごとの更新
	void UpdateAnimationState(float speed);

	//重力で倒れている
	void UpdateFallState();

	// プレイヤーの Transform は動かさない。ラグドール中はボーンが世界座標で動くので、
    // Transform を動かすと、メッシュと物理の位置がずれて点滅する原因になる。
	void UpdateHangState(float deltaTime, float speed);

	// 矢印キーの入力で、Transform を動かして移動する(通常のとき)
	void MovePlayer(const Vector3& direction,float speed);

	void OnCollisionEnter(std::shared_ptr<Collider>& other, const CollisionResult& result)override;
	void OnCollisionStay(std::shared_ptr<Collider>& other, const CollisionResult& result)override;
	void OnCollisionExit(std::shared_ptr<Collider>& other, const CollisionResult& result)override;

private:
	InputSystem* _input = nullptr;
	std::shared_ptr<Ragdoll>_ragdoll = nullptr;
	std::shared_ptr<RigidBody> _rigidBody = nullptr;
	std::shared_ptr<Animator> _animator = nullptr;

	PlayerState _state = PlayerState::Animation; // 今の状態(最初は通常)
};
