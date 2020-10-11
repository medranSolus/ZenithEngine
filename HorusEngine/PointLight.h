#pragma once
#include "BaseLight.h"
#include "ICamera.h"
#include "ConstBufferExCache.h"
#include "RenderGraph.h"
#include "GlobeVolume.h"

namespace GFX::Light
{
	class PointLight : public BaseLight
	{
		Volume::GlobeVolume volume;
		mutable std::shared_ptr<Resource::ConstBufferExPixelCache> lightBuffer = nullptr;

		static inline Data::CBuffer::DCBLayout MakeLayout() noexcept;

	public:
		PointLight(Graphics& gfx, Pipeline::RenderGraph& graph, const DirectX::XMFLOAT3& position, const std::string& name,
			float intensity = 1.0f, const Data::ColorFloat3& color = { 1.0f, 1.0f, 1.0f }, float radius = 0.5f);
		PointLight(PointLight&& light) noexcept;
		PointLight& operator=(PointLight&& light) noexcept;
		PointLight(const PointLight&) = delete;
		PointLight& operator=(const PointLight&) = delete;
		virtual ~PointLight() = default;

		inline UINT GetIndexCount() const noexcept override { return volume.GetIndexCount(); }
		inline void Submit(uint64_t channelFilter) noexcept override { mesh->Submit(channelFilter); JobData::Submit(channelFilter); }

		void Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override;
		void Bind(Graphics& gfx) override;
	};
}