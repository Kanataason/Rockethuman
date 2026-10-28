#include "Explosive.h"

#include "GameObject.h"
#include "ObjectStatus.h"

void Explosive::Initialize(ServiceLocator& locator)
{
	//的として当たった瞬間(体が当たったときも、爆風が当たったときも)に、爆発する
	if (auto* status = GetOwner()->GetComponent<ObjectStatus>())
		status->OnSendScore.Subscribe([this](HitInfo) { Explode(); });
}

void Explosive::Explode()
{
	if (_exploded)
		return;
	_exploded = true;

	const auto& position = GetOwner()->GetTransform().Position;

	Explosion explosion;
	explosion.position = Vector3(position.x, position.y, position.z);
	explosion.radius = _radius;
	explosion.power = _strength;

	OnExplode.Invoke(explosion);
}
