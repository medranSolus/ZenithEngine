#pragma once
#include "ILight.h"
#include "ConstBufferExCache.h"
#include "RenderGraph.h"

namespace GFX::Light
{
	class PointLight : public ILight
	{
		size_t range;

		static inline Data::CBuffer::DCBLayout MakeLayout() noexcept;

	public:
		PointLight(Graphics& gfx, Pipeline::RenderGraph& graph, const DirectX::XMFLOAT3& position, const std::string& name,
			size_t range, float intensity = 1.0f, const Data::ColorFloat3& color = { 1.0f, 1.0f, 1.0f }, float radius = 0.5f);
		inline PointLight(PointLight&& light) noexcept { *this = std::forward<PointLight&&>(light); }
		inline PointLight& operator=(PointLight&& light) noexcept { this->ILight::operator=(std::forward<ILight&&>(light)); range = light.range;  return *this; }
		virtual ~PointLight() = default;

		bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override;
	};
}