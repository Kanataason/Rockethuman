#include "ExplosionManager.h"

#include <algorithm>
#include <cmath>

#include "GameObject.h"
#include "ObjectStatus.h"
#include "Ragdoll.h"

namespace
{
	constexpr float blastWaveSpeed = 20.0f; //爆風が広がる速さ(m/s)。遅いほど、連鎖がゆっくり見える
	constexpr float ragdollReachScale = 1.5f; //人間に届く距離は、半径の何倍か
	constexpr float ragdollMinFalloff = 0.3f; //遠くても、最低これだけは、打ち上げる(0〜1)
	constexpr float ragdollForwardBias = 0.5f; //打ち上げる向きの、進行方向への傾き(1で、斜め45度)
	constexpr float ragdollPushScale = 1.0f; //打ち上げの強さの倍率(0で、人間は打ち上げない)
}

void ExplosionManager::AddTarget(ObjectStatus* target)
{
	if (target)
		_targets.push_back(target);
}

void ExplosionManager::Subscribe(Explosive* explosive)
{
	if (!explosive)
		return;

	//依頼を、積むだけ。実行は、Update でまとめて行う
	explosive->OnExplode.Subscribe([this](Explosion explosion) { _requests.push_back(explosion); });
}

void ExplosionManager::Update(float deltaTime)
{
	//爆発の依頼を、ここでまとめて実行する(当たりの通知の最中に、物の状態を変えないため)
	std::vector<Explosion> requests = std::move(_requests);
	_requests.clear();
	for (const Explosion& explosion : requests)
		Detonate(explosion);

	//時間差で、爆風を届ける。届いた的が爆発する箱なら、新しい依頼が積まれる(連鎖)
	std::vector<PendingHit> pending = std::move(_pendingHits);
	_pendingHits.clear();
	for (PendingHit& hit : pending)
	{
		hit.delay -= deltaTime;
		if (hit.delay > 0.0f)
		{
			_pendingHits.push_back(hit);
			continue;
		}

		if (hit.target)
			hit.target->ForceHit(hit.origin);
	}
}

void ExplosionManager::Detonate(const Explosion& explosion)
{
	OnExploded.Invoke(explosion);

	//半径の中の的に、距離に応じた時間差で、爆風を届ける
	for (ObjectStatus* target : _targets)
	{
		if (!target || target->IsHit())
			continue;

		const auto& position = target->GetOwner()->GetTransform().Position;
		const float dx = position.x - explosion.position.x;
		const float dy = position.y - explosion.position.y;
		const float dz = position.z - explosion.position.z;
		const float distance = std::sqrt(dx * dx + dy * dy + dz * dz);
		if (distance > explosion.radius)
			continue;

		PendingHit hit;
		hit.target = target;
		hit.origin = explosion.position;
		hit.delay = distance / blastWaveSpeed;
		_pendingHits.push_back(hit);
	}

	PushRagdoll(explosion);
}

void ExplosionManager::PushRagdoll(const Explosion& explosion)
{
	//発射してから、止まるまでの間だけ
	if (!_ragdoll || !_ragdoll->IsWaitingSettle())
		return;

	const Vector3 root = _ragdoll->GetRootPosition();
	const float dx = root.x - explosion.position.x;
	const float dy = root.y - explosion.position.y;
	const float dz = root.z - explosion.position.z;
	const float distance = std::sqrt(dx * dx + dy * dy + dz * dz);

	const float reach = explosion.radius * ragdollReachScale;
	if (distance > reach)
		return;

	//近いほど強く打ち上げる
	const float falloff = (std::max)(1.0f - distance / reach, ragdollMinFalloff);

	//爆心から離れる向きだと、飛んできた方へ押し戻されてしまうので、上へ、進行方向へ少し傾けて打ち上げる
	const Vector3 velocity = _ragdoll->GetRootVelocity();
	const float forwardX = (velocity.x >= 0.0f) ? 1.0f : -1.0f;
	const float x = forwardX * ragdollForwardBias;
	const float y = 1.0f;
	const float length = std::sqrt(x * x + y * y);

	const float speed = explosion.power * falloff * ragdollPushScale;
	_ragdoll->ApplyBoost(Vector3(x / length * speed, y / length * speed, 0.0f));
}
