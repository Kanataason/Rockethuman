#pragma once
#include <algorithm>
#include <iostream>
#include "RigidBody.h"
#include "VectorUtility.h"
#include "ConsoleUtility.h"

class PhysicsSystem;

class Joint
{
public:
    Joint(RigidBody* bodyA,
        RigidBody* bodyB):_bodyA(bodyA),_bodyB(bodyB)
    {
        Vector3 parentPos = _bodyA->GetPosition();
        Vector3 childPos = _bodyB->GetPosition();
        Quaternion aRot = _bodyA->GetRotation();
        Quaternion bRot = _bodyB->GetRotation();

        Vector3 anchor = childPos; //子の位置＝実際の関節位置

        _localAnchorA = aRot.Inverse() * (anchor - parentPos);
        _localAnchorB = bRot.Inverse() * (anchor - childPos); //これは常にゼロベクトル

    }
    // 登録先のPhysicsSystemがあれば、自動的に登録を解除する(定義はJoint.cpp)
    // PhysicsSystemは生ポインタで持つだけなので、これがないと、シーンを作り直したときに、
    // 破棄済みのJointを触ってクラッシュする
    virtual ~Joint();

    // PhysicsSystem::AddJointが呼ぶ(手動で呼ぶ必要はない)
    void SetRegistry(PhysicsSystem* registry) { _registry = registry; }

    //位置拘束とアンカー点の速度拘束
    //疑似速度にすれを渡して角度拘束を呼ぶ
    virtual void Solve(float deltaTime) = 0;

    //初期姿勢からのヒンジ軸まわりの角度ずれに応じて、元に戻すトルクを溜める
    //ここでは直接値を加えるのではなく溜めるだけにしてrigidBodyで一回で処理をする
    virtual void ApplyRestoringTorque() {};

    // 軸に垂直な相対角速度を消す
    virtual void SolveAngularConstraint() = 0;

    // 可動範囲から余裕(limitTolerance)を超えて外れているか
    // 境界ぎりぎりで「範囲内/範囲外」が交互に判定されるのを防ぐための余裕
    virtual bool IsOutOfLimit(float hingeAngle) const = 0;

    //Aから見たBの相対回転」から、初期姿勢を0度としたヒンジ軸まわりの符号付き角度(ラジアン)を返す
    virtual float CalculateAngleError(const Quaternion& relativeRotation) = 0;


    //Jointの位置誤差を質量比で分配する関数
    void CalculateAnchorCorrection(Vector3& correctionA, Vector3& correctionB)
    {
        Vector3 bodyAPos = _bodyA->GetPosition();
        Vector3 bodyBPos = _bodyB->GetPosition();

        Quaternion bodyARot = _bodyA->GetRotation();
        Quaternion bodyBRot = _bodyB->GetRotation();


        // アンカーはローカル座標で持っているので、現在の位置と回転でワールド座標へ変換する
        Vector3 worldAnchorA =
            bodyAPos + bodyARot * _localAnchorA;

        Vector3 worldAnchorB =
            bodyBPos + bodyBRot * _localAnchorB;

        // Jointのズレ
        Vector3 error =
            worldAnchorB - worldAnchorA;

        float errLen = error.Length();
        Vector3 clampedError = error;

        // 1回で直す量に上限をかける
        // ラグドールが大きくずれたときに全部を一気に直そうとすると、体が跳ねたり発散したりするため
        if (errLen > _maxCorrection)
        {
            clampedError = error * (_maxCorrection / errLen);
        }

        // 逆質量
        //軽い方ほど多く動かし、重い方は少なく動かす
        // Kinematic の骨は動かせない(逆質量0)ので、位置の補正は、相手がすべて受け持つ。
        // 吊る骨(Kinematic)を持ち上げたとき、子の骨が補正を全部受け取って、しっかり引っ張られるようにする
        float invMassA =
            _bodyA->IsKinematic() ? 0.0f : 1.0f / _bodyA->GetMass();

        float invMassB =
            _bodyB->IsKinematic() ? 0.0f : 1.0f / _bodyB->GetMass();

        float sum =
            invMassA + invMassB;

        if (sum <= 0.0f)return;


        // Massに応じて補正量を分配
        correctionA =
            clampedError * (invMassA / sum);

        correctionB =
            clampedError * (invMassB / sum);
    }

