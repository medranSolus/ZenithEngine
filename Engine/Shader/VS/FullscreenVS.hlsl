struct VSOut
{
	float2 tc : TEXCOORD;
	float4 pos : SV_POSITION;
};

VSOut main(uint id : SV_VertexID)
{
	VSOut vso;
	/*
	* X: 0,1 -> 0,  2  -> 2
	* Y:  0  -> 2, 1,2 -> 0
	*/
	vso.tc = float2(id & 2, (((id | (id >> 1)) & 1) ^ 1) << 1);

	/*
	* X: 0,1 -> -1,  2  -> 3
	* Y:  0  -> -3, 1,2 -> 1
	*/
	const float2 offset = vso.tc * 2.0f;
	vso.pos = float4(-1.0f + offset.x, 1.0f - offset.y, 1.0f, 1.0f);

	return vso;
}