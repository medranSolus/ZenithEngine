#pragma once
#include "FullscreenPass.h"
#include "ICamera.h"
#include "ConstBufferExCache.h"

namespace GFX::Pipeline::RenderPass
{
	class SSAOPass : public Base::FullscreenPass
	{
		static constexpr size_t SSAO_KERNEL_SIZE = 64;
		static constexpr size_t SSAO_NOISE_SIZE = 16;

		Camera::ICamera* mainCamera = nullptr;
		std::shared_ptr<Resource::IRenderTarget> ssaoScratchBuffer;
		std::shared_ptr<GFX::Resource::ConstBufferExPixelCache> kernelBuffer;
		std::shared_ptr<GFX::Resource::ConstBufferExPixelCache> optionsBuffer;
		std::shared_ptr<GFX::Resource::ConstBufferExPixelCache> blurBuffer;
		std::shared_ptr<GFX::Resource::ConstBufferExPixelCache> direction;

		static inline Data::CBuffer::DCBLayout MakeLayoutKernel() noexcept;
		static inline Data::CBuffer::DCBLayout MakeLayoutOptions() noexcept;

	public:
		SSAOPass(Graphics& gfx, const std::string& name);
		virtual ~SSAOPass() = default;

		constexpr void BindCamera(Camera::ICamera& camera) noexcept { mainCamera = &camera; }

		void Execute(Graphics& gfx) override;
		void ShowWindow(Graphics& gfx);
	};
}