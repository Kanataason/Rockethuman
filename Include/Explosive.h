#pragma once
#include "Component.h"
#include "Event.h"
#include "VectorUtility.h"

//爆発の情報(どこで、どのくらいの範囲に、どのくらいの強さで)
struct Explosion
{
	Vector3 position{}; //爆心
	float radius = 4.0f; //爆風が届く半径(m)
	float power = 14.0f; //人間を打ち上げる速さ(m/s)
};

//爆発する箱。的として当たった瞬間に、爆発を知らせる
//ここが決めるのは「爆発した」という合図と、半径・強さだけ。
//爆風を的に届けるのは ExplosionManager、火の玉は EffectSystem、画面の揺れや音は、通知を受けた側が行う
class Explosive : public Component
{
public:
	//爆発した瞬間
	Event<Explosion> OnExplode;

	void Initialize(ServiceLocator& locator)override;

	//半径と強さ(Jsonの "explosionRadius" "explosionPower")
	void Configure(float radius, float power)
	{
		_radius = radius;
		_strength = power;
	}

private:
	void Explode();

private:
	float _radius = 4.0f;
	float _strength = 20.0f;
	bool _exploded = false; //爆発は、1回だけ
};
