#include "PhysicsSystem.h"
#include "DebagUtility.h"

#include "ConsoleUtility.h"
#include "GameObject.h"

// 地面との接触。地面は動かないので、自分の速度だけを変える。
static void ApplyGroundContactImpulse(RigidBody& body, const Vector3& contactPoint, const Vector3& normal)
{
    // ボーン原点から接触点までのベクトル
    Vector3 contactArm = contactPoint - body.GetPosition();
    Vector3 linearVelocity = body.GetVelocity();
    Vector3 angularVelocity = body.GetAngularVelocity();

    // 接触点の実際の速度(回転による速度も含む)
    const Vector3 contactPointVelocity = linearVelocity + Vector::Cross(angularVelocity, contactArm);
    const float approachSpeed = Vector::Dot(contactPointVelocity, normal);

    // 0以上なら地面から離れる向きなので、何もしない
    if (approachSpeed >= 0.0f)
        return;

    const Vector3 armCrossNormal = Vector::Cross(contactArm, normal);
    const float inverseMass = 1.0f / body.GetMass();
    const float inverseInertia = 1.0f / body.GetInertia();
    const float effectiveInverseMass = inverseMass + inverseInertia * Vector::SqMagnitude(armCrossNormal);
    if (effectiveInverseMass <= 0.0f)
        return;

    const float impulseMagnitude = -approachSpeed / effectiveInverseMass; // no bounce
    // 線形速度は法線方向に、角速度は (腕 x 法線) 方向に変化する
    body.SetVelocity(linearVelocity + normal * (impulseMagnitude * inverseMass));
    body.SetAngularVelocity(angularVelocity + armCrossNormal * (impulseMagnitude * inverseInertia));
}

// ボーン同士の接触。地面と違い、相手も動くので、双方に逆向きのインパルスを分配する。
static void ApplyBoneContactImpulse(RigidBody& self, RigidBody* other, const Vector3& contactPoint, const Vector3& normal)
{
    // それぞれの、接触点までの腕・線形速度・角速度
    Vector3 selfContactArm = contactPoint - self.GetPosition();
    Vector3 selfLinearVelocity = self.GetVelocity();
    Vector3 selfAngularVelocity = self.GetAngularVelocity();
    Vector3 otherContactArm(0.0f, 0.0f, 0.0f);
    Vector3 otherLinearVelocity(0.0f, 0.0f, 0.0f);
    Vector3 otherAngularVelocity(0.0f, 0.0f, 0.0f);
    if (other)
    {
        otherContactArm = contactPoint - other->GetPosition();
        otherLinearVelocity = other->GetVelocity();
        otherAngularVelocity = other->GetAngularVelocity();
    }

    // 接触点での、それぞれの実際の速度と、その差(相対速度)
    Vector3 selfContactPointVelocity =
        selfLinearVelocity + Vector::Cross(selfAngularVelocity, selfContactArm);
    const Vector3 otherContactPointVelocity =
        otherLinearVelocity + Vector::Cross(otherAngularVelocity, otherContactArm);
    const Vector3 relativeContactVelocity = selfContactPointVelocity - otherContactPointVelocity;
    const float relativeApproachSpeed = Vector::Dot(relativeContactVelocity, normal);

    // 離れていく向きなら何もしない
    if (relativeApproachSpeed >= 0.0f)
        return;

    const Vector3 selfArmCrossNormal = Vector::Cross(selfContactArm, normal);
    const Vector3 otherArmCrossNormal = Vector::Cross(otherContactArm, normal);
    const float selfInverseMass = 1.0f / self.GetMass();
    const float selfInverseInertia = 1.0f / self.GetInertia();
    // other がいないときは逆質量・逆慣性を 0(=無限に重い)として扱う
    const float otherInverseMass = other ? 1.0f / other->GetMass() : 0.0f;
    const float otherInverseInertia = other ? 1.0f / other->GetInertia() : 0.0f;
    const float effectiveInverseMass =
        selfInverseMass + selfInverseInertia * Vector::SqMagnitude(selfArmCrossNormal) +
        otherInverseMass + otherInverseInertia * Vector::SqMagnitude(otherArmCrossNormal);
    if (effectiveInverseMass <= 0.0f)
        return;

    const float impulseMagnitude = -relativeApproachSpeed / effectiveInverseMass;
    self.SetVelocity(selfLinearVelocity + normal * (impulseMagnitude * selfInverseMass));
    self.SetAngularVelocity(selfAngularVelocity + selfArmCrossNormal * (impulseMagnitude * selfInverseInertia));
    if (other)
    {
        // 作用・反作用: 相手には逆向きに加える
        other->SetVelocity(otherLinearVelocity - normal * (impulseMagnitude * otherInverseMass));
        other->SetAngularVelocity(otherAngularVelocity - otherArmCrossNormal * (impulseMagnitude * otherInverseInertia));
    }
}

