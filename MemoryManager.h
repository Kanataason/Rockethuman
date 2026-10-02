#pragma once
#include "PoolAllocator.h"
#include "GameObject.h"

class MemoryManager
{
public:
	void Initialize();

	GameObject* AllocateGameObject() { return _objectPool.Allocate(); };
	void FreeGameObject(GameObject* object);

private:
	//ここに各プール
	PoolAllocator<GameObject> _objectPool;
};