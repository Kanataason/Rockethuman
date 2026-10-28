#include "EffectSystem.h"

#include <algorithm>
#include <cmath>

#include "BaseScene.h"
#include "BaseGameObjectFactory.h"
#include "GameObject.h"
#include "EffectParticle.h"

namespace
{
	//---- 粒の数(あらかじめ並べる数。多いほど派手だが、重くなる) ----
	constexpr int sparkPoolSize = 80;
	constexpr int smokePoolSize = 54;

	//---- 火花 ----
	constexpr int sparkBaseCount = 14; //ふつうのヒットで出す数
	constexpr int sparkHeadCount = 28; //頭ヒットで出す数
	constexpr int sparkComboBonus = 2; //コンボ1つにつき、足す数
	constexpr int sparkComboMax = 5; //足すコンボの上限
	constexpr float sparkSpeedMin = 5.0f; //飛び出す速さ(m/s)
	constexpr float sparkSpeedMax = 13.0f;
	constexpr float sparkLifeMin = 0.45f; //消えるまでの秒数
	constexpr float sparkLifeMax = 0.9f;
	constexpr float sparkSizeMin = 0.12f; //大きさ(m)
	constexpr float sparkSizeMax = 0.3f;
	constexpr float sparkHeadSizeScale = 1.4f; //頭ヒットは、粒を大きくする
	constexpr float sparkGravity = -12.0f;
	constexpr float sparkDepthSpread = 2.0f; //奥行き(Z)への、散らばり(2.5Dなので、小さめ)

	//---- 土煙 ----
	constexpr int smokeCount = 15;
	constexpr float smokeSideSpeed = 3.0f;
	constexpr float smokeUpSpeedMin = 0.5f;
	constexpr float smokeUpSpeedMax = 3.0f;
	constexpr float smokeLifeMin = 0.7f;
	constexpr float smokeLifeMax = 1.2f;
	constexpr float smokeSizeMin = 0.35f;
	constexpr float smokeSizeMax = 0.7f;
	constexpr float smokeGravity = 0.5f; //少し浮かぶ

	//---- ロケットの炎(1回の呼び出しで出す数 x 寿命 x フレームレートが、並べる数を超えないように) ----
	constexpr int flamePoolSize = 60;
	constexpr int flamePerCall = 3;
	constexpr float flameSpeedMin = 4.0f; //後ろへ流れる速さ(m/s)
	constexpr float flameSpeedMax = 9.0f;
	constexpr float flameSpread = 1.5f; //横へのばらつき(m/s)
	constexpr float flameLifeMin = 0.2f;
	constexpr float flameLifeMax = 0.4f;
	constexpr float flameSizeMin = 0.22f;
	constexpr float flameSizeMax = 0.38f;

	//---- 爆発(炎の粒と、煙の粒を使う) ----
	constexpr float explosionBaseRadius = 4.0f; //この半径のとき、下の大きさと速さになる(m)
	constexpr int explosionFireCount = 36;
	constexpr float explosionFireSpeedMin = 6.0f;
	constexpr float explosionFireSpeedMax = 16.0f;
	constexpr float explosionFireSizeMin = 0.3f;
	constexpr float explosionFireSizeMax = 0.7f;
	constexpr float explosionFireLifeMin = 0.5f;
	constexpr float explosionFireLifeMax = 1.0f;
	constexpr float explosionFireGravity = -6.0f;
	constexpr int explosionSmokeCount = 10;
	constexpr float explosionSmokeSpeed = 4.0f;
	constexpr float explosionSmokeSizeMin = 0.8f;
	constexpr float explosionSmokeSizeMax = 1.6f;
	constexpr float explosionSmokeLifeMin = 1.0f;
	constexpr float explosionSmokeLifeMax = 1.8f;

	constexpr float twoPi = 6.2831853f;
}

void EffectSystem::Initialize(ServiceLocator& locator, BaseScene& scene)
{
	//光る色の材質(Asset/Material の fx*.mat)。並べる順に使うので、色が混ざる
	CreatePool(locator, scene, { "fxYellow", "fxFire", "fxWhite", "fxFire" }, sparkPoolSize, _sparks);
	CreatePool(locator, scene, { "fxSmoke" }, smokePoolSize, _smokes);
	CreatePool(locator, scene, { "fxYellow", "fxFire", "fxFire", "fxWhite" }, flamePoolSize, _flames);
}

void EffectSystem::CreatePool(ServiceLocator& locator, BaseScene& scene,
	const std::vector<const char*>& materialNames, int count, std::vector<EffectParticle*>& pool)
{
	if (!locator.factory || materialNames.empty())
		return;

	for (int i = 0; i < count; ++i)
	{
		ObjectData data;
		data.attribute = Attribute::Primitive;
		data.typeName = ObjectType::Effect;
		data.shape = PrimitiveShape::Box;
		data.materialName = materialNames[i % materialNames.size()];
		data.transform.Scale = { 0.0f, 0.0f, 0.0f }; //最初は見えない

		auto object = locator.factory->CreateObject(data);
		if (!object)
			continue;

		//ステージを作ったあとに足すので、ここで初期化する(メッシュの準備など)
		object->Initialize(locator);

		if (auto* particle = object->GetComponent<EffectParticle>())
			pool.push_back(particle);

		scene.AddObject(std::move(object));
	}
}

