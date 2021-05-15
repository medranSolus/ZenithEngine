#pragma once
#include "GFX/Pipeline/RenderPass/Base/FullscreenPass.h"
#include "Camera/ICamera.h"
#include "GFX/Resource/ConstBufferExCache.h"

namespace ZE::GFX::Pipeline::RenderPass
{
	class SSAOPass : public Base::FullscreenPass
	{
		static constexpr U32 SSAO_KERNEL_SIZE = 32;
		static constexpr U32 SSAO_NOISE_SIZE = 32;

		float bias = 0.188f;
		float radius = 0.86f;
		float power = 2.77f;
		U32 size = SSAO_KERNEL_SIZE;

		Camera::ICamera* mainCamera = nullptr;
		GfxResPtr<Resource::IRenderTarget> ssaoScratchBuffer;
		GfxResPtr<GFX::Resource::ConstBufferExPixelCache> kernelBuffer;

		static Data::CBuffer::DCBLayout MakeLayout() noexcept;

	public:
		SSAOPass(Graphics& gfx, std::string&& name);
		virtual ~SSAOPass() = default;

		constexpr void BindCamera(Camera::ICamera& camera) noexcept { mainCamera = &camera; }

		void Execute(Graphics& gfx) override;
		void ShowWindow(Graphics& gfx);
	};
}