#pragma once
#include "ShadowMapPass.h"
#include "GFX/Pipeline/RenderPass/Base/FullscreenPass.h"

namespace ZE::GFX::Pipeline::RenderPass
{
	class DirectionalLightingPass : public Base::QueuePass, public Base::FullscreenPass
	{
		ShadowMapPass shadowMapPass;
		Camera::ICamera* mainCamera = nullptr;
		GfxResPtr<GFX::Resource::ConstBufferExPixelCache> shadowBuffer;

		static Data::CBuffer::DCBLayout MakeLayout() noexcept;

	public:
		DirectionalLightingPass(Graphics& gfx, std::string&& name, U32 mapSize);
		virtual ~DirectionalLightingPass() = default;

		constexpr void BindCamera(Camera::ICamera& camera) noexcept { mainCamera = &camera; shadowMapPass.BindCamera(camera); }
		std::vector<Base::BasePass*> GetInnerPasses() override { return { &shadowMapPass }; }

		void Reset() noexcept override;
		Base::BasePass& GetInnerPass(const std::deque<std::string>& nameChain) override;
		void Execute(Graphics& gfx) override;
	};
}