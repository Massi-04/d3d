struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 TextCoord : TEXTCOORDS;
};

cbuffer constBufferLayout
{
	float4x4 WVP;
};

VS_OUTPUT main(float4 inPosition : POSITION, float2 inTextCoords : TEXTCOORDS)
{
	VS_OUTPUT res;
	res.Pos = mul(inPosition, WVP);
	res.TextCoord = inTextCoords;
	return res;
}