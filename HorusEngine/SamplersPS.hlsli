/*	Sampler definitions for different types declared in code

	Possible type categories:
	{
		P - Point
		L - Linear
		A - Anisotropic
	}
	{
		N - No coordinates reflection
		R - Coordinates reflection
	}
*/

SamplerState splr_AN : register(s0);
SamplerState splr_AR : register(s1);

SamplerState splr_LN : register(s2);
SamplerState splr_LR : register(s3);

SamplerState splr_PN : register(s4);
SamplerState splr_PR : register(s5);