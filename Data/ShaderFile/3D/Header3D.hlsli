struct Skin
{
    float4 pos;
    float3 normal;
};

struct VS_IN
{
    float4 pos : POSITION0;
    float4 normal : NORMAL0;
    float2 tex : TEXCOORD0;
    uint4 clusterIndex : BONEINDEX;
    float4 weights : BONEWEIGHT;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float4 normal : NORMAL0;
    float2 tex : TEXCOORD0;
    float4 color : TEXCOORD1;
};