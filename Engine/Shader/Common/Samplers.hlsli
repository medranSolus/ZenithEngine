/*	Sampler definitions for different types declared in code.

	Possible type categories:
	{
		P - Point
		L - Linear
		A - Anisotropic
	}
	{
		B - Exceedeing clamp to border color
		W - Wrap coordinates
		R - Coordinates reflection
	}
*/

SamplerState splr_AB : register(s0);
SamplerState splr_AW : register(s1);
SamplerState splr_AR : register(s2);

SamplerState splr_LW : register(s3);
SamplerState splr_LR : register(s4);

SamplerState splr_PW : register(s5);
SamplerState splr_PR : register(s6);