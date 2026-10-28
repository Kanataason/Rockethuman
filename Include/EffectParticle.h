#pragma once
#include <DirectXMath.h>

#include "Component.h"

//エフェクトの粒(光る小さな箱)。物理は持たず、決まった動きで飛んで、縮んで消える
//使っていないときは、大きさを0にして、見えなくしておく(EffectSystemが、使い回す)
class EffectParticle : public Component
{
public:
	void Initialize(ServiceLocator& locator)override;
	void Update(float deltaTime)override;

	//粒を出す
	//position:出す場所 / velocity:最初の速さ(m/s) / lifeSeconds:消えるまでの秒数 / size:最初の大きさ(m)
	//gravityY:重力(m/s^2)。下に落とすなら負、煙のように浮かせるなら正 / spinDegrees:回る速さ(度/秒)
	void Emit(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& velocity,
		float lifeSeconds, float size, float gravityY, float spinDegrees);

	bool IsActive() const { return _active; }
private:
	void Hide();
private:
	bool _active = false;
	DirectX::XMFLOAT3 _velocity{};
	float _lifeSeconds = 1.0f;
	float _age = 0.0f;
	float _size = 0.2f;
	float _gravityY = 0.0f;
	float _spinDegrees = 0.0f;
};