void PhysicsSystem::RemoveRigidBody(RigidBody* rigidBody)
{
    // RigidBodyのデストラクタから、自動的に呼ばれる(手動で呼ぶ必要はない)
    std::erase_if(_rigidBodies, [&](const PhysicsBody& body)
        {
            return body.rigidBody == rigidBody;
        });
}

void PhysicsSystem::RemoveCollider(Collider* collider)
{
    // Colliderのデストラクタから、自動的に呼ばれる(手動で呼ぶ必要はない)
    // _rigidBodiesの中に、このColliderを指しているエントリが残っていると、
    // SyncStaticColliders等が破棄済みのColliderを触ってクラッシュするので、先に消す
    std::erase_if(_rigidBodies, [&](const PhysicsBody& body)
        {
            return body.collider == collider;
        });

    _collisionManager.ReleaseCollider(collider->GetID());
}

void PhysicsSystem::FixedUpdate(float deltaTime)
{

    // 物理の影響を受けないオブジェクトの場所同期処理
    SyncKinematicBodies();
    SyncStaticColliders();

    // RigidBody -> Colliderの回転と場所同期
    //計算前にColliderの場所を最新の位置にしておく
    SyncCollidersFromRigidBodies();


    //サブステップ用のdeltaTime
    const float subDeltaTime = deltaTime / (float)SubStepCount;

    for (int i = 0; i < SubStepCount; ++i)
    {
        // 重力による加速、速度による位置更新、
        IntegrateDynamics(subDeltaTime);


        // ragdoll全体の重心を使って、ルート(腰)に重力トルクを加える。これで全体が倒れる。
        _ragdollSystem.ApplyRootGravityTorque(_rigidBodies, subDeltaTime);


        // 前のサブステップの疑似速度(位置補正用)とトルクを消す。
        for (auto& body : _rigidBodies)
        {
            if (body.rigidBody)
            {
                body.rigidBody->ClearPseudoVelocity();
                body.rigidBody->ClearTorque();
            }
        }

     
        // ジョイントの拘束(位置・角度の制限)を解く
        SolveJoints(subDeltaTime);


        //ジョイントで補正した分だけColliderに反映させる
        SyncCollidersFromRigidBodies();


        // 衝突検出と、衝突イベントの通知
        _collisionManager.Intersects();
        _collisionManager.ResolveEvent();


        // 衝突の結果に応じて、速度・疑似速度・角速度を更新する
        SolveCollisions(deltaTime, subDeltaTime);


        // 接触処理のあとで、トルク→角速度→回転を更新する
        IntegrateRotations(subDeltaTime);

        
        //疑似速度をrigidBodyに反映
        ApplyPseudoVelocityToPositions(subDeltaTime);


        // 次のサブステップのために、Colliderを最新にする
        SyncCollidersFromRigidBodies();

    }


    // ragdollが倒れて、ほぼ静止したらスリープさせて凍結する
    _ragdollSystem.UpdateRest(_rigidBodies, deltaTime);


    //Debugの線
    if(RagdollTuning::IsDebug)
    _collisionManager.DebugCollider();

  
    //物理演算の結果をGameObjectに反映する
    SyncGameObjectTransforms();

}
void PhysicsSystem::SyncGameObjectTransforms()
{
    for (auto& body : _rigidBodies)
    {
        if (body.rigidBody)
        {
            if (auto owner = body.rigidBody->GetOwner())
            {
                owner->GetTransform().Position = Vector::fromV3ToF3(body.rigidBody->GetPosition());
            }
        }
    }
}
void PhysicsSystem::SyncKinematicBodies()
{
    for (auto& body : _rigidBodies)
    {
        if (body.rigidBody &&
            body.rigidBody->IsKinematic())
        {
            if (auto owner = body.rigidBody->GetOwner())
            {
                //親の場所と、rigidBosyの場所を同期させる
                body.rigidBody->SetPosition(
                    Vector::fromF3ToV3(
                        owner->GetTransform().Position
                    )
                );
            }
        }
    }
}
void PhysicsSystem::SyncStaticColliders()
{
    for (auto& body : _rigidBodies)
    {
        if (body.rigidBody || !body.collider)
            continue;

        if (auto owner = body.collider->GetOwner())
            body.collider->SetPosition(owner->GetTransform().Position);
    }
}

