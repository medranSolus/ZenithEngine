struct VSOut
{
	float2 tc : TEXCOORD;
	float4 pos : SV_POSITION;
};

VSOut main(float2 pos : POSITION)
{
	VSOut vso;
	vso.pos = float4(pos, 0.0f, 1.0f);
	vso.tc = float2((pos.x + 1.0f) / 2.0f, (pos.y - 1.0f) / -2.0f);
	return vso;
}