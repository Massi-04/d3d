struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLORE;
};

float4 main(VS_OUTPUT roba) : SV_TARGET
{
	return roba.Color;
}