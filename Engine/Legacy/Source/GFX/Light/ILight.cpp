#include "GFX/Light/ILight.h"

namespace ZE::GFX::Light
{
	void ILight::SetAttenuation(U64 range) noexcept
	{
		auto& buffer = lightBuffer->GetBuffer();
		buffer["atteuationLinear"] = 4.5f / range;
		buffer["attenuationQuad"] = 75.0f / (range * range);
	}

	void ILight::Update(const Float3& delta, const Vector& rotor) noexcept
	{
		mesh->Update(delta, rotor);
		lightBuffer->GetBuffer()["lightPos"] = mesh->GetPos();
		volume->Update(lightBuffer->GetBufferConst());
	}

	void ILight::UpdatePos(const Float3& delta) noexcept
	{
		mesh->UpdatePos(delta);
		lightBuffer->GetBuffer()["lightPos"] = mesh->GetPos();
		volume->Update(lightBuffer->GetBufferConst());
	}

	void ILight::SetPos(const Float3& position) noexcept
	{
		mesh->SetPos(position);
		lightBuffer->GetBuffer()["lightPos"] = position;
		volume->Update(lightBuffer->GetBufferConst());
	}

	void ILight::Bind(Graphics& gfx) const noexcept
	{
		lightBuffer->Bind(gfx);
		volume->Bind(gfx);
	}
}