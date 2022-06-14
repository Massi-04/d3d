struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 TextCoord : TEXTCOORDS;
};

Texture2D ObjTexture;
SamplerState ObjSamplerState;

float4 main(VS_OUTPUT roba) : SV_TARGET
{
	return ObjTexture.Sample(ObjSamplerState, roba.TextCoord);
}