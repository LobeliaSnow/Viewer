#include "../Header.hlsli"
#include "Header2D.hlsli"

cbuffer RasterScrollInfo : register(b5)
{
    float4 info : packoffset(c0);
    float2 timer : packoffset(c1);
}

float4 RasterScroll(PS_IN_TEX ps_in) : SV_Target
{
    float2 calcPos;
    calcPos.x = ps_in.tex.x + sin(ps_in.tex.y * 3.14 * info.x + timer.x) * info.y;
    calcPos.y = ps_in.tex.y + sin(ps_in.tex.x * 3.14 * info.z + timer.y) * info.w;
    return txDiffuse.Sample(samLinear, calcPos) * ps_in.col;
}

cbuffer GaussianFilterInfo : register(b6)
{
    float weight0 : packoffset(c0.x);
    float weight1 : packoffset(c0.y);
    float weight2 : packoffset(c0.z);
    float weight3 : packoffset(c0.w);
    float weight4 : packoffset(c1.x);
    float weight5 : packoffset(c1.y);
    float weight6 : packoffset(c1.z);
    float weight7 : packoffset(c1.w);
	//サーフェスのサイズ
    float width : packoffset(c2.x);
    float height : packoffset(c2.y);
    float offset_x : packoffset(c2.z);
    float offset_y : packoffset(c2.w);
}

struct VS_OUT_GAUSSIAN
{
    float4 pos : SV_POSITION;
    float2 texel0 : TEXCOORD0; // テクセル
    float2 texel1 : TEXCOORD1; // テクセル
    float2 texel2 : TEXCOORD2; // テクセル
    float2 texel3 : TEXCOORD3; // テクセル
    float2 texel4 : TEXCOORD4; // テクセル
    float2 texel5 : TEXCOORD5; // テクセル
    float2 texel6 : TEXCOORD6; // テクセル
    float2 texel7 : TEXCOORD7; // テクセル
    float4 color : TEXCOORD8;
};

VS_OUT_GAUSSIAN GaussianFilterXVS(VS_IN_TEX vs_in)
{
    VS_OUT_GAUSSIAN vs_out = (VS_OUT_GAUSSIAN) 0;
    vs_out.pos = float4(vs_in.pos.xyz, 1.0f);
    vs_out.texel0 = vs_in.tex + float2(-1.0f / width, 0.0f);
    vs_out.texel1 = vs_in.tex + float2(-3.0f / width, 0.0f);
    vs_out.texel2 = vs_in.tex + float2(-5.0f / width, 0.0f);
    vs_out.texel3 = vs_in.tex + float2(-7.0f / width, 0.0f);
    vs_out.texel4 = vs_in.tex + float2(-9.0f / width, 0.0f);
    vs_out.texel5 = vs_in.tex + float2(-11.0f / width, 0.0f);
    vs_out.texel6 = vs_in.tex + float2(-13.0f / width, 0.0f);
    vs_out.texel7 = vs_in.tex + float2(-15.0f / width, 0.0f);
    vs_out.color = vs_in.col;
    return vs_out;
}
VS_OUT_GAUSSIAN GaussianFilterYVS(VS_IN_TEX vs_in)
{
    VS_OUT_GAUSSIAN vs_out = (VS_OUT_GAUSSIAN) 0;
    vs_out.pos = float4(vs_in.pos.xyz, 1.0f);
    vs_out.texel0 = vs_in.tex + float2(0.0f, -1.0f / height);
    vs_out.texel1 = vs_in.tex + float2(0.0f, -3.0f / height);
    vs_out.texel2 = vs_in.tex + float2(0.0f, -5.0f / height);
    vs_out.texel3 = vs_in.tex + float2(0.0f, -7.0f / height);
    vs_out.texel4 = vs_in.tex + float2(0.0f, -9.0f / height);
    vs_out.texel5 = vs_in.tex + float2(0.0f, -11.0f / height);
    vs_out.texel6 = vs_in.tex + float2(0.0f, -13.0f / height);
    vs_out.texel7 = vs_in.tex + float2(0.0f, -15.0f / height);
    vs_out.color = vs_in.col;
    return vs_out;
}

