/*	Sampler definitions for different types declared in code.

	Possible type categories:
	{
		P - Point
		L - Linear
		A - Anisotropic
	}
	{
		E - Exceedeing clamp to edge color
		R - Repeat coordinates after edge
		M - Coordinates mirror at the edge
	}
*/

SamplerState splr_AE : register(s0);
SamplerState splr_AR : register(s1);
SamplerState splr_AM : register(s2);

SamplerState splr_LE : register(s3);
SamplerState splr_LR : register(s4);
SamplerState splr_LM : register(s5);

SamplerState splr_PE : register(s6);
SamplerState splr_PR : register(s7);
SamplerState splr_PM : register(s8);