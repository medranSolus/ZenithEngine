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
	vso.tc = uint2(id & 2, ~((id & 1) ^ ((id >> 1) & 1)) << 1);
	/*
	* X: 0,1 -> -1,  2  -> 3
	* Y:  0  -> -3, 1,2 -> 1
	*/
	const uint offset = vso.tc.x * 2;
	vso.pos = float4(-1 + offset, -3 + offset, 0.0f, 1.0f);
	return vso;
}