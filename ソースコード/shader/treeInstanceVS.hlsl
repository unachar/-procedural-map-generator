#include "common.hlsl"

struct VS_TREE_IN
{
    float4 Position : POSITION0;
    float4 Normal : NORMAL0;
    float4 Diffuse : COLOR0;
    float2 TexCoord : TEXCOORD0;

    float4x4 InstanceWorld : WORLD;
    uint InstanceID : SV_InstanceID;
};


PS_IN main(VS_TREE_IN In)
{
    PS_IN Out;

    // インスタンス行列（C++側でChunk行列と合成済み）を適用
    float4 worldPos = mul(In.Position, In.InstanceWorld);
    Out.WorldPosition = worldPos;

    float4 viewPos = mul(worldPos, View);
    Out.Position = mul(viewPos, Projection);
    
    // 法線変換（平行移動を除外するため w=0）
    float4 worldNormal = mul(float4(In.Normal.xyz, 0.0), In.InstanceWorld);
    Out.Normal = normalize(worldNormal);
    
    Out.TexCoord = In.TexCoord;
    
    // 色を濃くするためにマテリアル色を強調（必要に応じて係数を調整）
    Out.Diffuse = Material.Diffuse * 1.4f;

    return Out;
}
