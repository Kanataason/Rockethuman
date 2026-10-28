#pragma once
#include <random>
#include <vector>
#include <DirectXMath.h>

#include "ServiceLocator.h"

class BaseScene;
class EffectParticle;

//画面を派手にする、小さなエフェクト(火花・土煙)
//光る小さな箱(EffectParticle)を、あらかじめ並べておいて、使い回す
//(実行中に作ったり消したりしないので、負荷が増えにくい)
class EffectSystem
{
public:
	//粒を、シーンに並べる(シーンのInitializeのあとに呼ぶ)
	void Initialize(ServiceLocator& locator, BaseScene& scene);

	//当たった場所に、火花を散らす。頭ヒットとコンボが多いほど、数が増える
	void Burst(const DirectX::XMFLOAT3& position, bool isHead, int combo);

	//着地したときの、土煙
	void Dust(const DirectX::XMFLOAT3& position);

	//爆発。大きな火の玉と煙を出す。radius(m)が大きいほど、大きく、勢いよく広がる
	void Explosion(const DirectX::XMFLOAT3& position, float radius);

	//ロケットの炎。backward(噴射と反対の向き)へ、粒を出す。噴射している間、毎フレーム呼ぶ
	void Flame(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& backward);

private:
	void CreatePool(ServiceLocator& locator, BaseScene& scene,
		const std::vector<const char*>& materialNames, int count, std::vector<EffectParticle*>& pool);
	EffectParticle* Acquire(std::vector<EffectParticle*>& pool, size_t& cursor);
	float Random(float minValue, float maxValue);

private:
	std::vector<EffectParticle*> _sparks;
	std::vector<EffectParticle*> _smokes;
	std::vector<EffectParticle*> _flames;
	size_t _sparkCursor = 0;
	size_t _smokeCursor = 0;
	size_t _flameCursor = 0;

	std::mt19937 _random{ std::random_device{}() };
};
