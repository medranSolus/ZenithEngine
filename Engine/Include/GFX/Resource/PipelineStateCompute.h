#pragma once
#include "GFX/API/DX11/Resource/PipelineStateCompute.h"

namespace ZE::GFX::Resource
{
	// GPU state for compute call
	class PipelineStateCompute final
	{
		ZE_API_BACKEND(PipelineStateCompute) backend;

	public:
		constexpr PipelineStateCompute(Device& dev, const std::wstring& nameCS) { backend.Init(dev, nameCS); }
		PipelineStateCompute(PipelineStateCompute&&) = default;
		PipelineStateCompute(const PipelineStateCompute&) = delete;
		PipelineStateCompute& operator=(PipelineStateCompute&&) = default;
		PipelineStateCompute& operator=(const PipelineStateCompute&) = delete;
		~PipelineStateCompute() = default;

		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, const std::wstring& nameCS) { backend.Switch(nextApi, dev, nameCS); }
		constexpr ZE_API_BACKEND(PipelineStateCompute)& Get() noexcept { return backend; }

		constexpr void Bind(Context& ctx) const noexcept { ZE_API_BACKEND_CALL(backend, Bind, ctx); }
	};
}