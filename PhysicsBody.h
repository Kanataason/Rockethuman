#pragma once
#include <iostream>
#include "VectorUtility.h"
#include "Ragdoll.h"

struct PhysicsBody
{
    std::shared_ptr<RigidBody> rigidBody;
    std::shared_ptr<Collider> collider;

    RagdollBone ragdollBone;


    //前回の移動量とかを保存する
    Vector3 restLastPos{};
    Quaternion restLastRot{};

    bool restHasLast = false;
};