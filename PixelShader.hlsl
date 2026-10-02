struct LightData
{
    int Type;
    float3 Padding0;

    float3 Position;
    float Range;

    float3 Direction;
    float Angle;

    float4 Color;

};
struct MaterialData
{
    float4 Color;
    int Unlit;
    float3 Padding;
};

Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;

    LightData Light;
    MaterialData Material;
};
struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

float4 main(PS_IN input) : SV_TARGET
{
    //これは単位ベクトルを作って、内積で角度を出して１～０の範囲で収めている
    float3 N = normalize(input.normal);
    float3 L = normalize(-Light.Direction);
    
    float4 texColor = gTexture.Sample(gSampler, input.uv) * Material.Color;

    if (Material.Unlit != 0)
        return texColor;

    float diffuse = saturate(dot(N, L));
    return texColor * Light.Color * diffuse;

}