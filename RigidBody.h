#pragma once
#include <DirectXMath.h>
#include <iostream>
#include <string>

#include "Component.h"
#include "VectorUtility.h"
#include "RagdollData.h"

using namespace DirectX;

constexpr const float EPSILON = 0.00001f;
constexpr const float GRAVITY = -9.8f;

class PhysicsSystem;

class RigidBody :public Component
{
public:
	void Initialize(ServiceLocator& loacator)override;

    // 登録済みのPhysicsSystemがあれば、自動的に登録を解除する(定義はRigidBody.cpp)
    ~RigidBody() override;

    // 登録先のPhysicsSystemを覚えておく(デストラクタで、自動的に登録解除するため)
    void SetRegistry(PhysicsSystem* system) { _registry = system; }
	void FixedUpdate(float deltaTime);

    // 並進運動の更新: 重力 → 減衰 → 速度制限 → 位置の積分 の順に行う
    // 速度を先に更新してから、その新しい速度で位置を動かす
    // 位置を先に動かす方式より、重力下でのエネルギーが増えにくく安定するため
	void UpdateGravity(float deltaTime);

    // ジョイントのピボットから見てずれた位置にある重心に、重力で回転力(トルク)をかける
    // ピボットを支点に、腕や脚が振り子のように垂れ下がる動きを出すために、回転側にも重力を反映する
    void ApplyGravityTorque(const Vector3& centerOfMassOffsetLocal, float deltaTime);

    // ワールド座標のオフセットを直接受け取る版の重力トルク
    // ラグドール全体の重心(ルートから見たずれ)に対して、全体の質量で重力トルクをかける用途を想定
    void ApplyGravityTorqueWorld(const Vector3& worldComOffset, float totalMass, float deltaTime);


    // 溜めたトルクを角速度に反映する
    // 他のシステム(ジョイントなど)からは Torque に足すだけにして、
    // 角加速度・減衰・速度制限はここだけで処理するようにする
    void ApplyTorque(float deltaTime);


    // 接触面に沿った(接線方向の)速度だけを減らす
    // 法線方向の速度は、押し出しや反発の処理が扱うのでここでは触らない
    void ApplyFriction(const Vector3& normal, float frictionDecayPerSecond, float deltaTime);


    // 角速度を、Rotationに反映する
    // 角速度ベクトルは「向き = 回転軸、長さ = 1秒あたりの回転角」を表すので、
    // 1フレーム分の回転を 軸+角度 から作ってクォータニオンとして掛ける
    void IntegrateRotation(float deltaTime);


    // 法線方向の速度のうち、面にめり込む向きの成分だけを取り除く
    // 押し出し後に、面へ向かう速度が残ってまためり込むのを防ぐのが目的
    void RemoveVelocityAlongNormal(const Vector3& normal);


    //ねじれ追加関数
    void AddTorque(const Vector3& torque) { Torque += torque; };

    //瞬間的に力を角速度に加える関数
    void AddAngularImpulse(Vector3& angularImpulse) { AngularVelocity += angularImpulse / Inertia; }

    //めり込み時の速度追加関数
    void AddPseudoVelocity(const Vector3& velocity) { PseudoVelocity += velocity; }



    //ここからはゲッターとセッター

    //地面判定フラグ設定関数
    void SetGrounded(bool grounded) {isGrounded = grounded; }
    bool IsGrounded() const { return isGrounded; }

    //重力使用フラグ設定関数
	void SetIsGravity(bool useGravity) { isGravity = useGravity; }
	bool GetUseGravity()const { return isGravity; }

    // 重力トルク(重心が関節からずれているぶん、重い側を下に向ける力)を使うかの設定
    // 自由落下している間は、重力は回転を生まないので、falseにする
    void SetUseGravityTorque(bool use) { useGravityTorque = use; }
    bool GetUseGravityTorque() const { return useGravityTorque; }

    //回転と場所設定関数
    void SetPosition(const Vector3& position) { Position = position; }
    void SetRotation(const Quaternion& rotation) { Rotation = rotation; }
    Vector3 GetPosition() { return Position; }
    const Vector3& GetPosition() const { return Position; }
    const Quaternion& GetRotation()const { return Rotation; }

    //角速度設定関数
    void SetAngularVelocity(const Vector3& angle) { AngularVelocity = angle; }
    const Vector3& GetAngularVelocity() const { return AngularVelocity; }
    Vector3 GetAngularVelocity() { return AngularVelocity; }

    //物理演算反映設定関数
    void DisableKinematic() { isKinematic = false; }
    void EnableKinematic() { isKinematic = true; }
    bool IsKinematic() { return isKinematic; }

    //質量設定関数
    void SetMass(float mass) { _mass = mass; }
    const float& GetMass()const { return _mass; }

    //慣性モーメント設定関数(回転のしにくさ)
    void SetInertia(float inertia) { Inertia = inertia; }
    float GetInertia() const { return Inertia; }

    //物体の運動値設定関数
    void SetVelocity(const Vector3& velocity) { Velocity = velocity; }
    const Vector3& GetVelocity() const { return Velocity; }
    Vector3& GetVelocity() { return Velocity; }

    //めり込み時の速度取得関数
    Vector3 GetPseudoVelocity() const { return PseudoVelocity; }

    //ラグドール時の物理演算無効設定関数
    bool IsSleeping() const { return _isSleeping; }
    void SetSleeping(bool sleeping)
    {
        _isSleeping = sleeping;
        _sleepTimer = 0.0f;
        if (sleeping)
        {
            Velocity = Vector3(0.0f, 0.0f, 0.0f);
            AngularVelocity = Vector3(0.0f, 0.0f, 0.0f);
        }
    }


    void ClearTorque() { Torque = Vector3(0.0f, 0.0f, 0.0f); }
    void ClearPseudoVelocity() { PseudoVelocity = Vector3(0.0f, 0.0f, 0.0f); }
private:
    Vector3 Scale{};
    Quaternion Rotation{};
	Vector3 Position{};

	Vector3 Velocity{};
    Vector3 PseudoVelocity{};
    Vector3 AngularVelocity{};
    Vector3 Torque{};

	float _mass =1.0f;//0だと衝突の計算で1/質量が無限大になりNaNになるので、既定値は1
    float forcePower = 5.0f;
    float Inertia = 1.0f;

    bool isGrounded = false;
	bool isGravity = false;
    bool useGravityTorque = true;
	bool isKinematic = false;


    bool _isSleeping = false;
    float _sleepTimer = 0.0f;

    PhysicsSystem* _registry = nullptr;
};
