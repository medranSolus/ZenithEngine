#include "IVolume.h"

namespace GFX::Light::Volume
{
	float IVolume::GetVolume(const Data::CBuffer::DynamicCBuffer& lightBuffer)
	{
		const Data::ColorFloat3& color = lightBuffer["lightColor"];
		const float lightMax = static_cast<float>(lightBuffer["lightIntensity"]) * fmaxf(fmaxf(color.col.x, color.col.y), color.col.z);

		const float linear = lightBuffer["atteuationLinear"];
		const float quad = lightBuffer["attenuationQuad"];
		return (-linear + sqrtf(linear * linear - 4.0f * quad * (static_cast<float>(lightBuffer["atteuationConst"]) - lightMax * 256.0f / 5.0f))) / (2.0f * quad);
	}

	IVolume& IVolume::operator=(IVolume&& volume) noexcept
	{
		transformBuffer = std::move(volume.transformBuffer);
		transformBuffer->ChangeOwner(*this);
		indexBuffer = std::move(volume.indexBuffer);
		vertexBuffer = std::move(volume.vertexBuffer);
		pixelShader = std::move(volume.pixelShader);
		return *this;
	}

	void IVolume::Bind(Graphics& gfx, const Data::CBuffer::DynamicCBuffer& lightBuffer)
	{
		const float scale = GetVolume(lightBuffer);
		DirectX::XMStoreFloat4x4(transform.get(),
			DirectX::XMMatrixScaling(scale, scale, scale) *
			DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&lightBuffer["lightPos"])));

		transformBuffer->Bind(gfx);
		indexBuffer->Bind(gfx);
		vertexBuffer->Bind(gfx);
		pixelShader->Bind(gfx);
	}
}