#include "GFX/Light/Volume/IVolume.h"

namespace GFX::Light::Volume
{
	float IVolume::GetVolume(const Data::CBuffer::DynamicCBuffer& lightBuffer) noexcept
	{
		const ColorF3& color = lightBuffer["lightColor"];
		const float lightMax = static_cast<float>(lightBuffer["lightIntensity"]) * fmaxf(fmaxf(color.RGB.x, color.RGB.y), color.RGB.z);

		const float linear = lightBuffer["atteuationLinear"];
		const float quad = lightBuffer["attenuationQuad"];
		return (-linear + sqrtf(linear * linear - 4.0f * quad * (1.0f - lightMax * 256.0f))) / (2.0f * quad);
	}

	void IVolume::Update(const Data::CBuffer::DynamicCBuffer& lightBuffer) noexcept
	{
		UpdateTransform(GetVolume(lightBuffer), lightBuffer);
	}

	void IVolume::Bind(Graphics& gfx) const noexcept
	{
		transformBuffer->Bind(gfx);
		indexBuffer->Bind(gfx);
		vertexBuffer->Bind(gfx);
	}
}