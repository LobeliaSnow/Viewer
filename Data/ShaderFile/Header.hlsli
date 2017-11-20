cbuffer View : register(b0)
{
    column_major float4x4 view;
    column_major float4x4 projection;
};

cbuffer World : register(b1)
{
    column_major float4x4 world;
};

cbuffer Material : register(b2)
{
    float4 diffuse;
    float4 ambient;
    float4 specular;
    float4 texColor;
}

cbuffer Bone : register(b3)
{
	//1meshï”ÇË255ñ{ÇÃÉ{Å[Éì
    column_major float4x4 keyFrames[256];
}

cbuffer Environment : register(b4)
{
    float4 lightDirection;
    float4 ambientColor;
    float4 fogInfo;
}

Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);
