#include "common.hlsl"

Texture2D g_Texture : register(t0);
Texture2D g_TextureDirt : register(t1);
SamplerState g_SamplerState : register(s0);

void main(in PS_IN In, out float4 outDiffuse : SV_Target)
{
    if (Material.TextureEnable)
    {
        outDiffuse = g_Texture.Sample(g_SamplerState, In.TexCoord) * In.Diffuse.a;
        outDiffuse += g_TextureDirt.Sample(g_SamplerState, In.TexCoord) * (1.f - In.Diffuse.a);
        
        
        outDiffuse *= In.Diffuse; 
    }
    else
    {
        outDiffuse = In.Diffuse;
    }
    
    
    
    outDiffuse.a = 1;
}