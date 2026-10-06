#pragma once
#include <DirectXMath.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace DirectX;

class MatrixUtility
{
public:
	///<summary>
	///aiのMatrixをXMFLOAT4＊4に変換
	///</summary>
	///<param name="matrix"></param>
	///<returns></returns>
	static XMFLOAT4X4 ConvertMatrix(const aiMatrix4x4& matrix)
    {
        DirectX::XMFLOAT4X4 result;

        result._11 = matrix.a1;
        result._12 = matrix.b1;
        result._13 = matrix.c1;
        result._14 = matrix.d1;

        result._21 = matrix.a2;
        result._22 = matrix.b2;
        result._23 = matrix.c2;
        result._24 = matrix.d2;

        result._31 = matrix.a3;
        result._32 = matrix.b3;
        result._33 = matrix.c3;
        result._34 = matrix.d3;

        result._41 = matrix.a4;
        result._42 = matrix.b4;
        result._43 = matrix.c4;
        result._44 = matrix.d4;

        return result;
    }

 
    ///<summary>
    ///
    ///</summary>
    ///<typeparam name="T ReturnTransformの値を指定"></typeparam>
    ///<param name="matrix"></param>
    ///<returns></returns>
    
    static XMFLOAT3 GetPosition(const XMFLOAT4X4& matrix)
    {
        XMFLOAT3 result;

        XMMATRIX m = XMLoadFloat4x4(&matrix);

        XMVECTOR position, rotation, scale;
        XMMatrixDecompose(&scale, &rotation, &position, m);

        XMStoreFloat3(&result, position);

        return result;
    }
    static XMFLOAT4 GetRotation(const XMFLOAT4X4& matrix)
    {
        XMFLOAT4 result;

        XMMATRIX m = XMLoadFloat4x4(&matrix);

        XMVECTOR position, rotation, scale;
        XMMatrixDecompose(&scale, &rotation, &position, m);

        XMStoreFloat4(&result, rotation);

        return result;
    }
    enum ReturnTransform
    {
        Position,
        Rotation,
        Scale
    };
    template <ReturnTransform T>
    static auto GetTransform(const XMFLOAT4X4& matrix)
    {
        XMMATRIX m = XMLoadFloat4x4(&matrix);

        XMVECTOR position;
        XMVECTOR rotation;
        XMVECTOR scale;

        XMMatrixDecompose(
            &scale,
            &rotation,
            &position,
            m
        );

        if constexpr (T == ReturnTransform::Position)
        {
            XMFLOAT3 result;
            XMStoreFloat3(&result, position);
            return result;
        }
        else if constexpr (T == ReturnTransform::Scale)
        {
            XMFLOAT3 result;
            XMStoreFloat3(&result, scale);
            return result;
        }
        else if constexpr (T == ReturnTransform::Rotation)
        {
            XMFLOAT4 result;
            XMStoreFloat4(&result, rotation);
            return result;
        }
    }
};