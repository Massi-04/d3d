struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float4 Color : COLORE;
};

cbuffer constBufferLayout
{
	float4x4 WVP;
};

VS_OUTPUT main(float4 inPosition : POSITION, float4 inColor : COLORE)
{
	VS_OUTPUT res;
	res.Pos = mul(inPosition, WVP);
	res.Color = inColor;
	return res;
}