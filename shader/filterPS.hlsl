#include "common.hlsl"

Texture2D    g_Texture      : register(t0);
SamplerState g_SamplerState : register(s0);

void main(in PS_IN In, out float4 outDiffuse : SV_Target)
{
    float2 texel = 1 / 500;

    float3 color = g_Texture.Sample(g_SamplerState, In.TexCoord).rgb;
    float luminance = dot(color, float3(0.299, 0.587, 0.114));

    float threshold = 0.75f; // Ç±ÇÃílà»è„ÇÃñæÇÈÇ≥ÇÉuÉãÅ[ÉÄëŒè€Ç…Ç∑ÇÈ
    float bloomAmount = luminance - threshold;
    float3 brightColor = color * bloomAmount / luminance;

    float3 bloom = 0.0f;

    float2 offsets[9] =
    {
        float2(-texel.x, -texel.y),
        float2(0.0f, -texel.y),
        float2(texel.x, -texel.y),

        float2(-texel.x, 0.0f),
        float2(0.0f, 0.0f),
        float2(texel.x, 0.0f),

        float2(-texel.x, texel.y),
        float2(0.0f, texel.y),
        float2(texel.x, texel.y)
    };

    float weights[9] =
    {
        1.0f, 2.0f, 1.0f,
        2.0f, 4.0f, 2.0f,
        1.0f, 2.0f, 1.0f
    };

    float weightSum = 0.0f;


    for (int i = 0; i < 9; ++i)
    {
        float3 s = g_Texture.Sample(g_SamplerState, In.TexCoord + offsets[i]).rgb;
        float lum = dot(s, float3(0.299, 0.587, 0.114));
        float ba = max(lum - threshold, 0.0f);

        float w = weights[i] * ba;
        bloom += s * w;
        weightSum += w;
    }

    if (weightSum > 0.0f)
    {
        bloom /= weightSum;
    }

    outDiffuse.rgb = color + bloom;
    outDiffuse.a = 1.0f;
}