void PhysicsSystem::SyncCollidersFromRigidBodies()
{
    for (auto& body : _rigidBodies)
    {
        if (body.rigidBody && body.collider)
        {
            Vector3 rbPos = body.rigidBody->GetPosition();
            Quaternion rbRot = body.rigidBody->GetRotation();
            // ローカルオフセットを回転させてから足す。
            Vector3 colliderPos =
                rbPos + rbRot * body.ragdollBone.colliderLocalOffset;

            Quaternion colliderRot =
                rbRot * body.ragdollBone.colliderLocalRotOffset;

            body.collider->SetPosition(Vector::fromV3ToF3(colliderPos));
            body.collider->SetRotation(colliderRot);
        }
    }

}

void PhysicsSystem::IntegrateDynamics(float subDeltaTime)
{
    for (auto& body : _rigidBodies)
    {
        if (body.rigidBody &&
            !body.rigidBody->IsKinematic() &&
            !body.rigidBody->IsSleeping())
        {
            //重力を与える
            body.rigidBody->FixedUpdate(subDeltaTime);

            //設置中かどうかを確認して、信頼できる物だけ重力トルクを与える
            // 足と手は、接地中は重力トルクを使わない(慣性が小さく、地面の押し戻しとぶつかって震えるため)
            const bool isExtremity =
                body.ragdollBone.name.find("Foot") != std::string::npos ||
                body.ragdollBone.name.find("Hand") != std::string::npos;
            const bool torqueWhenGrounded =
                RagdollTuning::kGravityTorqueWhenGrounded &&
                (!isExtremity || RagdollTuning::kExtremityGravityTorqueWhenGrounded);

            if (body.ragdollBone.hasReliableComOffset &&
                (torqueWhenGrounded || !body.rigidBody->IsGrounded()))
            {
                body.rigidBody->ApplyGravityTorque(
                    body.ragdollBone.colliderLocalOffset,
                    subDeltaTime
                );
            }
        }

    }
}

void PhysicsSystem::SolveJoints(float subDeltaTime)
{
    for (auto& joint : _jointList)
    {
        if (joint)
            joint->ApplyRestoringTorque();
    }

    // 1つのジョイントを解いて、その結果を即座にボーンの位置へ反映する
    auto solveJointImmediate = [&](Joint* joint)
        {
            if (!joint) return;

            if (joint->GetBodyA()->IsSleeping() && joint->GetBodyB()->IsSleeping())
                return;

            joint->Solve(subDeltaTime);                 // 位置の拘束
            joint->SolveAngularConstraint();            // 角度の制限
            joint->RotateBodyByAnchorCorrection();      // アンカーのずれを回転補正

            for (auto& body : { joint->GetBodyA(), joint->GetBodyB() })
            {
                if (body && !body->IsKinematic())
                {
                    ApplyPseudoVelocityToPosition(*body, subDeltaTime);
                    body->ClearPseudoVelocity();
                }
            }
        };

    // 前方(root->末端)
    for (auto it = _jointList.begin(); it != _jointList.end(); ++it)
    {
        solveJointImmediate(*it);
    }

    // 後方(末端->root)
    for (auto it = _jointList.rbegin(); it != _jointList.rend(); ++it)
    {
        solveJointImmediate(*it);
    }
}

