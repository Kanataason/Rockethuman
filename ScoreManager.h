#pragma once
#include "ServiceLoacator.h"

class ObjectStatus;

class ScoreManager
{
public:
	void Initialize(ServiceLocator& locator) {};

	void SubScribeEvent(ObjectStatus* object);
	float GetScore()const { return _totalScore; }
private:
	void AddScore(float score)
	{ _totalScore += score; }
private:
	float _totalScore = 0.0f;
};