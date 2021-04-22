#pragma once
#include "ILight.h"
#include "GFX/Resource/ConstBufferExCache.h"
#include "GFX/Pipeline/RenderGraph.h"

namespace GFX::Light
{
	class PointLight : public ILight
	{
		U64 range;

		static Data::CBuffer::DCBLayout MakeLayout() noexcept;

	public:
		PointLight(Graphics& gfx, Pipeline::RenderGraph& graph, std::string&& name, float intensity,
			const ColorF3& color, const Float3& position, U64 range, float radius = 0.5f);
		PointLight(PointLight&&) = default;
		PointLight& operator=(PointLight&&) = default;
		virtual ~PointLight() = default;

		bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override;
	};
}