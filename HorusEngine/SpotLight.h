#pragma once
#include "ILight.h"
#include "ConstBufferExCache.h"
#include "RenderGraph.h"

namespace GFX::Light
{
	class SpotLight : public ILight
	{
		size_t range;

		static inline Data::CBuffer::DCBLayout MakeLayout() noexcept;

	public:
		SpotLight(Graphics& gfx, Pipeline::RenderGraph& graph, const DirectX::XMFLOAT3& position, const std::string& name, size_t range,
			float innerAngle, float outerAngle, const DirectX::XMFLOAT3& direction = { 0.0f, -1.0f, 0.0f }, float intensity = 1.0f,
			const Data::ColorFloat3& color = { 1.0f, 1.0f, 1.0f }, float size = 1.0f);
		inline SpotLight(SpotLight&& light) noexcept { *this = std::forward<SpotLight&&>(light); }
		inline SpotLight& operator=(SpotLight&& light) noexcept { this->ILight::operator=(std::forward<ILight&&>(light)); range = light.range;  return *this; }
		virtual ~SpotLight() = default;

		bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override;
	};
}