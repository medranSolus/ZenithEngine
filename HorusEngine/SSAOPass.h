#pragma once
#include "FullscreenPass.h"
#include "ICamera.h"
#include "ConstBufferExCache.h"

namespace GFX::Pipeline::RenderPass
{
	class SSAOPass : public Base::FullscreenPass
	{
		static constexpr size_t SSAO_KERNEL_SIZE = 32;
		static constexpr size_t SSAO_NOISE_SIZE = 32;

		float bias = 0.188f;
		float radius = 0.86f;
		float power = 2.77f;
		uint32_t size = SSAO_KERNEL_SIZE;

		Camera::ICamera* mainCamera = nullptr;
		GfxResPtr<Resource::IRenderTarget> ssaoScratchBuffer;
		GfxResPtr<GFX::Resource::ConstBufferExPixelCache> kernelBuffer;

		static inline Data::CBuffer::DCBLayout MakeLayout() noexcept;

	public:
		SSAOPass(Graphics& gfx, const std::string& name);
		virtual ~SSAOPass() = default;

		constexpr void BindCamera(Camera::ICamera& camera) noexcept { mainCamera = &camera; }

		void Execute(Graphics& gfx) override;
		void ShowWindow(Graphics& gfx);
	};
}