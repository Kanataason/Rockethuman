#pragma once
#include "VectorUtility.h" 

class Collider;

/// <summary>
/// コライダー同士の当たり判定の結果構造体
/// </summary>
struct CollisionResult
{
    Collider* colliderA;
    Collider* colliderB;

    Vector3 normal{};//方向
    float penetration;//めり込み度

    Vector3 contactPoint{};//ヒットポイント

    CollisionResult()
    {
        colliderA = nullptr;
        colliderB = nullptr;

        penetration = 0.0f;
    }
};