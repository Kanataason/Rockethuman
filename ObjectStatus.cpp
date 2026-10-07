#include <algorithm>
#include <cmath>
#include <random>

#include "ObjectStatus.h"
#include "Collider.h"
#include "GameObject.h"
#include "PhysicsSystem.h"
#include "Ragdoll.h"

void ObjectStatus::Initialize(ServiceLocator& locator)
{
	_physicsSystem = locator.physicsSystem;
}

void ObjectStatus::OnCollisionEnter(Collider* other, const CollisionResult& result)
{
	if (_isHit || other->GetCollisionGroup() != CollisionGroup::Ragdoll)
		return;

	_isHit = true;

	HitInfo hit;
	hit.baseScore = _score;
	hit.position = result.contactPoint;

	//念のための保険: 接触点が空(原点)の結果が来たときは、当たったボーンのコライダーの中心を使う
	if (result.colliderA == nullptr || result.colliderB == nullptr)
	{
		const XMFLOAT3 center = other->GetCenter();
		hit.position = Vector3(center.x, center.y, center.z);
	}

	//体が飛んでいく向き(右か左か)。
	//当たったコライダーが、どのボーンのものかも調べて、頭かどうかを決める
	float directionX = 1.0f;
	if (auto* owner = other->GetOwner())
	{
		if (auto* ragdoll = owner->GetComponent<Ragdoll>())
		{
			hit.isHead = ragdoll->GetBoneName(other).find("Head") != std::string::npos;

			const float rootVelocityX = ragdoll->GetRootVelocity().x;
			if (std::abs(rootVelocityX) > 0.1f)
				directionX = rootVelocityX > 0.0f ? 1.0f : -1.0f;
		}
	}

	InitializeFly(directionX);

	OnSendScore.Invoke(hit);
}

void ObjectStatus::InitializeFly(float directionX)
{
	//飛ぶ速さと回転を決める(毎回少しずつ違う)。2Dの見た目なので、奥(Z)には飛ばさない
	static std::mt19937 rng{ std::random_device{}() };
	std::uniform_real_distribution<float> speedScale(0.7f, 1.3f);
	std::uniform_int_distribution<int> spinSign(0, 1);

	_flyVelocity = { directionX * _flySpeed * speedScale(rng), _flyUpSpeed * speedScale(rng), 0.0f };
	_spinDegrees = (spinSign(rng) == 0 ? 1.0f : -1.0f) * _flySpinDegrees;

	//物理の中(衝突の通知の最中)で、物理の登録を変えると危ないので、次のUpdateで外す
	_flyState = FlyState::Requested;
}

void ObjectStatus::Update(float deltaTime)
{
	UpdateFly(deltaTime);
}

void ObjectStatus::UpdateFly(float deltaTime)
{
	if (_flyState == FlyState::None || _flyState == FlyState::Done)
		return;

	auto& transform = GetOwner()->GetTransform();

	if (_flyState == FlyState::Requested)
	{
		//物理から外す。これ以降は、ここで位置と向きを動かす(体は通り抜けて、積んであった箱は落ちる)
		if (_physicsSystem)
		{
			if (auto* collider = GetOwner()->GetComponent<Collider>())
				_physicsSystem->RemoveColliderAndWakeUnsupported(collider);
		}

		_baseScale = transform.Scale;
		_flyAge = 0.0f;
		_flyState = FlyState::Flying;
	}

	_flyAge += deltaTime;

	//放物線: 重力で下に引かれながら、飛んでいく
	_flyVelocity.y += GRAVITY * deltaTime;
	transform.Position.x += _flyVelocity.x * deltaTime;
	transform.Position.y += _flyVelocity.y * deltaTime;
	transform.Position.z += _flyVelocity.z * deltaTime;

	//回りながら、だんだん小さくなって消える(最後に向けて、小さくなる速さが上がる)
	transform.Rotation.z += _spinDegrees * deltaTime;

	const float t = (std::min)(_flyAge / _flySeconds, 1.0f);
	const float scaleRatio = (std::max)(1.0f - t * t, 0.0001f);
	transform.Scale = { _baseScale.x * scaleRatio, _baseScale.y * scaleRatio, _baseScale.z * scaleRatio };

	if (t >= 1.0f)
		_flyState = FlyState::Done;
}