    //bodyAとbodyBの回転を補正する
    void RotateBodyByAnchorCorrection()
    {
        if (!RagdollTuning::kAnchorRotation)
            return;

        Vector3 correctionA, correctionB;
        CalculateAnchorCorrection(correctionA, correctionB);

        RotateBodyToMoveAnchor(_bodyA, _localAnchorA, correctionA);
        RotateBodyToMoveAnchor(_bodyB, _localAnchorB, correctionB);
    }

    // アンカー点の速度拘束: 2つのアンカー点(ジョイントでつないだ点)が、同じ速度になるようにする。
    // インパルス(瞬間的な力積)を、並進速度と角速度の両方に加える
    void SolveAnchorVelocity(RigidBody* a, RigidBody* b,
        Vector3 rA, Vector3 rB, float strength)
    {
        // 逆質量・逆慣性。動かせない(キネマティックな)ボディは0にして、インパルスを受けないようにする。
        const float invMA = a->IsKinematic() ? 0.0f : 1.0f / a->GetMass();
        const float invMB = b->IsKinematic() ? 0.0f : 1.0f / b->GetMass();
        const float invIA = a->IsKinematic() ? 0.0f : 1.0f / a->GetInertia();
        const float invIB = b->IsKinematic() ? 0.0f : 1.0f / b->GetInertia();

        // 速度は最初にまとめて読み、最後にまとめて書く(途中で値が変わって計算がずれるのを防ぐ)。
        Vector3 vA = a->GetVelocity();
        Vector3 vB = b->GetVelocity();
        Vector3 wA = a->GetAngularVelocity();
        Vector3 wB = b->GetAngularVelocity();

        // アンカー点の相対速度(消したいもの)。
        // 点の速度 = 並進速度 + 角速度 × 原点からその点までのベクトル(rA, rB)。
        Vector3 vrel = (vB + Vector::Cross(wB, rB)) - (vA + Vector::Cross(wA, rA));

        //    並進だけでなく、回転を通じてアンカーが動く分も含まれるので、単純な質量の逆数にはならない。
        const float rA2 = rA.x * rA.x + rA.y * rA.y + rA.z * rA.z;
        const float rB2 = rB.x * rB.x + rB.y * rB.y + rB.z * rB.z;
        const float k = invMA + invMB + invIA * rA2 + invIB * rB2;
        const float ra[3] = { rA.x, rA.y, rA.z };
        const float rb[3] = { rB.x, rB.y, rB.z };
        float m[3][3];
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                m[i][j] = (i == j ? k : 0.0f) - invIA * ra[i] * ra[j] - invIB * rb[i] * rb[j];

        //の逆行列を求める(3x3の余因子展開)。まず行列式。
        const float det =
            m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) -
            m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
            m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
        // 行列式がほぼ0なら逆行列がない。たとえば両方のボディが動かせない場合は、解けないので何もしない。
        if (std::fabs(det) < 1e-12f)
            return;
        const float id = 1.0f / det;

        float inv[3][3];
        inv[0][0] = (m[1][1] * m[2][2] - m[1][2] * m[2][1]) * id;
        inv[0][1] = (m[0][2] * m[2][1] - m[0][1] * m[2][2]) * id;
        inv[0][2] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * id;
        inv[1][0] = (m[1][2] * m[2][0] - m[1][0] * m[2][2]) * id;
        inv[1][1] = (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * id;
        inv[1][2] = (m[0][2] * m[1][0] - m[0][0] * m[1][2]) * id;
        inv[2][0] = (m[1][0] * m[2][1] - m[1][1] * m[2][0]) * id;
        inv[2][1] = (m[0][1] * m[2][0] - m[0][0] * m[2][1]) * id;
        inv[2][2] = (m[0][0] * m[1][1] - m[0][1] * m[1][0]) * id;

        // 目標の速度変化 = 相対速度を打ち消す向きに、strength の割合だけ。
        const float t[3] = { -vrel.x * strength, -vrel.y * strength, -vrel.z * strength };

        //必要なインパルスを求める。
        Vector3 lambda(
            inv[0][0] * t[0] + inv[0][1] * t[1] + inv[0][2] * t[2],
            inv[1][0] * t[0] + inv[1][1] * t[1] + inv[1][2] * t[2],
            inv[2][0] * t[0] + inv[2][1] * t[1] + inv[2][2] * t[2]);

        b->SetVelocity(vB + lambda * invMB);
        b->SetAngularVelocity(wB + Vector::Cross(rB, lambda) * invIB);
        a->SetVelocity(vA - lambda * invMA);
        a->SetAngularVelocity(wA - Vector::Cross(rA, lambda) * invIA);
    }
    
