#include "common.hlsl"

Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

void main(in PS_IN In, out float4 outDiffuse : SV_Target)
{
    // --- ライティング計算 ---
    float3 lv = (In.WorldPosition - Light.Position).xyz;
    float ld = length(lv);
    lv = normalize(lv);

    // 減衰
    float ofs = 1.0f - (1.0f / Light.PointLightParam.x) * ld;
    ofs = max(0, ofs);

    // 法線
    float3 normal = normalize(In.Normal.xyz);

    // 拡散光
    float light = saturate(-dot(normal, lv));

    // --- セルシェーディング段階分け ---
    if (light >= 0.7f)
        light = 1.0f;
    else if (light >= 0.5f)
        light = 0.7f;
    else
        light = 0.4f;

    light *= ofs;

    // --- テクスチャ or Diffuse ---
    if (Material.TextureEnable)
    {
        outDiffuse = g_Texture.Sample(g_SamplerState, In.TexCoord);
        outDiffuse.rgb *= In.Diffuse.rgb * light + Light.Ambient.rgb;
    }
    else
    {
        outDiffuse = In.Diffuse;
        outDiffuse.rgb *= light + Light.Ambient.rgb;
    }

    // αはそのまま
    outDiffuse.a *= In.Diffuse.a;

    // --- 輪郭線風処理 ---
    float3 eyev = normalize((In.WorldPosition - CameraPosition).xyz);
    float d = dot(normal, eyev);
    if (d > -0.25f) // 角度が急なら輪郭
    {
        outDiffuse.rgb *= 0.3f;
    }
}
