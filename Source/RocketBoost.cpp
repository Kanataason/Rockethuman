#include "RocketBoost.h"

#include <cmath>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

#include "Ragdoll.h"
#include "RoundManager.h"
#include "EffectSystem.h"
#include "InputSystem.h"
#include "UIRenderer.h"

namespace
{
	constexpr float boostSpeed = 10.0f; //噴射で足す速さ(m/s)。大きいほど、強く加速する
	constexpr float inputDelaySeconds = 0.3f; //発射してから、この間は、噴射を受け付けない(発射のスペースと、重ならないように)
	constexpr float flameSeconds = 0.5f; //炎を出し続ける時間(秒)
	constexpr float verticalInfluence = 1.0f; //W / S を押したとき、上下に曲げる強さ(1で、斜め45度くらい)
	constexpr float minDirectionSpeed = 1.0f; //これより遅いときは、進行方向がわからないので、右へ噴射する(m/s)

	//案内の表示(1280x720基準)。フォントにある文字だけを使う(漢字は、ほとんど入っていない)
	constexpr float hintCenterX = 640.0f;
	constexpr float hintCenterY = 650.0f;
	constexpr float hintScale = 0.4f;
}

void RocketBoost::Initialize(ServiceLocator& locator)
{
	_input = locator.input;
	_uiRenderer = locator.uiRenderer;
}

void RocketBoost::Bind(Ragdoll* ragdoll, RoundManager& roundManager, EffectSystem& effects)
{
	_ragdoll = ragdoll;
	_roundManager = &roundManager;
	_effects = &effects;

	//発射したら、また使える状態に戻す
	roundManager.OnShotFired.Subscribe([this]() { Reset(); });
}

void RocketBoost::Reset()
{
	_used = false;
	_secondsSinceShot = 0.0f;
	_flameRemaining = 0.0f;
}

bool RocketBoost::CanBoost() const
{
	if (!_ragdoll || !_roundManager || _used)
		return false;

	return _roundManager->GetState() == RoundState::Flying
		&& _ragdoll->IsFlying()
		&& _secondsSinceShot >= inputDelaySeconds;
}

void RocketBoost::Update(float deltaTime)
{
	if (!_ragdoll || !_roundManager || !_input)
		return;

	//飛んでいる間だけ働く
	if (_roundManager->GetState() != RoundState::Flying)
	{
		_flameRemaining = 0.0f;
		return;
	}

	_secondsSinceShot += deltaTime;

	if (CanBoost() && _input->GetKeyDown(VK_SPACE))
		Fire();

	//噴射している間は、体の後ろへ、炎を出し続ける
	if (_flameRemaining > 0.0f && _effects)
	{
		const Vector3 root = _ragdoll->GetRootPosition();
		_effects->Flame(
			DirectX::XMFLOAT3(root.x, root.y, root.z),
			DirectX::XMFLOAT3(-_direction.x, -_direction.y, -_direction.z));
		_flameRemaining -= deltaTime;
	}
}

void RocketBoost::Fire()
{
	_direction = DecideDirection();

	//全身に、同じ速さを足す(体を動かすのは Ragdoll)
	_ragdoll->ApplyBoost(Vector3(_direction.x * boostSpeed, _direction.y * boostSpeed, 0.0f));

	_used = true;
	_flameRemaining = flameSeconds;

	OnBoosted.Invoke();
}

Vector3 RocketBoost::DecideDirection() const
{
	//いまの進行方向(横から見た画面の、XとY)。ほとんど止まっているときは、右
	const Vector3 velocity = _ragdoll->GetRootVelocity();
	float x = 1.0f;
	float y = 0.0f;

	const float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
	if (speed >= minDirectionSpeed)
	{
		x = velocity.x / speed;
		y = velocity.y / speed;
	}

	//W / S（↑ / ↓）で、上下に曲げる
	float vertical = 0.0f;
	if (_input->GetKey('W') || _input->GetKey(VK_UP))
		vertical += 1.0f;
	if (_input->GetKey('S') || _input->GetKey(VK_DOWN))
		vertical -= 1.0f;
	y += vertical * verticalInfluence;

	//長さを1に戻す
	const float length = std::sqrt(x * x + y * y);
	if (length < 0.0001f)
		return Vector3(1.0f, 0.0f, 0.0f);

	return Vector3(x / length, y / length, 0.0f);
}

void RocketBoost::Draw()
{
	if (!_uiRenderer || !CanBoost())
		return;

	_uiRenderer->DrawStringCentered(L"SPACE: ロケットで かそく!   W / S: むき",
		hintCenterX, hintCenterY, hintScale,
		DirectX::XMVectorSet(1.0f, 0.85f, 0.3f, 1.0f), DirectX::XMVectorSet(0, 0, 0, 1));
}
