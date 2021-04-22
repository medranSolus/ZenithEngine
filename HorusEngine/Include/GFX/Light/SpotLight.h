#pragma once
#include "ILight.h"
#include "GFX/Resource/ConstBufferExCache.h"
#include "GFX/Pipeline/RenderGraph.h"

namespace GFX::Light
{
	class SpotLight : public ILight
	{
		U64 range;

		static Data::CBuffer::DCBLayout MakeLayout() noexcept;

	public:
		SpotLight(Graphics& gfx, Pipeline::RenderGraph& graph, std::string&& name, float intensity,
			const ColorF3& color, const Float3& position, U64 range, float size,
			float innerAngle, float outerAngle, const Float3& direction);
		SpotLight(SpotLight&&) = default;
		SpotLight& operator=(SpotLight&&) = default;
		virtual ~SpotLight() = default;

		bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override;
	};
}