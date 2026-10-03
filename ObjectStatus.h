#pragma once
#include "Component.h"
#include "VectorUtility.h"
#include "Event.h"

class ObjectStatus : public Component
{
public:
	Event<float> OnSendScore;

	void SetScore(float score) { _score = score; }
	void OnCollisionEnter(Collider* other, const CollisionResult& result)override;
private:
	Vector3 _initializePos;
	float _score = 10.0f;
};