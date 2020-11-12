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
		SpotLight(Graphics& gfx, Pipeline::RenderGraph& graph, const std::string& name, float intensity,
			const Data::ColorFloat3& color, const DirectX::XMFLOAT3& position, size_t range, float size,
			float innerAngle, float outerAngle, const DirectX::XMFLOAT3& direction);
		inline SpotLight(SpotLight&& light) noexcept { *this = std::forward<SpotLight&&>(light); }
		inline SpotLight& operator=(SpotLight&& light) noexcept { this->ILight::operator=(std::forward<ILight&&>(light)); range = light.range;  return *this; }
		virtual ~SpotLight() = default;

		bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override;
	};
}