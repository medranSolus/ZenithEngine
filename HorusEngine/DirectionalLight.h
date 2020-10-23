#pragma once
#include "ILight.h"
#include "ConstBufferExCache.h"
#include "RenderGraph.h"
#include "GlobeVolume.h"

namespace GFX::Light
{
	class DirectionalLight : public ILight
	{
		Volume::GlobeVolume volume;
		mutable std::shared_ptr<Resource::ConstBufferExPixelCache> lightBuffer = nullptr;

		static inline Data::CBuffer::DCBLayout MakeLayout() noexcept;

	public:
		DirectionalLight(Graphics& gfx, Pipeline::RenderGraph& graph, const DirectX::XMFLOAT3& direction, const std::string& name,
			float intensity = 1.0f, const Data::ColorFloat3& color = { 1.0f, 1.0f, 1.0f });
		DirectionalLight(DirectionalLight&& light) noexcept;
		DirectionalLight& operator=(DirectionalLight&& light) noexcept;
		DirectionalLight(const DirectionalLight&) = delete;
		DirectionalLight& operator=(const DirectionalLight&) = delete;
		virtual ~DirectionalLight() = default;

		inline UINT GetIndexCount() const noexcept override { return volume.GetIndexCount(); }
		inline void Submit(uint64_t channelFilter) noexcept override { mesh->Submit(channelFilter); JobData::Submit(channelFilter); }

		bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override;
		void Bind(Graphics& gfx) override;
	};
}