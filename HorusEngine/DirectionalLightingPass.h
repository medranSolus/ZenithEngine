#pragma once
#include "ShadowMapPass.h"
#include "FullscreenPass.h"

namespace GFX::Pipeline::RenderPass
{
	class DirectionalLightingPass : public Base::QueuePass, public Base::FullscreenPass
	{
		ShadowMapPass shadowMapPass;
		Camera::ICamera* mainCamera = nullptr;
		std::shared_ptr<GFX::Resource::ConstBufferExPixelCache> shadowBuffer;

		static inline Data::CBuffer::DCBLayout MakeLayout() noexcept;

	public:
		DirectionalLightingPass(Graphics& gfx, const std::string& name, UINT mapSize);
		virtual ~DirectionalLightingPass() = default;

		constexpr void BindCamera(Camera::ICamera& camera) noexcept { mainCamera = &camera; shadowMapPass.BindCamera(camera); }
		inline std::vector<Base::BasePass*> GetInnerPasses() override { return { &shadowMapPass }; }

		void Reset() noexcept override;
		Base::BasePass& GetInnerPass(std::deque<std::string> nameChain) override;
		void Execute(Graphics& gfx) override;
	};
}