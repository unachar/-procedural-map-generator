#include	"common.hlsl"
Texture2D g_Texture : register(t0); //テクスチャ０番
SamplerState g_SamplerState : register(s0); //サンプラー０番

void main(in PS_IN In, out float4 outDiffuse : SV_Target)
{
	//光源からピクセルへのベクトル
    float4 lv = In.WorldPosition - Light.Position;
	//物体と光源の距離
    float4 ld = length(lv);
	//ベクトルの正規化
    lv = normalize(lv);

	//減衰の計算
    float ofs = 1.0f - (1.0f / Light.PointLightParam.x) * ld; //減衰の計算
	//減衰率0未満は0にする。
    ofs = max(0, ofs);
    
   	//ピクセルの法線を正規化
    float4 normal = normalize(In.Normal);
	//光源計算
    float light = -dot(normal.xyz, lv.xyz);
    light = saturate(light);

 	//light <= 明るさの調整（段階分け）
    if (light >= 0.7f)
    {
        light = 1.0f;
    }
    else if (light >= 0.5f)
    {
        light = 0.7f;
    }
    else
    {
        light = 0.4f;
    }

    light *= ofs; //明るさを減衰させる
   
 
	//テクスチャのピクセル色を取得
    outDiffuse = g_Texture.Sample(g_SamplerState, In.TexCoord);
    outDiffuse.rgb *= In.Diffuse.rgb * light + Light.Ambient.rgb; //明るさを乗算
    outDiffuse.a *= In.Diffuse.a; //α値に明るさは関係ない
  
    //視線ベクトル
    float4 eyev = In.WorldPosition - CameraPosition;
    eyev = normalize(eyev);

    float d = dot(normal, eyev); // 視線とピクセル法線の内積
    if (d > -0.25f)
    {
        outDiffuse.rgb *= 0.3f;
    }
   
    
    
}





