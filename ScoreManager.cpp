#include <algorithm>

#include "ScoreManager.h"
#include "ObjectStatus.h"

void ScoreManager::SubScribeEvent(ObjectStatus* status)
{
	status->OnSendScore.Subscribe([this](HitInfo hit) { AddHit(hit); });
}

void ScoreManager::Update(float deltaTime)
{
	if (_comboTimer <= 0.0f)
		return;

	_comboTimer -= deltaTime;
	if (_comboTimer <= 0.0f)
		ResetCombo();
}

void ScoreManager::AddHit(const HitInfo& hit)
{
	// 制限時間内に次の的に当たったらコンボ。そうでなければ、1から数え直す
	_combo = (_comboTimer > 0.0f) ? _combo + 1 : 1;
	_comboTimer = _comboWindowSeconds;

	// (std::min) と括弧で囲むのは、Windows.h の min/max マクロと名前が衝突しないようにするため
	const float multiplier = (std::min)(1.0f + _comboStep * static_cast<float>(_combo - 1), _maxMultiplier);
	float gained = hit.baseScore * multiplier;
	if (hit.isHead)
		gained += _headBonus;

	_totalScore += gained;

	ScoreGain gain;
	gain.points = gained;
	gain.combo = _combo;
	gain.isHead = hit.isHead;
	gain.position = hit.position;
	OnScoreGained.Invoke(gain);
}

void ScoreManager::AddDistanceBonus(float meters)
{
	_totalScore += (std::max)(0.0f, meters) * _pointsPerMeter;
}

void ScoreManager::ResetCombo()
{
	_combo = 0;
	_comboTimer = 0.0f;
}
