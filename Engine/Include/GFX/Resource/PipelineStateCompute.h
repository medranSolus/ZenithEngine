#pragma once
#include "GFX/API/DX11/Resource/PipelineStateCompute.h"
#include "GFX/Context.h"

namespace ZE::GFX::Resource
{
	// GPU state for draw call
	class PipelineStateCompute final
	{
		ZE_API_BACKEND(PipelineStateCompute, DX11, DX11, DX11, DX11) backend;

	public:
		constexpr PipelineStateCompute(Device& dev, const std::wstring& nameCS) { backend.Init(dev, nameCS); }
		PipelineStateCompute(PipelineStateCompute&&) = delete;
		PipelineStateCompute(const PipelineStateCompute&) = delete;
		PipelineStateCompute& operator=(PipelineStateCompute&&) = delete;
		PipelineStateCompute& operator=(const PipelineStateCompute&) = delete;
		~PipelineStateCompute() = default;

		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, const std::wstring& nameCS) { backend.Switch(nextApi, dev, nameCS); }
		constexpr ZE_API_BACKEND(PipelineStateCompute, DX11, DX11, DX11, DX11)& Get() noexcept { return backend; }

		void Bind(Context& ctx) const noexcept { ZE_API_BACKEND_CALL(backend, Bind, ctx); }
	};
}