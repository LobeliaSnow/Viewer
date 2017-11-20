#include "../Header.hlsli"
#include "Header3D.hlsli"

//ïΩçsåıåπ
inline float DirectionalLight(float4 arg_nor, float3 light_dir, float4x4 world)
{
    float3 normal = mul((float3) arg_nor, (float3x3) world);
    normal = normalize(normal);
    return max(dot(normal, light_dir), 0.0f) * 0.5f + 0.3f;
}

matrix FetchBoneMatrix(uint iBone)
{
    return keyFrames[iBone];
}

Skin SkinVertex(VS_IN vs_in)
{
    Skin output = (Skin) 0;
	
    float4 pos = vs_in.pos;
    float3 normal = vs_in.normal;
    for (int i = 0; i < 4; i++)
    {
        uint bone = vs_in.clusterIndex[i];
        float weight = vs_in.weights[i];
        matrix m = FetchBoneMatrix(bone);
        output.pos += weight * mul(pos, m);
        output.normal += weight * mul(normal, (float3x3) m);
    }
    return output;

}

PS_IN Main3D(VS_IN vs_in)
{
    PS_IN ps_out = (PS_IN) 0;
    Skin skinned = SkinVertex(vs_in);
    skinned.pos.x *= -1;
    skinned.normal.x *= -1;
    ps_out.pos = mul(skinned.pos, world);
    ps_out.pos = mul(ps_out.pos, view);
    ps_out.pos = mul(ps_out.pos, projection);
    ps_out.normal = float4(skinned.normal, 1.0f);
    ps_out.normal = float4(mul((float3) ps_out.normal, (float3x3) world), 1.0f);
    ps_out.tex = vs_in.tex;
    ps_out.color = DirectionalLight(vs_in.normal, (float3) lightDirection, world);
    ps_out.color.a = 1.0f;
    return ps_out;
}

PS_IN Main3DNoSkin(VS_IN vs_in)
{
    PS_IN ps_out = (PS_IN) 0;
    vs_in.pos.x *= -1;
    vs_in.normal.x *= -1;
    ps_out.pos = mul(vs_in.pos, world);
    ps_out.pos = mul(ps_out.pos, view);
    ps_out.pos = mul(ps_out.pos, projection);
    ps_out.normal = vs_in.normal;
    ps_out.tex = vs_in.tex;
    ps_out.color = DirectionalLight(vs_in.normal, (float3) lightDirection, world);
    ps_out.color.a = 1.0f;

    return ps_out;
}
