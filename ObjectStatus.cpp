#include "ObjectStatus.h"
#include "Collider.h"

void ObjectStatus::OnCollisionEnter(Collider* other, const CollisionResult& result)
{
	if (other->GetCollisionGroup() == CollisionGroup::Ragdoll)
		OnSendScore.Invoke(_score);
}