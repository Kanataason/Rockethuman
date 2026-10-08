#pragma once

class Collider;

class IOnCollider
{
public:
    virtual ~IOnCollider() = default;

    virtual void OnColliderEnter(Collider* other) = 0;
    virtual void OnColliderStay(Collider* other) = 0;
    virtual void OnColliderExit(Collider* other) = 0;
};