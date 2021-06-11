#pragma once
#include "GFX/Pipeline/RenderPass/Base/ComputePass.h"
#include "GFX/Resource/ConstBufferExCache.h"

namespace ZE::GFX::Pipeline::RenderPass
{
	class LightCombinePass : public Base::ComputePass
	{
		GfxResPtr<GFX::Resource::ConstBufferExComputeCache> ambientBuffer;

		static Data::CBuffer::DCBLayout MakeLayout() noexcept;

	public:
		LightCombinePass(Graphics& gfx, std::string&& name);
		virtual ~LightCombinePass() = default;

		void Execute(Graphics& gfx) override;
		void ShowWindow(Graphics& gfx);
	};
}