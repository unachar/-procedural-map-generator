#include "common.hlsl"

Texture2D    g_Texture      : register(t0);
SamplerState g_SamplerState : register(s0);

void main(in PS_IN In, out float4 outDiffuse : SV_Target)
{
    float offset = 1.0 / 500.0;
    
    float4 colorX0 = g_Texture.Sample(g_SamplerState, In.TexCoord + float2(-offset, 0.0));
    float4 colorX1 = g_Texture.Sample(g_SamplerState, In.TexCoord + float2(offset, 0.0));
    float4 colorY0 = g_Texture.Sample(g_SamplerState, In.TexCoord + float2(0.0, -offset));
    float4 colorY1 = g_Texture.Sample(g_SamplerState, In.TexCoord + float2(0.0, offset));
    float4 color = g_Texture.Sample(g_SamplerState, In.TexCoord);
    
    
    float4 colorDXY = (colorX0 + colorX1 + colorY0 + colorY1) - 4.0 * color;
    
    outDiffuse = color - abs(colorDXY) * 3.0;
    outDiffuse.a = 1.0;
}