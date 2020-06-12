#pragma once
#include "RenderPass.h"
#include "RenderTarget.h"
#include "GfxResources.h"
#include <array>

namespace GFX::Pipeline
{
	class RenderCommander
	{
		DepthStencil depthStencil;
		RenderTarget target;
		std::shared_ptr<Resource::Blender> fullscreenBlender = nullptr;
		std::shared_ptr<Resource::Sampler> fullscreenSampler = nullptr;
		std::shared_ptr<Resource::VertexBuffer> fullscreenVertexBuffer = nullptr;
		std::shared_ptr<Resource::IndexBuffer> fullscreenIndexBuffer = nullptr;
		std::shared_ptr<Resource::InputLayout> fullscreenInputLayout = nullptr;
		std::shared_ptr<Resource::VertexShader> fullscreenVS = nullptr;
		std::shared_ptr<Resource::PixelShader> fullscreenPS = nullptr;
		std::array<RenderPass, 3> passes;

	public:
		RenderCommander(Graphics& gfx);
		RenderCommander(const RenderCommander&) = default;
		RenderCommander& operator=(const RenderCommander&) = default;
		~RenderCommander() = default;

		inline void Add(Job&& job, size_t targetPass) noexcept { passes.at(targetPass).Add(std::forward<Job>(job)); }

		void Render(Graphics& gfx) noexcept(!IS_DEBUG);
		void Reset() noexcept;
	};
}