#include "common.hlsl"

struct VS_ROOM_IN
{
    float4 Position : POSITION0;
    float4 Normal : NORMAL0;
    float4 Diffuse : COLOR0;
    float2 TexCoord : TEXCOORD0;

    float4x4 InstanceWorld : WORLD;
    uint InstanceID : SV_InstanceID;
};


PS_IN main(VS_ROOM_IN In)
{
    PS_IN Out;

    float4 worldPos = mul(In.Position, In.InstanceWorld);
    worldPos = mul(worldPos, World);
    Out.WorldPosition = worldPos;
    

    float4 viewPos = mul(worldPos, View);
    Out.Position = mul(viewPos, Projection);
    
    
    Out.Normal = In.Normal;
    Out.TexCoord = In.TexCoord;
    Out.Diffuse = Material.Diffuse;

    return Out;
}