void PhysicsSystem::SolveCollisions(float deltaTime,float subDeltaTime)
{
    //地面フラグを初期化
    for (auto& body : _rigidBodies)
    {
        if (body.rigidBody)
            body.rigidBody->SetGrounded(false);
    }


    for (const auto& pairResultList :_collisionManager.GetCollisionResults())
    {
        auto& result = pairResultList.second;

        if (!result.colliderA ||!result.colliderB)
            continue;


        for (auto& body : _rigidBodies)
        {
            //影響を受けない、動かないものは処理しない
            if (!body.rigidBody ||!body.collider ||
                body.rigidBody->IsKinematic() ||body.rigidBody->IsSleeping())
                continue;

            // このボーンが今回のペアの当事者かを調べ、法線と地面かどうかを得る
            Vector3 normal;
            bool isGround = false;
            if (!TryGetContactInfo(body, result, normal, isGround))
                continue;

            //速度をどこよりも先に(まだ何も消されてない状態で)測っておく
            float vAlongNormalBefore =
                Vector::Dot(body.rigidBody->GetVelocity(), normal);


            if (isGround)
            {
                ResolveGroundContact(body, result, normal, deltaTime, subDeltaTime);
            }
            else
            {
                ResolveBoneContact(body, result, normal, subDeltaTime);
                ApplyBoneImpactAngularImpulse(body, result, normal, vAlongNormalBefore);
            }

            //通常Velocity
            //めり込む向きの速度成分を消す
            body.rigidBody->RemoveVelocityAlongNormal(normal);

            //摩擦(地面とボーン同士で係数を変える)
            body.rigidBody->ApplyFriction(normal,isGround ? RagdollTuning::kGroundFriction : RagdollTuning::kBoneFriction, subDeltaTime);
        }
    }
}


void PhysicsSystem::ResolveGroundContact(PhysicsBody& body, const CollisionResult& result,
    const Vector3& contactNormal, float deltaTime, float subDeltaTime)
{
    RigidBody& rigidBody = *body.rigidBody;

    //下向き(-Y)の速度だけを消す(平らな地面の前提)
    Vector3 velocity = rigidBody.GetVelocity();
    if (velocity.y < 0.0f)
        velocity.y = 0.0f;
    rigidBody.SetVelocity(velocity);

    constexpr float penetrationCorrectionRate = RagdollTuning::penetrationCorrectionRate;
    constexpr float penetrationSlop = RagdollTuning::penetrationSlop;
    const float correctedPenetration = std::max(0.0f, result.penetration - penetrationSlop);
    // めり込み量を、サブステップの時間で割って速度にする
    rigidBody.AddPseudoVelocity(
        contactNormal * (penetrationCorrectionRate * correctedPenetration / subDeltaTime));

    rigidBody.SetGrounded(true);
    //接触点で、めり込む向きの速度を線形+角速度で打ち消す
    const float groundAngularDamp =
        std::pow(RagdollTuning::kGroundAngularDampPerFrame, subDeltaTime / deltaTime);
    rigidBody.SetAngularVelocity(rigidBody.GetAngularVelocity() * groundAngularDamp);

    ApplyGroundContactImpulse(rigidBody, result.contactPoint, contactNormal);
}

void PhysicsSystem::ResolveBoneContact(PhysicsBody& body, const CollisionResult& result,
    const Vector3& contactNormal, float subDeltaTime)
{
    RigidBody& selfRigidBody = *body.rigidBody;
    RigidBody* otherRigidBody = FindOtherRigidBody(body, result);

    // 逆質量で押し出しを分配(軽いボーンが動き、重い腰や胴はほとんど動かない)
    const float selfInverseMass = 1.0f / selfRigidBody.GetMass();
    const float otherInverseMass = otherRigidBody ? 1.0f / otherRigidBody->GetMass() : 0.0f;
    const float pushShare = selfInverseMass / (selfInverseMass + otherInverseMass);

    //小さい押し出しは無視して、大きすぎたら範囲内に収める
    float pushDepth = std::max(0.0f, result.penetration - RagdollTuning::kBoneContactSlop);
    pushDepth = std::min(pushDepth, RagdollTuning::kBoneContactMaxPush);

    selfRigidBody.AddPseudoVelocity(
        contactNormal * (RagdollTuning::kBoneContactBeta * pushDepth * pushShare / subDeltaTime));

    ApplyBoneContactImpulse(selfRigidBody, otherRigidBody, result.contactPoint, contactNormal);
}


