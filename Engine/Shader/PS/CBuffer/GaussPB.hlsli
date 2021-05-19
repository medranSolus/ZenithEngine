cbuffer GaussBuffer : register(b12)
{
	// Must not exceed coefficients size
	int cb_radius;
	uint cb_width;
	uint cb_height;
	float cb_intensity;
	// Should be 6 * sigma - 1, current sigma for best effect 1.3 (but with reduced render target can be 2.6)
	float cb_coefficients[8];
}