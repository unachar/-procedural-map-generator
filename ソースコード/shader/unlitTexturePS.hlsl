
#include "common.hlsl"


Texture2D		g_Texture : register(t0);
SamplerState	g_SamplerState : register(s0);


void main(in PS_IN In, out float4 outDiffuse : SV_Target)
{

	if (Material.TextureEnable)
	{
		outDiffuse = g_Texture.Sample(g_SamplerState, In.TexCoord);
		
        clip(In.Diffuse.a - 0.01);
		outDiffuse *= In.Diffuse * 1.3;
	}
	else
	{
		outDiffuse = In.Diffuse * 1.3;
	}


}
