#pragma once
#include "Transform.h"

class Object
{
public :
	virtual ~Object() = default;	

	virtual void Update(float deltaTime) {}

	void SetTransform(const Transform& transform)
	{
		_transform = transform;
	}

	const Transform& GetTransform() const
	{
		return _transform;
	}

	Transform& GetTransform()
	{
		return _transform;
	}
protected:
	Transform _transform;
};