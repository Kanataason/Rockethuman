#pragma once
#undef min
#undef max
#define NOMINMAX
#include <Windows.h>

#include <iostream>
#include <vector>
#include <DirectXMath.h>
#include <algorithm>

#include "Collider.h"
#include "RigidBody.h"

using namespace DirectX;

static constexpr int MAX_COLLIDERS = 8;
static constexpr int MAX_DEPTH = 5;
static constexpr int CHILD_COUNT = 8;
class Octree
{

public:
	Octree(const XMFLOAT3& min, const XMFLOAT3& max, int depth = 0)
		: _min(min), _max(max), _depth(depth)
	{
	}
    //コライダーをツリーに追加する
	void Insert(const std::shared_ptr<Collider>& collider);

    //指定した範囲にあって交差する可能性があるコライダーを取得　レイキャスト用
	void QueryRay(const XMFLOAT3& origin, const XMFLOAT3& direction, std::vector<std::shared_ptr<Collider>>& result);

    //指定した範囲にあって交差する可能性があるコライダーを取得
	void Query(const XMFLOAT3& min, const XMFLOAT3& max, std::vector<std::shared_ptr<Collider>>& result);

    void Clear()
    {
        _colliders.clear();

        for (int i = 0; i < CHILD_COUNT; ++i)
        {
            _children[i].reset();
        }
    }
private:

    //このNodeを空間的に8分割し、8個の子Nodeを生成する。
    void Subdivide();

    //渡されたAABBが8分割された子Nodeのどれに収まるか判定し、そのインデックス(0～7)を返す
    int GetChildIndex(const XMFLOAT3& colliderMin, const XMFLOAT3& colliderMax);

    bool Intersects(const XMFLOAT3& min, const XMFLOAT3& max, const XMFLOAT3& nodeMin, const XMFLOAT3& nodeMax)
        {
            return min.x <= nodeMax.x && max.x >= nodeMin.x &&
                min.y <= nodeMax.y && max.y >= nodeMin.y &&
                min.z <= nodeMax.z && max.z >= nodeMin.z;
        }

    //Rayが交差するか判定する
    bool IntersectsRay(const XMFLOAT3& origin, const XMFLOAT3& direction,
        const XMFLOAT3& nodeMin, const XMFLOAT3& nodeMax)
    {

        float tMin = 0.0f;
        float tMax = FLT_MAX;

        // X
        if (std::abs(direction.x) < EPSILON)
        {
            //AAbbの枠に入ってなかったら処理しない
            if (origin.x < nodeMin.x ||
                origin.x > nodeMax.x)
            {
                return false;
            }
        }
        else
        {
            //どこを通っているかの確認
            float t1 =
                (nodeMin.x - origin.x) / direction.x;

            float t2 =
                (nodeMax.x - origin.x) / direction.x;

            float tMinX = std::min(t1, t2);
            float tMaxX = std::max(t1, t2);

            tMin = std::max(tMin, tMinX);
            tMax = std::min(tMax, tMaxX);
        }

        // Y
        if (std::abs(direction.y) < EPSILON)
        {
            if (origin.y < nodeMin.y ||
                origin.y > nodeMax.y)
            {
                return false;
            }
        }
        else
        {
            float t1 =
                (nodeMin.y - origin.y) / direction.y;

            float t2 =
                (nodeMax.y - origin.y) / direction.y;

            float tMinY = std::min(t1, t2);
            float tMaxY = std::max(t1, t2);

            tMin = std::max(tMin, tMinY);
            tMax = std::min(tMax, tMaxY);
        }

        // Z
        if (std::abs(direction.z) < EPSILON)
        {
            if (origin.z < nodeMin.z ||
                origin.z > nodeMax.z)
            {
                return false;
            }
        }
        else
        {
            float t1 =
                (nodeMin.z - origin.z) / direction.z;

            float t2 =
                (nodeMax.z - origin.z) / direction.z;

            float tMinZ = std::min(t1, t2);
            float tMaxZ = std::max(t1, t2);

            tMin = std::max(tMin, tMinZ);
            tMax = std::min(tMax, tMaxZ);
        }

        return tMax >= tMin;
    }
private:
	XMFLOAT3 _min;
	XMFLOAT3 _max;
	int _depth;

	std::vector<std::shared_ptr<Collider>> _colliders;

	std::unique_ptr<Octree> _children[CHILD_COUNT];
};