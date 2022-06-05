struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float4 Color : COLORE;
};

VS_OUTPUT main(float4 inPosition : POSITION, float4 inColor : COLORE)
{
	VS_OUTPUT res;
	res.Pos = inPosition;
	res.Color = inColor;
	return res;
}