EffectParticle* EffectSystem::Acquire(std::vector<EffectParticle*>& pool, size_t& cursor)
{
	if (pool.empty())
		return nullptr;

	//順番に使う。全部使っていたら、いちばん古い粒を、使い回す
	cursor = (cursor + 1) % pool.size();
	return pool[cursor];
}

float EffectSystem::Random(float minValue, float maxValue)
{
	std::uniform_real_distribution<float> distribution(minValue, maxValue);
	return distribution(_random);
}

void EffectSystem::Burst(const DirectX::XMFLOAT3& position, bool isHead, int combo)
{
	const int comboBonus = (std::min)((std::max)(combo, 0), sparkComboMax) * sparkComboBonus;
	const int count = (isHead ? sparkHeadCount : sparkBaseCount) + comboBonus;
	const float sizeScale = isHead ? sparkHeadSizeScale : 1.0f;

	for (int i = 0; i < count; ++i)
	{
		EffectParticle* particle = Acquire(_sparks, _sparkCursor);
		if (!particle)
			return;

		//横から見た画面(XY)の、あらゆる向きへ、飛び散らせる
		const float angle = Random(0.0f, twoPi);
		const float speed = Random(sparkSpeedMin, sparkSpeedMax);
		const DirectX::XMFLOAT3 velocity(
			std::cos(angle) * speed,
			std::sin(angle) * speed + 2.0f, //少し、上に偏らせる
			Random(-sparkDepthSpread, sparkDepthSpread));

		particle->Emit(position, velocity,
			Random(sparkLifeMin, sparkLifeMax),
			Random(sparkSizeMin, sparkSizeMax) * sizeScale,
			sparkGravity,
			Random(360.0f, 900.0f));
	}
}

void EffectSystem::Dust(const DirectX::XMFLOAT3& position)
{
	for (int i = 0; i < smokeCount; ++i)
	{
		EffectParticle* particle = Acquire(_smokes, _smokeCursor);
		if (!particle)
			return;

		const DirectX::XMFLOAT3 velocity(
			Random(-smokeSideSpeed, smokeSideSpeed),
			Random(smokeUpSpeedMin, smokeUpSpeedMax),
			Random(-1.0f, 1.0f));

		particle->Emit(position, velocity,
			Random(smokeLifeMin, smokeLifeMax),
			Random(smokeSizeMin, smokeSizeMax),
			smokeGravity,
			Random(60.0f, 180.0f));
	}
}

void EffectSystem::Flame(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& backward)
{
	for (int i = 0; i < flamePerCall; ++i)
	{
		EffectParticle* particle = Acquire(_flames, _flameCursor);
		if (!particle)
			return;

		//噴射と反対の向きへ流れる。横と奥へは、少しだけばらつかせる
		const float speed = Random(flameSpeedMin, flameSpeedMax);
		const DirectX::XMFLOAT3 velocity(
			backward.x * speed + Random(-flameSpread, flameSpread),
			backward.y * speed + Random(-flameSpread, flameSpread),
			Random(-flameSpread, flameSpread));

		particle->Emit(position, velocity,
			Random(flameLifeMin, flameLifeMax),
			Random(flameSizeMin, flameSizeMax),
			0.0f,
			Random(360.0f, 900.0f));
	}
}

void EffectSystem::Explosion(const DirectX::XMFLOAT3& position, float radius)
{
	//半径が大きいほど、大きく、速く広がる
	const float scale = radius / explosionBaseRadius;

	//炎の玉(あらゆる向きへ)
	for (int i = 0; i < explosionFireCount; ++i)
	{
		EffectParticle* particle = Acquire(_flames, _flameCursor);
		if (!particle)
			break;

		const float angle = Random(0.0f, twoPi);
		const float speed = Random(explosionFireSpeedMin, explosionFireSpeedMax) * scale;
		const DirectX::XMFLOAT3 velocity(
			std::cos(angle) * speed,
			std::sin(angle) * speed,
			Random(-sparkDepthSpread, sparkDepthSpread));

		particle->Emit(position, velocity,
			Random(explosionFireLifeMin, explosionFireLifeMax),
			Random(explosionFireSizeMin, explosionFireSizeMax) * scale,
			explosionFireGravity,
			Random(180.0f, 600.0f));
	}

	//煙(ゆっくり広がって、浮かぶ)
	for (int i = 0; i < explosionSmokeCount; ++i)
	{
		EffectParticle* particle = Acquire(_smokes, _smokeCursor);
		if (!particle)
			break;

		const float angle = Random(0.0f, twoPi);
		const float speed = Random(0.0f, explosionSmokeSpeed) * scale;
		const DirectX::XMFLOAT3 velocity(
			std::cos(angle) * speed,
			std::sin(angle) * speed + 1.0f,
			Random(-1.0f, 1.0f));

		particle->Emit(position, velocity,
			Random(explosionSmokeLifeMin, explosionSmokeLifeMax),
			Random(explosionSmokeSizeMin, explosionSmokeSizeMax) * scale,
			smokeGravity,
			Random(60.0f, 180.0f));
	}
}