typedef VS_OUT_GAUSSIAN PS_IN;

float4 GaussianFilterXPS(PS_IN ps_in) : SV_Target
{
    float4 color = 0.0f;
    color += (txDiffuse.Sample(samLinear, ps_in.texel0) + txDiffuse.Sample(samLinear, float2(ps_in.texel7.x + offset_x, ps_in.texel7.y))) * weight0;
    color += (txDiffuse.Sample(samLinear, ps_in.texel1) + txDiffuse.Sample(samLinear, float2(ps_in.texel6.x + offset_x, ps_in.texel6.y))) * weight1;
    color += (txDiffuse.Sample(samLinear, ps_in.texel2) + txDiffuse.Sample(samLinear, float2(ps_in.texel5.x + offset_x, ps_in.texel5.y))) * weight2;
    color += (txDiffuse.Sample(samLinear, ps_in.texel3) + txDiffuse.Sample(samLinear, float2(ps_in.texel4.x + offset_x, ps_in.texel4.y))) * weight3;
    color += (txDiffuse.Sample(samLinear, ps_in.texel4) + txDiffuse.Sample(samLinear, float2(ps_in.texel3.x + offset_x, ps_in.texel3.y))) * weight4;
    color += (txDiffuse.Sample(samLinear, ps_in.texel5) + txDiffuse.Sample(samLinear, float2(ps_in.texel2.x + offset_x, ps_in.texel2.y))) * weight5;
    color += (txDiffuse.Sample(samLinear, ps_in.texel6) + txDiffuse.Sample(samLinear, float2(ps_in.texel1.x + offset_x, ps_in.texel1.y))) * weight6;
    color += (txDiffuse.Sample(samLinear, ps_in.texel7) + txDiffuse.Sample(samLinear, float2(ps_in.texel0.x + offset_x, ps_in.texel0.y))) * weight7;
    return color * ps_in.color;
}

float4 GaussianFilterYPS(PS_IN ps_in) : SV_Target
{
    float4 color = 0.0f;
    color += (txDiffuse.Sample(samLinear, ps_in.texel0) + txDiffuse.Sample(samLinear, float2(ps_in.texel7.x, ps_in.texel7.y + offset_y))) * weight0;
    color += (txDiffuse.Sample(samLinear, ps_in.texel1) + txDiffuse.Sample(samLinear, float2(ps_in.texel6.x, ps_in.texel6.y + offset_y))) * weight1;
    color += (txDiffuse.Sample(samLinear, ps_in.texel2) + txDiffuse.Sample(samLinear, float2(ps_in.texel5.x, ps_in.texel5.y + offset_y))) * weight2;
    color += (txDiffuse.Sample(samLinear, ps_in.texel3) + txDiffuse.Sample(samLinear, float2(ps_in.texel4.x, ps_in.texel4.y + offset_y))) * weight3;
    color += (txDiffuse.Sample(samLinear, ps_in.texel4) + txDiffuse.Sample(samLinear, float2(ps_in.texel3.x, ps_in.texel3.y + offset_y))) * weight4;
    color += (txDiffuse.Sample(samLinear, ps_in.texel5) + txDiffuse.Sample(samLinear, float2(ps_in.texel2.x, ps_in.texel2.y + offset_y))) * weight5;
    color += (txDiffuse.Sample(samLinear, ps_in.texel6) + txDiffuse.Sample(samLinear, float2(ps_in.texel1.x, ps_in.texel1.y + offset_y))) * weight6;
    color += (txDiffuse.Sample(samLinear, ps_in.texel7) + txDiffuse.Sample(samLinear, float2(ps_in.texel0.x, ps_in.texel0.y + offset_y))) * weight7;
    return color * ps_in.color;
}

cbuffer Mosaic : register(b7)
{
    float1 grids : packoffset(c0);
};

float4 Mosaic(PS_IN_TEX ps_in):SV_Target
{
    float2 ms_uv = floor(ps_in.tex * grids + 0.5f) / grids;
    return txDiffuse.Sample(samLinear, ms_uv) * ps_in.col;
}