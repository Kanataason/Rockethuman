#include "EffectParticle.h"

#include <algorithm>

#include "GameObject.h"

void EffectParticle::Initialize(ServiceLocator& locator)
{
	Hide();
}

void EffectParticle::Hide()
{
	_active = false;
	GetOwner()->GetTransform().Scale = { 0.0f, 0.0f, 0.0f };
}

void EffectParticle::Emit(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& velocity,
	float lifeSeconds, float size, float gravityY, float spinDegrees)
{
	auto& transform = GetOwner()->GetTransform();
	transform.Position = position;
	transform.Rotation = { 0.0f, 0.0f, 0.0f };
	transform.Scale = { size, size, size };

	_velocity = velocity;
	_lifeSeconds = (std::max)(lifeSeconds, 0.01f);
	_size = size;
	_gravityY = gravityY;
	_spinDegrees = spinDegrees;
	_age = 0.0f;
	_active = true;
}

void EffectParticle::Update(float deltaTime)
{
	if (!_active)
		return;

	_age += deltaTime;
	if (_age >= _lifeSeconds)
	{
		Hide();
		return;
	}

	auto& transform = GetOwner()->GetTransform();

	//重力で曲がりながら、飛んでいく
	_velocity.y += _gravityY * deltaTime;
	transform.Position.x += _velocity.x * deltaTime;
	transform.Position.y += _velocity.y * deltaTime;
	transform.Position.z += _velocity.z * deltaTime;

	//回りながら、だんだん小さくなって消える(最後に向けて、小さくなる速さが上がる)
	transform.Rotation.z += _spinDegrees * deltaTime;
	transform.Rotation.x += _spinDegrees * 0.6f * deltaTime;

	const float t = _age / _lifeSeconds;
	const float scale = _size * (std::max)(1.0f - t * t, 0.0f);
	transform.Scale = { scale, scale, scale };
}
