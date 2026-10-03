#pragma once
#include <vector>
#include <string>

#include "CollisionManager.h"
#include "RigidBody.h"
#include "Collider.h"
#include "PhysicsBody.h"
#include "RagdollSystem.h"

class DebugDraw;

constexpr int SubStepCount = 5;//サブステップの回数

class PhysicsSystem
{
public:
    PhysicsSystem(DebugDraw* debug) : _collisionManager(debug) {}

    void Initialize();

    // 毎フレーム呼び出す物理更新のエントリーポイント
    void FixedUpdate(float deltaTime);

    // RigidBody / Collider / Jointの登録
    // RagdollBoneがunique_ptrを持つためPhysicsBodyはコピーできない。値で受け取り、内部でmoveする
    void AddPhysicsBody(PhysicsBody body);

    // RigidBodyのデストラクタから、自動的に呼ばれる(手動で呼ぶ必要はない)
    void RemoveRigidBody(RigidBody* rigidBody);

    // Colliderのデストラクタから、自動的に呼ばれる(手動で呼ぶ必要はない)
    // CollisionManager側の登録解除と、_rigidBodies内の参照の後始末を両方行う
    void RemoveCollider(Collider* collider);

    void AddJoint(Joint* joint) { _jointList.push_back(joint); };
    void AddCollider(Collider* collider) { collider->SetRegistry(this); _collisionManager.AddCollider(collider); }

    CollisionManager* GetCollisionManager() { return &_collisionManager; }

    bool RayCast(const XMFLOAT3& origin, const XMFLOAT3& direction, const float& distance, XMFLOAT3& hitPos);

    // ragdollのボーンごとの、colliderのずれ(ローカルオフセット・回転オフセット)と、
    // 重心オフセットが信頼できるかどうかを更新する
    void UpdateRagdollBoneOffsets(RigidBody* rigidBody,
        const Vector3& localOffset, const Quaternion& localRotOffset, bool hasReliableComOffset);

    void ResetRagdollTimer() { _ragdollElapsedTime = 0.0f; _ragdollSystem.ResetRest(); }
    // 休止(倒れて静止したら止める処理)を使うかを切り替える。吊り下げ中は false にする
    void SetRagdollRestEnabled(bool enabled) { _ragdollSystem.SetRestEnabled(enabled); }
private:
    // ---- FixedUpdate内部で使うステップ処理 ----

    // 動かない物でも、GameObject側で動かされたときに追従できるようにしている
    void SyncStaticColliders();

    // GameObjectのTransformに合わせる
    void SyncKinematicBodies();

    // 位置だけを書き出す
    void SyncGameObjectTransforms();

    // RigidBodyの位置・回転から、colliderの位置・回転を計算して設定する
    void SyncCollidersFromRigidBodies();

    // 全ボーンの重力・速度による位置更新と、
    // ボーンごとの重力トルクを加える
    void IntegrateDynamics(float subDeltaTime);

    // すべてのジョイントの拘束を解く
    // 片方だけに解いても影響が伝わらないのを防止のため、両方を解く
    void SolveJoints(float subDeltaTime);

    // 接触判定
    // 衝突ペアごとに全ボーンを走査するので、計算量は「ペア数 × ボーン数」
    void SolveCollisions(float deltaTime, float subDeltaTime);

    // 同じサブステップで回転に反映する
    // キネマティックとスリープ中は対象外
    void IntegrateRotations(float subDeltaTime);

    // 1つのRigidBodyの位置に、疑似速度 * 時間 を足す
    void ApplyPseudoVelocityToPosition(RigidBody& rigidBody, float subDeltaTime);

    // 衝突で溜めた疑似速度(めり込みの押し戻し)を、全ボーンの位置に反映する
    void ApplyPseudoVelocityToPositions(float subDeltaTime);

    // このボーンが、今回の衝突ペアの当事者かを調べる
    bool TryGetContactInfo(const PhysicsBody& body, const CollisionResult& result,
        Vector3& contactNormal, bool& isGround) const;

    // ボーン同士の接触の処理
    void ResolveBoneContact(PhysicsBody& body, const CollisionResult& result,
        const Vector3& contactNormal, float subDeltaTime);

    // 接触ペアの「相手側」のRigidBodyを探して返す
    RigidBody* FindOtherRigidBody(PhysicsBody& body, const CollisionResult& result);

    // 地面との接触の処理
    void ResolveGroundContact(PhysicsBody& body, const CollisionResult& result,
        const Vector3& contactNormal, float deltaTime, float subDeltaTime);

    // ボーン同士がぶつかった衝撃を、角速度に変える
    void ApplyBoneImpactAngularImpulse(PhysicsBody& body, const CollisionResult& result,
        const Vector3& contactNormal, float velocityAlongNormalBefore);

private:
    float _ragdollElapsedTime = 0.0f;
    std::vector<PhysicsBody> _rigidBodies;
    // 所有者はRagdoll(RagdollBone::joint)。ここでは、解くために参照するだけ
    std::vector<Joint*> _jointList;

    RagdollSystem _ragdollSystem;
    CollisionManager _collisionManager;
};
