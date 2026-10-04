#include "ObjectStatus.h"
#include "Collider.h"
#include "GameObject.h"
#include "Ragdoll.h"

void ObjectStatus::OnCollisionEnter(Collider* other, const CollisionResult& result)
{
	if (_isHit || other->GetCollisionGroup() != CollisionGroup::Ragdoll)
		return;

	_isHit = true;

	HitInfo hit;
	hit.baseScore = _score;
	hit.position = result.contactPoint;

	// 当たったコライダーが、どのボーンのものかを調べて、頭かどうかを決める
	if (auto* owner = other->GetOwner())
	{
		if (auto* ragdoll = owner->GetComponent<Ragdoll>())
			hit.isHead = ragdoll->GetBoneName(other).find("Head") != std::string::npos;
	}

	OnSendScore.Invoke(hit);
}