RigidBody* PhysicsSystem::FindOtherRigidBody(PhysicsBody& body, const CollisionResult& result)
{
    const Collider* otherCollider =
        (body.collider == result.colliderA) ? result.colliderB : result.colliderA;
    for (auto& other : _rigidBodies)
    {
        if (other.collider == otherCollider && other.rigidBody &&
            !other.rigidBody->IsKinematic())
        {
            return other.rigidBody;
        }
    }
    return nullptr; // 見つからない
}


bool PhysicsSystem::TryGetContactInfo(const PhysicsBody& body, const CollisionResult& result,
    Vector3& contactNormal, bool& isGround) const
{
    if (body.collider == result.colliderA)
    {
        contactNormal = result.normal;
        isGround = (result.colliderB->GetCollisionGroup() == 3);
        return true;
    }
    if (body.collider == result.colliderB)
    {
        contactNormal = -result.normal;
        isGround = (result.colliderA->GetCollisionGroup() == 3);
        return true;
    }
    return false; // このbodyは今回のペアと無関係
}


void PhysicsSystem::ApplyBoneImpactAngularImpulse(PhysicsBody& body, const CollisionResult& result,
    const Vector3& contactNormal, float velocityAlongNormalBefore)
{
    if (velocityAlongNormalBefore >= -0.05f)
        return;

    // 重心
    const Vector3 centerOfMass =
        body.rigidBody->GetPosition() +
        body.rigidBody->GetRotation() * body.ragdollBone.colliderLocalOffset;
    // ピボットではなく実際の重心から測る
    const Vector3 contactArm = result.contactPoint - centerOfMass;

    const Vector3 linearImpulse =
        contactNormal * (-velocityAlongNormalBefore * body.rigidBody->GetMass());
    Vector3 angularImpulse = Vector::Cross(contactArm, linearImpulse);
    body.rigidBody->AddAngularImpulse(angularImpulse);
}
void PhysicsSystem::IntegrateRotations(float subDeltaTime)
{
    for (auto& body : _rigidBodies)
    {
        if (!body.rigidBody || body.rigidBody->IsKinematic() || body.rigidBody->IsSleeping())
            continue;

        body.rigidBody->ApplyTorque(subDeltaTime);      
        body.rigidBody->IntegrateRotation(subDeltaTime);  
    }
}
void PhysicsSystem::ApplyPseudoVelocityToPositions(float subDeltaTime)
{
    for (auto& body : _rigidBodies)
    {
        if (!body.rigidBody || body.rigidBody->IsKinematic())
            continue;

        ApplyPseudoVelocityToPosition(*body.rigidBody, subDeltaTime);
    }
}
void PhysicsSystem::ApplyPseudoVelocityToPosition(RigidBody& rigidBody, float subDeltaTime)
{
    rigidBody.SetPosition(rigidBody.GetPosition() + rigidBody.GetPseudoVelocity() * subDeltaTime);
}

bool PhysicsSystem::RayCast(const XMFLOAT3& origin, const XMFLOAT3& direction, const float& distance, XMFLOAT3& hitPos)
{
 return _collisionManager.RayCast(origin, direction, distance, hitPos);
}

void PhysicsSystem::AddPhysicsBody(PhysicsBody Body)
{
    // 登録先を覚えさせておく。所有者(GameObject)が破棄されてRigidBodyが消えるとき、
    // デストラクタが自動的にRemoveRigidBodyを呼ぶので、手動の後片付けが不要になる
    if (Body.rigidBody)
        Body.rigidBody->SetRegistry(this);

    if (Body.collider)
    {
        Body.collider->SetRegistry(this);
        _collisionManager.AddCollider(Body.collider);
    }

    _rigidBodies.push_back(std::move(Body));
}

void PhysicsSystem::UpdateRagdollBoneOffsets(RigidBody* rigidBody,
    const Vector3& localOffset, const Quaternion& localRotOffset, bool hasReliableComOffset)
{
    //ragdollのボーンごとに更新
    for (auto& body : _rigidBodies)
    {
        if (body.rigidBody == rigidBody)
        {
            body.ragdollBone.colliderLocalOffset = localOffset;
            body.ragdollBone.colliderLocalRotOffset = localRotOffset;
            body.ragdollBone.hasReliableComOffset = hasReliableComOffset;
            return;
        }
    }
}