    //ここからはゲッターとセッター

    virtual void SetRotationLimit(float min, float max) {};
    virtual void SetAxis(const Vector3& axis) {}
    virtual void SetTwistAndSwing(float minT, float maxT, float maxS) {};
    virtual void SetStiffness(float stiffness, float damping) {}
    virtual void SetRestRotation(const Quaternion& restRot) {};

    virtual void SetTwistDirectionWorld(const Vector3& dir) {}

    RigidBody* GetBodyA() { return _bodyA; };
    RigidBody* GetBodyB() { return _bodyB; };
private:
    // 1体のボディを、アンカーが correction の向きへ動くように回す
    void RotateBodyToMoveAnchor(RigidBody* body, const Vector3& localAnchor, const Vector3& correction)
    {
        //各変数の説明はRagdollTuningのほうを参照してください
        constexpr float kAnchorRotationWeight = RagdollTuning::kAnchorRotationWeight;
        constexpr float kAnchorRotationMaxAngle = RagdollTuning::kAnchorRotationMaxAngle;
        constexpr float kAnchorRotationDeadband = RagdollTuning::kAnchorRotationDeadband;
        constexpr float kMinAnchorArmSqLength = RagdollTuning::kMinAnchorArmSqLength;
        constexpr float kMinRotationAxisLength = RagdollTuning::kMinRotationAxisLength;


        if (body->IsKinematic())
            return;

        // 極小のずれは無視する
        if (Vector::SqMagnitude(correction) < kAnchorRotationDeadband * kAnchorRotationDeadband)
            return;

        // 重心からアンカーまでのベクトル(ワールド座標)
        // 回転は、このベクトルの根元(重心)を支点にして起こる
        Vector3 anchorArm = body->GetRotation() * localAnchor;
        float anchorArmSqLength = Vector::SqMagnitude(anchorArm);
        if (anchorArmSqLength < kMinAnchorArmSqLength)
            return;


        // この軸まわりに、下の角度だけ回すと、アンカーは「ずれのうち腕に垂直な成分」だけ動く
        // (腕に沿った成分は、回転では動かせないので、ここでは扱わない)
        Vector3 rotationAxis = Vector::Cross(anchorArm, correction);
        float rotationAxisLength = rotationAxis.Length();
        if (rotationAxisLength < kMinRotationAxisLength)
            return;

        // 小さい角度で回したとき、アンカーが動く距離がちょうどずれの垂直成分になるように決めている
        float rotationAngle = rotationAxisLength / anchorArmSqLength;

        // 上限をかける
        if (rotationAngle > kAnchorRotationMaxAngle)
            rotationAngle = kAnchorRotationMaxAngle;

        // 一部だけ適用して、複数回の呼び出しで少しずつ収束させる
        rotationAngle *= kAnchorRotationWeight;

        Quaternion correctionRotation =
            QuaternionMath::CreateFromAxisAngle(rotationAxis / rotationAxisLength, rotationAngle);

        // 増分は左から掛ける
        Quaternion correctedRotation = correctionRotation * body->GetRotation();
        correctedRotation.Normalize();

        body->SetRotation(correctedRotation);
    }

protected:
    PhysicsSystem* _registry = nullptr;

    RigidBody* _bodyA = nullptr;
    RigidBody* _bodyB = nullptr;

    Vector3 _localAnchorA;
    Vector3 _localAnchorB;

    float _maxCorrection = 0.3f;
    float _beta = 0.3f;
};
