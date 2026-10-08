struct LightData
{
    int Type;
    float3 Padding0; // 16バイトアライメント調整用のパディング

    float3 Position;
    float Range;

    float3 Direction;
    float Angle;

    float4 Color;
};

// マテリアル情報
struct MaterialData
{
    float4 Color;
    int Unlit;
    float3 Padding;
};

Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

// スキニング用のボーン行列配列(CPU側でFinalTransformを書き込んだもの)
StructuredBuffer<float4x4> BoneMatrices : register(t1);

// 毎フレーム/毎オブジェクトで更新される定数バッファ
cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
    
    LightData Light;
    MaterialData Material;
};

struct VS_IN
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;

    uint4 BoneIndex : BLENDINDICES; // 影響を受ける最大4本のボーンのインデックス
    float4 Weight : BLENDWEIGHT; // 各ボーンの影響度(合計は概ね1になるよう正規化済み)
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    
    uint4 BoneIndex : BLENDINDICES;
    float4 Weight : BLENDWEIGHT;
};

VS_OUT main(VS_IN input)
{
    VS_OUT output;

    // 4本のBoneでスキニング
    float4 pos = float4(input.pos, 1.0f);
    float3 normal = input.normal;

    float totalWeight =
    input.Weight.x +
    input.Weight.y +
    input.Weight.z +
    input.Weight.w;

    // ウェイトが設定されている頂点だけスキニングを行う
    // (ボーンに紐付かない頂点は元のローカル座標をそのまま使う)
    if (totalWeight > 0.0f)
    {
        float4 skinnedPosition = 0.0f;
        float3 skinnedNormal = 0.0f;

        for (int i = 0; i < 4; ++i)
        {
            const float weight = input.Weight[i];

            // ウェイト0のスロットは計算する意味が無いのでスキップ
            if (weight <= 0.0f)
                continue;

            // 各ボーン行列で変換した位置・法線を、ウェイトで加重合成する
            skinnedPosition +=
            mul(pos, BoneMatrices[input.BoneIndex[i]]) * weight;

            // 法線は平行移動の影響を受けないよう3x3部分だけで変換する
            skinnedNormal +=
            mul(
                input.normal,
                (float3x3) BoneMatrices[input.BoneIndex[i]]
            ) * weight;
        }

        pos = skinnedPosition;
        normal = normalize(skinnedNormal);
    }

    // ローカル→ワールド→ビュー→プロジェクション空間へ変換
    pos = mul(pos, World);
    pos = mul(pos, View);
    pos = mul(pos, Projection);

    output.pos = pos;

    // 法線はワールド行列の回転成分だけで変換(平行移動を除去)
    output.normal = normalize(
    mul(normal, (float3x3) World));

    output.uv = input.uv;
    output.BoneIndex = input.BoneIndex;
    output.Weight = input.Weight;
    
    return output;
}