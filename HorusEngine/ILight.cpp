#include "ILight.h"

namespace GFX::Light
{
	void ILight::SetAttenuation(size_t range) noexcept
	{
		auto& buffer = lightBuffer->GetBuffer();
		buffer["atteuationLinear"] = 4.5f / range;
		buffer["attenuationQuad"] = 75.0f / (range * range);
	}

	ILight& ILight::operator=(ILight&& light) noexcept
	{
		this->JobData::operator=(std::forward<JobData&&>(light));
		volume = std::move(light.volume);
		mesh = std::move(light.mesh);
		lightBuffer = std::move(light.lightBuffer);
		return *this;
	}

	void ILight::Update(const DirectX::XMFLOAT3& delta, const DirectX::XMFLOAT3& deltaAngle) noexcept
	{
		mesh->Update(delta, deltaAngle);
		lightBuffer->GetBuffer()["lightPos"] = mesh->GetPos();
		volume->Update(lightBuffer->GetBufferConst());
	}

	void ILight::UpdatePos(const DirectX::XMFLOAT3& delta) noexcept
	{
		mesh->UpdatePos(delta);
		lightBuffer->GetBuffer()["lightPos"] = mesh->GetPos();
		volume->Update(lightBuffer->GetBufferConst());
	}

	void ILight::SetPos(const DirectX::XMFLOAT3& position) noexcept
	{
		mesh->SetPos(position);
		lightBuffer->GetBuffer()["lightPos"] = position;
		volume->Update(lightBuffer->GetBufferConst());
	}

	void ILight::Bind(Graphics& gfx)
	{
		lightBuffer->Bind(gfx);
		volume->Bind(gfx);
	}
}