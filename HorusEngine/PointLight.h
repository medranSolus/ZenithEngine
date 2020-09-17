#pragma once
#include "BaseLight.h"
#include "ICamera.h"
#include "ConstBufferExCache.h"
#include "RenderGraph.h"

namespace GFX::Light
{
	class PointLight : public BaseLight
	{
		mutable std::shared_ptr<Resource::ConstBufferExPixelCache> lightBuffer = nullptr;

		static Data::CBuffer::DCBLayout MakeLayout() noexcept;

	public:
		PointLight(Graphics& gfx, Pipeline::RenderGraph& graph, const DirectX::XMFLOAT3& position, const std::string& name, float radius = 0.5f);
		PointLight(const PointLight&) = delete;
		PointLight& operator=(const PointLight&) = delete;
		virtual ~PointLight() = default;

		inline void Submit(uint64_t channelFilter) noexcept override { mesh->Submit(channelFilter); JobData::Submit(channelFilter); }

		void Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override;
		void Bind(Graphics& gfx) override;
	};
}